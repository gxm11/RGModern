# 新的RGM核心调度规则
在阅读了erlang的协程设计后，决定采用以下核心调度规则。目前尚未完全确定规则，所以可以逐渐补充。

## 规则
1. 最顶层是一个多线程调度器scheduler，同时管理group间的共享数据share_data（包括终止标记）。
2. 每个线程里代表一个group，group中管理内部数据group_data和share_data，并负责内部worker的运行调度。
3. group中管理多个worker，每个worker分为主动和被动。
4. 主动的worker在独立的协程中运行其run函数，并且定期交出执行权。主动的worker也会定期清理自己的队列。
5. 被动的worker没有run函数，运行时会清理自己的队列，清理结束后交出执行权。
6. group中的share_data长期被占用时，会阻塞等待占用解除。
7. worker之间使用task进行数据的交换，task通常会进入到对应worker的队列中。
8. 只在task发送到同个group中的其他被动worker时，task并不会进入队列，而是立即执行。
9. worker在get数据成功后，在释放数据的所有权之前，其他的worker的get会阻塞（或者导致交出执行权）。
10. 共享数据share_data也一样，只是所有权的计数要使用原子变量。而group_data的计数是线程安全的。
11. 如果是不可变的引用getc，允许其他的worker获取不可变引用。类似于rust的数据所有权。
12. 每次执行get，getc，send等函数，都会增加worker的计数。当计数超过一定值时，这些函数会交出执行权。
13. 考虑到数据可能有死锁的问题。get数据需要判断顺序，如果高优先级的数据已经被获取，低优先级的数据的get必须阻塞。或者用其他的策略？

## 数据的共享和防死锁
1. 所有的数据都有引用计数。多个group之间分享的share_data的计数是原子变量，而group_data的计数是普通变量。
2. share_data是临时创建的，然后发送给其他的worker。可以考虑做成`std::variant<share_data, group_data>`。发送后get此share_data就会被阻塞了。
3. share_data在task中，在编译时检验其顺序，只要不乱序就不会出现死锁。这意味着share_data必须被scheduler管理。
4. group_data的死锁同上。故编译时就要检查顺序，从而task只能是用crtp格式，在run之前跑一个初始化的程序给数据。数据可以作为run的参数。

## 继续设计
根据上面的描述，task的类型应该是这样的：
```c++
template<typename T>
struct task {
    void run(auto& group) {
        std::tuple t = group.template get<T::data>();
        static_cast<T*>(this)->run(*std::get<0>(t), ...);
    }
};

struct A : task<A> {
    using data = std::tuple<special::queue, T_0, const T_1>;

    void run(auto& queue, T_0& data_0, const T_1& data_1) {
        ++data_0.x;
        queue << B{data_0.x, data_1.y};
    }
}
```
如此，
1. 数据在group.template get中拿到。
2. 在get函数中，会有静态检查，要求T::data的必须是有序的。
3. 如果数据被其他worker或者线程占用，引用计数不符合规范，get会阻塞，并导致yield。
4. t是自定义的类似指针的类型，析构时会减少引用计数。
5. 可以使用const类型，表示获取数据的不可变引用。
6. special::queue是integral类型，代指特殊的数据，比如任务队列queue，或者group、worker本身等等。
7. 原则上task只能操作数据和queue。RGMWAIT都定义在ruby函数中，实际上task是不会等待其他的task的，只有主动的worker的run会等待。

RGMWAIT的实现是这样的，以get_state为例：
```c++
struct get_state : task<get_state> {
    using data = std::tuple<const T0>;

    future<int> p_state;

    void run(const T0& data_0) {
        *p_state = data_0.x;
    }
};

int rb_get_state() {
    future<int> s{0};
    group.queue << get_state{s};
    return s.get();
}
```
1. get_state任务结束后`future<int>`析构，引用计数通过检验，s.get()就能拿到int了。否则会yield。
2. 这里可以看出，在同一个group中的被动worker的任务并不需要立即执行。这里将任务放入被动worker的queue里，因为s.get()第一次会yield，然后调度器就会调度此被动worker，清空队列后s自然有值了。当然，这里的future的引用计数是否是原子的，需要判断get_state是否会发送到相同的group，可以编译时判断。比如使用`future<int, get_state>`，或者create_future等方式。

## 继续设计
1. 全局的data都有一个顺序。可以用`__CONNTER__`宏来定义，或者编译期获取所有group，所有worker的data，放到tuple里排个序。
2. send是全局函数，用来发送一个任务。
   - 或者让任务类拥有defer()函数，会自行找到合适的worker进入队列。可能不合适，因为任务要被move走。
3. 由于send现在是全局函数，task除了调用自己需要的data之外，不再有别的依赖。get_state这样写就行：
```c++
struct get_state : task<get_state> {
    int* p_state;

    void run(const T0& data_0) {
        *p_state = data_0.x;
    }
};

int rb_get_state() {
    future<int> s{0};
    send<get_state>(s.data());
    return s.get();
}
```
上面的设计就完成了task和data的脱耦。task只需要定义run函数，函数的参数是什么类型，调度器就会主动将对应的data作为入参。
- 获取data时可能会阻塞或者yield，然而此时任务逻辑还没跑，数据本身是不会被修改的，卡在这里是安全的。
- run函数的参数类型必须遵循全局唯一的顺序，以避免死锁。编译期检查。
- 如果是share_data，数据必然是创建worker时当作成员变量传进来的。所以在获取data时，优先从成员变量中找，然后从worker的数据里找。
- task要传出share_data时，也要获取此data的，然后封装一下传走。通常都是run函数的参数是一个const引用：
```c++
struct async_calc : task<async_calc> {
    void run(const Table& t) {
        send<calc>(share(t));
    }
};

struct calc : task<calc> {
    void run(share<const Table> table) {}
};
```
由于都是const引用，可以安全同时访问。但是如果有可变引用，async_calc执行结束前，calc是不可能开始执行的。这一点可能要注意，尽可能把要改变数据的期间缩短。