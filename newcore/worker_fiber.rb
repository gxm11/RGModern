class Worker_Fiber
  attr_reader :state

  def initialize
    @task_queue = []
    @state = :idle
    @fiber = nil

    puts "worker initialize, state = #{@state}"
  end

  # scheduler 调用此函数，判断某个类型的task是否会被接受
  # 默认task都会被此worker接受
  def can_accept_task?(_klass)
    true
  end

  # scheduler 在进入 stopping 状态时，调用此函数，等待 worker 自行结束
  def stop
    return if @state != :running
    @state = :stopping

    puts "worker will stop soon, state = #{@state}"
  end

  # scheduler 调用此函数，执行权交给 worker
  # worker 每次执行都会导致自身 score 增加，使得 scheduler 会尝试调度其他的 worker
  def resume
    case @state
    when :idle
      # idle 的状态下，启动 fiber，执行 run 函数
      @state = :running
      puts "worker running, state = #{@state}"

      @fiber = Fiber.new { self.run() }
    when :running
      # running 的状态下，恢复 fiber
      @fiber.resume()
    when :stopping
      puts "worker stopping, state = #{@state}"
      @fiber.resume()

      # stopping 的状态下，结束 fiber
      @state = :stopped
      puts "worker stopped, state = #{@state}"
    end
  end

  def run
    while @state == :running
      while !@task_queue.empty?
        task = @task_queue.first
        task.run()
        @task_queue.shift()
      end
      Fiber.yield()
    end

    @task_queue.clear()
  end

  # 将task加入到queue里
  def add_task(task)
    @task_queue << task

    puts "worker receive the task: #{task}"
  end
end
