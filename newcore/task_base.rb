class Task_Base
  @@total_id = 0

  def initialize
    @@total_id += 1
    @id = @@total_id
  end

  def run(data)
    sleep rand(10) / 100
    puts "task <#{@id}> done."
  end
end
