# scheduler，在独立线程运行的游戏进程
# 工作方式：
# 1.
class Scheduler
  attr_reader :state

  def initialize
    @workers = []
    @state = :idle
    @atomic_task_queue = []
    @atomic_task_queue_empty_count = 0
    @scores = {}

    puts "scheduler initialize, state = #{@state}"
  end

  def add_worker(worker_class)
    return if @state == :stopping
    return if @state == :stopped
    w = worker_class.new
    @workers << w
    @scores[w.object_id] = 0

    puts "scheduler add a worker of <#{worker_class}>"
  end

  def add_task(task)
    @atomic_task_queue << task.clone

    puts "scheduler receive the task: #{task}"
  end

  def run
    @state = :running
    puts "scheduler running, state = #{@state}"

    while @state != :stopped
      update()
      next_w = next_worker()
      next if !next_w
      @scores[next_w.object_id] += 100
      next_w.resume()
    end
  end

  def stop
    return if @state != :running
    @state = :stopping

    puts "scheduler will stop soon, state = #{@state}"
  end

  def update
    update_queue()
    update_workers()
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
        w.add_task(t.clone) if w.can_accept_task?(t.class)
      }
    }
    @atomic_task_queue.clear()
  end

  def update_workers
    # 1. worker stopped 的场合，从 workers 中移除
    @workers.delete_if { |w| w.state == :stopped }

    @workers.select {|w| w.state == :idle}.each(&:resume)

    if @state ==:running
      active_workers = @workers.select { |w| w.state == :running || w.state == :stopping }
    min_score = active_workers.collect { |w| @scores[w.object_id] }.min
    active_workers.each { |w| @scores[w.object_id] -= min_score }
    end

    if @state == :stopping
      # 2. stoping 状态下，将所有的 worker stop
      @workers.each { |w| w.stop() }

      # 3. 没有 worker 时，scheduler 结束
      if @workers.empty?
        @state = :stopped

        puts "engine stopped, state = #{@state}"
      end
    end
  end

  def next_worker
    active_workers = @workers.select { |w| w.state == :running || w.state == :stopping }
    return active_workers.min { |w| @scores[w.object_id] }
  end
end
