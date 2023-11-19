# engine，引擎本体
# 工作方式：
# 1. 添加 scheduler
# 2. 运行期间会不断检查所有的 scheduler
#  - 将 IDLE 状态的 scheduler 创建 thread 运行
#  - 将 STOPPED 状态的 scheduler 移除
# 3. 在进入 STOPPING 状态时，调用 scheduler.stop()

class Engine
  attr_reader :state

  def initialize
    @schedulers = []
    @state = :ready

    puts "engine initialized"
  end

  def run
    @state = :active
    puts "engine starts running"

    while @state == :active
      sleep 0.1
      @schedulers.each { |scheduler|
        case scheduler.state
        when :ready
          # scheduler 运行在独立的 thread 里
          Thread.start { scheduler.run() }
        when :dead
          @schedulers.delete(scheduler)
        end
      }
    end

    @state = :exit
    puts "engine is cleaning up"

    while !@schedulers.empty?
      sleep 0.1
      @schedulers.each { |scheduler|
        case scheduler.state
        when :active
          scheduler.stop()
        when :dead
          @schedulers.delete(scheduler)
        end
      }
    end

    @state = :dead
    puts "engine finishes"
  end

  def stop
    return if @state != :active

    @state = :exit
    puts "engine will stop soon"
  end

  def add_scheduler(scheduler)
    return if @state == :exit
    return if @state == :dead

    puts "engine add the scheduler: #{scheduler}"
    @schedulers << scheduler
  end

  def broadcast_task(task)
    puts "engine broadcast the task: #{task}"

    @schedulers.each do |scheduler|
      scheduler.add_task(task.clone)
    end
  end
end
