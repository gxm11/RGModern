class Worker_Fiber
  attr_reader :state

  def initialize(_scheduler)
    @task_queue = []
    @state = :ready
    @fiber = nil

    puts "worker initialize, state = #{@state}"
  end

  # scheduler 在进入 stopping 状态时，调用此函数，等待 worker 自行结束
  def stop
    return if @state != :active
    @state = :exit

    puts "worker will stop soon, state = #{@state}"
  end

  # scheduler 调用此函数，执行权交给 worker
  # worker 每次执行都会导致自身 score 增加，使得 scheduler 会尝试调度其他的 worker
  def resume
    case @state
    when :ready
      # idle 的状态下，启动 fiber，执行 run 函数
      @state = :active
      puts "worker running, state = #{@state}"

      @fiber = Fiber.new { self.run() }
    when :active
      # running 的状态下，恢复 fiber
      @fiber.resume()
    when :exit
      puts "worker stopping, state = #{@state}"
      @fiber.resume()

      # stopping 的状态下，结束 fiber
      @state = :dead
      puts "worker stopped, state = #{@state}"
    end
  end

  def run
    loop do
      while !@task_queue.empty?
        task = @task_queue.first
        task.run()
        @task_queue.shift()
      end
      Fiber.yield()
      break if @state != :active
    end

    @task_queue.clear()
  end

  # 将task加入到queue里
  def add_task(task)
    return if !can_accept_task?(task.class)
    @task_queue << task.clone

    puts "worker receive the task: #{task}"
  end

  # 默认task都会被此worker接受
  def can_accept_task?(_klass)
    true
  end
end
