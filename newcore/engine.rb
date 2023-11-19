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
    @state = :idle

    puts "engine initialize, state = #{@state}"
  end

  def run
    @state = :running
    puts "engine running, state = #{@state}"

    while @state == :running
      sleep 0.1
      @schedulers.each { |scheduler|
        case scheduler.state
        when :idle
          # scheduler 运行在独立的 thread 里
          Thread.start { scheduler.run() }
        when :stopped
          @schedulers.delete(scheduler)
        end
      }
    end

    @state = :stopping
    puts "engine stopping, state = #{@state}"

    while !@schedulers.empty?
      sleep 0.1
      @schedulers.each { |scheduler|
        case scheduler.state
        when :running
          scheduler.stop()
        when :stopped
          @schedulers.delete(scheduler)
        end
      }
    end

    @state = :stopped
    puts "engine stopped, state = #{@state}"
  end

  def stop
    return if @state != :runing
    return if @state != :stopped

    @state = :stopping
    puts "engine will stop soon, state = #{@state}"
  end

  def add_scheduler(scheduler)
    return if @state == :stopping
    return if @state == :stopped

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
