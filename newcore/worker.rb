class Worker_Base
  attr_accessor :id
  attr_reader :score, :state, :fiber

  def initialize
    @task_queue = []
    @id = 0
    @data = []
    @state = :idle
    @hangup = false
    @score = 0
    @fiber = nil
  end

  def can_accept_task?(_klass)
    false
  end

  def update
    return if @hangup
    return if !@active
  end

  def stop
    return if @state != :running
    @state = :stopped
  end

  def resume
  end

  def update
  end

  def run_once
  end

  def run
    while @active
      @task_queue.each do |t|
        data = get_data(t.data)
        t.run()
      end
      @task_queue.clear()
    end
  end

  def add_task(t)
  end

  def get_data(t_data)
  end
end
