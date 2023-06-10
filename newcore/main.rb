class Active_Worker
  Tasks = %i[a b c]

  def run
    i = 0
    loop do
      puts i
      i += 1
      Fiber.yield
      break if i == 10
    end
  end
end

class Inactive_Worker
  Tasks = %i[d e f]
end

class Group
  Workers = [Active_Worker.new, Inactive_Worker.new]
  Queue = []

  def initialize
    @fs = Workers.collect { |w| w.methods.include?(:run) ? Fiber.new { w.run } : nil }
    @fs.compact!
    @done = false
  end

  def run
    loop do
      @fs.each do |f|
        f.resume
      rescue FiberError
        @done = true
        break
      end
      break if @done
    end
  end
end

g = Group.new
g.run
