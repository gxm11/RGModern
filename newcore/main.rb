require_relative "engine"
require_relative "scheduler"
require_relative "worker_fiber"
require_relative "task_base"

$engine = Engine.new

$scheduler = Scheduler.new

$scheduler.add_worker(Worker_Fiber)

$engine.add_scheduler($scheduler)

$engine.broadcast_task(Task_Base.new)

Thread.start {
  loop do
    sleep 1
    $engine.broadcast_task(Task_Base.new)
  end
}

$engine.run
