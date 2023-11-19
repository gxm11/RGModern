class Task_Base
  @@total_id = 0

  def initialize
    @@total_id += 1
    @id = @@total_id
  end

  def run()
    puts "task <#{@id}> done."
  end
end
