class Worker
  attr_reader :id, :state
  @@total_id = 0

  T_Passive = 0
  T_Fiber = 1
  T_Nested = 2

  def initialize(scheduler, type = T_Passive)
    @@total_id += 1
    @id = scheduler.id * 1000 + @@total_id
    @scheduler = scheduler
    @task_queue = []
    @state = :ready
    @type = type
    @fiber = nil

    puts "worker <#{@id}> initialized"
  end

  def stop
    if @state == :active || @state == :pause
      @state = :exit

      puts "worker <#{@id}> will stop soon"
    end
  end

  def add_task(task)
    return if !can_accept_task?(task.class)
    @task_queue << task.clone

    puts "worker <#{@id}> receive the task: #{task}"
  end

  def can_accept_task?(_klass)
    true
  end

  def pop_task
    task = @task_queue.first
    return false if task.nil?

    data = lock_data(task.class)
    return false if data.nil?

    task.run(data)
    unlock_data(task.class)
    @task_queue.shift()

    return true
  end

  def lock_data(task_class)
    if rand < 0.95
      puts " - worker <#{@id}> lock data failed!"
      return nil
    else
      puts " - worker <#{@id}> locked data."
      return []
    end
  end

  def unlock_data(task_class)
  end

  def run_before
    puts "worker <#{@id}> run before"
  end

  def run_after
    puts "worker <#{@id}> run after"

    @task_queue.clear()
  end

  # 标准的 run 函数，处理完任务后，调用 suspend() 交出控制权
  def run
    loop do
      run_once()

      suspend()
      break if @state != :active
    end
  end

  # run once 只在 passive 的 worker 中被用到，
  # 此类 worker 没有挂起的功能，只是被 scheduler 反复调用 run_once
  # 每次只会跑特定数量的任务（100）就交回执行权。
  def run_once
    100.times do
      ret = pop_task()

      # 任务失败后，不再继续执行
      break if !ret
    end
  end

  def suspend
    case @type
    when T_Fiber
      Fiber.yield()
    when T_Nested
      @state = :pause if @state == :active
      @scheduler.update()
      @state = :active if @state == :pause
    end
  end

  def resume
    case @state
    when :ready
      @state = :active
      puts "worker <#{@id}> starts running"
      case @type
      when T_Fiber
        @fiber = Fiber.new {
          self.run_before()
          self.run()
          self.run_after()
        }
        @fiber.resume()
      when T_Nested, T_Passive
        run_before()
      end
    when :active
      case @type
      when T_Fiber
        @fiber.resume()
      when T_Nested
        run()
      when T_Passive
        run_once()
      end
    when :exit
      puts "worker <#{@id}> is cleaning up"
      case @type
      when T_Fiber
        @fiber.resume()
      when T_Nested, T_Passive
        run_after()
      end
      @state = :dead
      puts "worker <#{@id}> finishes"
    end
  end
end
