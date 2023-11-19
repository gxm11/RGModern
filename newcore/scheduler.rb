# scheduler，在独立线程运行的游戏进程
# 工作方式：
# 1.
class Scheduler
  @@total_id = 0

  attr_reader :id, :state

  def initialize
    @@total_id += 1
    @id = @@total_id
    @workers = []
    @state = :ready
    @atomic_task_queue = []
    @atomic_task_queue_empty_count = 0
    @scores = {}

    puts "scheduler <#{@id}> initialized"
  end

  def add_worker(worker_class, *args)
    return if @state == :exit
    return if @state == :dead
    w = worker_class.new(self, *args)
    @workers << w
    @scores[w] = 0

    puts "scheduler <#{@id}> add a #{worker_class} <#{w.id}>"
  end

  def add_task(task)
    @atomic_task_queue << task.clone

    puts "scheduler <#{@id}> receive the task: #{task}"
  end

  def run
    @state = :active
    puts "scheduler <#{@id}> starts running"

    while @state != :dead
      update()
    end
  end

  def stop
    return if @state != :active
    @state = :exit

    puts "scheduler <#{@id}> will stop soon"
  end

  def update
    update_queue()
    update_workers()
    update_next_worker()
  end

  def update_queue
    if @atomic_task_queue_empty_count >= 100
      sleep(0.1)
      @atomic_task_queue_empty_count = 0
    end

    if @atomic_task_queue.empty?
      @atomic_task_queue_empty_count += 1
    end

    @atomic_task_queue.each { |t|
      @workers.each { |w|
        @scores[w] += 1
        w.add_task(t)
      }
    }
    @atomic_task_queue.clear()
  end

  def update_workers
    # 1. worker stopped 的场合，从 workers 中移除
    @workers.delete_if { |w| w.state == :dead }

    @workers.select { |w| w.state == :ready }.each(&:resume)

    if @state == :active
      active_workers = @workers.select { |w| w.state == :active || w.state == :exit }
      min_score = active_workers.collect { |w| @scores[w] }.min
      active_workers.each { |w| @scores[w] -= min_score }
    end

    if @state == :exit
      "scheduler <#{@id}> is cleaning up"
      # 2. stoping 状态下，将所有的 worker stop
      @workers.each { |w| w.stop() }

      # 3. 没有 worker 时，scheduler 结束
      if @workers.empty?
        @state = :dead

        puts "scheduler <#{@id}> finishes"
      end
    end
  end

  def update_next_worker
    active_workers = @workers.select { |w| w.state == :active || w.state == :exit }
    next_w = active_workers.min_by { |w| @scores[w] }

    if next_w
      @scores[next_w] += 100
      next_w.resume()
    end
  end
end
