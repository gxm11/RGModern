# Core 代码结构
Core 是引擎的核心代码，实现多线程执行框架，多态数据管理器，线程间信息传递等基础功能。

## type_traits.hpp
在 type_traits.hpp 中定义了类型的集合 TypeList<...> 和相关的几种运算：
 - is_repeated，判断一个类型是否在某个 TypeList 中
 - append，将一个类型添加到 TypeList 里，重复则什么也不做
 - unique，将一组 TypeList 中的重复类型去除
 - merge，合并两组 TypeList，同时去重
 - merge_data，合并的同时也合并 data 类型（如果存在的话）
 - as_variant，将 TypeList 转换成 std::variant
 - for_each，按顺序依次调用类的静态函数，before是顺序执行，after是倒序执行
 - remove_dummy，去掉 TypeList 中不含 run 方法的类型

### magic_cast
magic_cast 将 base 类的指针 scheduler<>* 转换成 derived 类的指针 engine_t*。而 engine_t 在代码的末尾才定义。

## datalist.hpp
在 datalist.hpp 中定义了数据管理类 datalist<...>。datalist 中各数据的类型都是不同的，并且通过多态方法 get 获取相应的数据。

定义 as_datalist 将 TypeList 转换成 datalist。

## tasklist.hpp
在 tasklist.hpp 中定义了任务的集合 tasklist<...>，内部包含两个类型变量：
 - tasks，表示任务的集合
 - data，表示数据的集合

tasklist 可以嵌套，并且会自动去掉重复的任务和数据类型。

### task
tasklist 所存放的即为任务，通常具有以下形式：
```c++
struct task
{   
    using data = TypeList<T_DATA_1, T_DATA_2>;    

    Type_1 member_1; 
    Type_2 member_2; 

    static void before(auto& executor);
    static void after(auto& executor);
    void run(auto& executor);
};
```
任务类的通常是 POD 类型（即没有自定义的构造/析构函数，所有的成员变量都是 public 的），有以下特点，不过各项都是可选的：
 - 成员变量 member_1 和 member_2
 - 类型成员 data 表示该任务可能要访问的数据类型是 T_DATA_1 和 T_DATA_2
 - 静态函数 before 和 after 表示该任务在执行前要做的准备工作和执行后的收尾工作，只会在线程创建后和销毁前各执行一次。
 - 函数 run 表示具体执行的内容。

注：
1. 如果没有定义任务的 run 函数，则该任务是 dummy 任务，会被 remove_dummy 移除。
2. before/after/run 的唯一输入参数 executor 表示当前执行该任务的执行者对象。

## kernel
kernel 是线程执行任务的方式。拥有两个成员变量和一个成员函数：
 - 信号量 m_pause，控制线程的中断和恢复
 - 任务队列 m_queue，用来存储待异步执行的任务
 - 多态函数 enqueue，用来向 m_queue 的末尾增加任务

kernel 分为 kernel_passive 和 kernel_active，都是模板类，其类型参数为 tasklist，表示队列里可以存放的任务类型。

### kernel_passive.hpp
kernel_passive 是被动的线程，它拥有一个单进单出的队列（BlockingReaderWriterQueue），其 run 函数会不断地从队列中读取任务并执行，直到外部信号通知程序结束。

### kernel_active.hpp
kernel_active 是主动的线程，它拥有一个多进多出的队列（ConcurrentQueue）。需要主动调用 step 函数来读取队列中的任务并执行。kernel_active 通常作为父类使用。

## executor.hpp
executor 是单线程的执行者，每一个 executor 都会在不同的线程执行任务。executor 有如下成员：
 - scheduler<>* 指针，用于做向下转型和调用 running 变量
 - m_data，是 datalist，存储该线程内部使用的变量
 - m_kernel，是 kernel，管理异步任务和执行任务的方式

executor 有如下函数：
 - run，依次执行任务的 before，kernel 的 run 和任务的 after。
 - running，查看当前的 scheduler 是否还在执行
 - get，多态方法，封装了 m_data 的 get 以获取特定类型的数据
 - send，多态方法，发送一个任务到自己或者其他 executor 的队列里异步执行。亦可使用 operator<<
 - signal，返回 kernel 的 m_pause 的指针
 - step，调用 kernel 的 step 方法。如果 kernel 是 passive_kernel 就什么也不做

## scheduler.hpp
scheduler 中定义了模板类 scheduler<...>，参数为全部的 executor 。scheduler<...> 继承自 scheduler<>，并使用递归继承定义。基类 scheduler<> 有唯一的成员变量 running。

scheduler 的 run 函数依次创建不同的线程，并在线程里执行 executor 的 run 函数。

scheduler 的 broadcast 函数会将任意的 task 放到正确的 executor 的队列中，如果无法放入这个任务，则返回 false。

每个 executor 都保存有 scheduler 的基类指针，从而通过 magic_cast 转成派生类的指针并调用 broadcast 方法向其他 executor 广播任务：
```c++
template<typename T, typename U = scheduler<>*>
bool send(T&& task)
{
    using base_t = U;
    using derived_t = traits::magic_cast<U>::type;
    return static_cast<derived_t>(p_scheduler)->template broadcast(std::move(task));
}
```

## core.hpp
core 汇总上述全部代码，并定义：
 - concept rgm::executor，必须是一个 executor。
 - data，是 traits::TypeList 的简称
 - 宏 RGMDATA 用于简便的调用 get 方法
 - 宏 RGMENGINE 用于特化 magic_cast
