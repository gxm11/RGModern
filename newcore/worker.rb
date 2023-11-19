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

  def run_task
    task = @task_queue.first
    if task
      task.run()
      @task_queue.shift()
    end
  end

  def run_before
    puts "worker <#{@id}> run before"
  end

  def run_after
    puts "worker <#{@id}> run after"

    @task_queue.clear()
  end

  def run
    loop do
      while !@task_queue.empty?
        run_task()
      end
      suspend()
      break if @state != :active
    end
  end

  def run_once
    while !@task_queue.empty?
      run_task()
    end
  end

  def suspend
    if @type == T_Fiber
      Fiber.yield()
    end
    if @type == T_Nested
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
      if @type == T_Fiber
        @fiber = Fiber.new {
          self.run_before()
          self.run()
          self.run_after()
        }
        @fiber.resume()
      else
        run_before()
      end
    when :active
      if @type == T_Fiber
        @fiber.resume()
      elsif @type == T_Nested
        run()
      else
        run_once()
      end
    when :exit
      puts "worker <#{@id}> is cleaning up"
      if @type == T_Fiber
        @fiber.resume()
      else
        run_after()
      end
      @state = :dead
      puts "worker <#{@id}> finishes"
    end
  end
end
