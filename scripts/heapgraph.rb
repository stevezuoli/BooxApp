#!/usr/bin/env ruby

def get_heap_size(pid)
  # The special system file has to be read using cat
  text = `cat /proc/#{pid}/status`
  re = /VmData:\s*(\d+)\s*/
  match = re.match(text)
  if match != nil
    return match[1]
  else
    return nil
  end
end

cmdline =  ARGV.join(' ')
pid = fork { exec cmdline }
File.open('heapsize.txt', 'w').close()
output = IO.popen('spline | graph -T X', 'w')
t = 0
while pid != Process.waitpid(pid, Process::WNOHANG)
  sleep 1
  heap_size = get_heap_size(pid)
  if heap_size != nil
    output << t << "\t" << heap_size << "\n"
    output.flush
    File.open('heapsize.txt', 'a') do |file|
      file << t << "\t" << heap_size << "\n"
    end
    t += 1
  else
    next
  end
end
output.close_write
# Process.waitpid(output.pid)
