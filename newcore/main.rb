require_relative "engine"
require_relative "scheduler"
require_relative "worker"
require_relative "task_base"

$e = Engine.new

$s = Scheduler.new
$s2 = Scheduler.new

$s.add_worker(Worker, Worker::T_Nested)
$s.add_worker(Worker, Worker::T_Passive)
$s.add_worker(Worker, Worker::T_Fiber)

$s2.add_worker(Worker, Worker::T_Fiber)

$e.add_scheduler($s)
$e.add_scheduler($s2)

$e.broadcast_task(Task_Base.new)

Thread.start {
  sleep 2
  $e.broadcast_task(Task_Base.new)
  sleep 2
  $e.stop()
}

$e.run()
