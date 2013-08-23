#!/usr/bin/env ruby1.9

# This scripts scans files under a directory and generates
# CMakeLists.txt for all targets. Manual content outside the
# automatically generated sections are preserved to allow
# customization.

require 'pp'

class Config
  attr_accessor :config

  def initialize
    @config = {}
    @config[:exclude] = [ "CMakeFiles" ]
    @config[:lib_prefix] = ''
    @config[:exe_prefix] = ''
    @config[:test_prefix] = ''
    @config[:additional_libs] = []
  end

  def [](key)
    return @config[key]
  end

  def add_additional_libs(*libs)
    @config[:additional_libs] += libs
  end

  def set_library_prefix(prefix)
    @config[:lib_prefix] = prefix
  end

  def set_executable_prefix(prefix)
    @config[:exe_prefix] = prefix
  end

  def set_test_prefix(prefix)
    @config[:test_prefix] = prefix
  end

  def merge(filename)
    new_config = Marshal.load(Marshal.dump(self))
    new_config.instance_eval(IO.read(filename), filename)
    return new_config
  end

  def recursive_config
    new_config = Marshal.load(Marshal.dump(self))
    new_config.clear_nonrecursive_options!
    return new_config
  end

  def clear_nonrecursive_options!
    @config[:exclude] = nil
    @config[:include] = nil
  end

  def should_process?(item)
    if @config[:include] && !(@config[:include].empty?)
      return @config[:include].include? item
    elsif @config[:exclude] && !(@config[:exclude].empty?)
      return !(@config[:exclude].include? item)
    else
      return true
    end
  end

  def include(item)
    unless @config[:include]
      @config[:include] = []
    end
    if item.is_a? Array
      @config[:include] += item
    else
      @config[:include] << item
    end
  end

  def exclude(item)
    unless @config[:exclude]
      @config[:exclude] = []
    end
    if item.is_a? Array
      @config[:exclude] += item
    else
      @config[:exclude] << item
    end
  end
end

def scan_files(dir)
  Dir.new(dir).each do |f|
    yield(f) unless f =~ /^\..*/
  end
  return []
end

def is_source(file, dir)
  if (file =~ /^.*\.(cc|cpp|C|c)$/) && (File.file?(File.expand_path file, dir))
    return true
  else
    return false
  end
end

def is_header(file, dir)
  if (file =~ /^.*\.(h|hpp|hh)$/) && (File.file?(File.expand_path file, dir))
    return true
  else
    return false
  end
end

def is_config(file)
  return file == "gen_cmake_config.rb"
end

class CMakeGenerator
  class TargetRepo
    @@targets = []
    def TargetRepo.add_target(t)
      @@targets << t
    end

    def TargetRepo.targets
      return @@targets
    end
  end

  class Target
    def initialize(src, dir, hdrs, config)
      @src, @dir = src, dir
      @hdr = find_hdr src, hdrs
      @config = config
      if @hdr
        @type = :library
        if need_moc?
          @moc_src = "#{remove_ext(src)}_moc_src"
        else
          @moc_src = nil
        end
        @full_hdr_path = File.expand_path(@hdr, @dir)
      elsif @src =~ /.*_test\..*/
        @type = :test
      else
        @type = :executable
      end
      TargetRepo.add_target(self)
    end

    def full_hdr_path
      @full_hdr_path
    end

    def resolve_deps
      scan_dep_hdrs
      @deps = []
      @dep_hdrs.each do |dep_hdr|
        TargetRepo.targets.each do |t|
          # ideally should use something like ends_with? but include? is
          # probably good enough.
          if t.full_hdr_path && (t.full_hdr_path.include? dep_hdr)
            @deps << t.output_name unless t.output_name == output_name
          end
        end
      end
    end

    def scan_dep_hdrs
      @dep_hdrs = []
      if @hdr
        File.open(File.expand_path(@hdr, @dir), 'r').each_line do |line|
          matches = line.scan /#include\s+"([^"]*)".*/
            unless matches.empty?
              @dep_hdrs << matches[0][0]
            end
        end
      end
      File.open(File.expand_path(@src, @dir), 'r').each_line do |line|
        matches = line.scan /#include\s+"([^"]*)".*/
        unless matches.empty?
          @dep_hdrs << matches[0][0]
        end
      end
    end

    def output_name
      prefix = ""
      if @type == :library
        prefix = @config[:lib_prefix]
      elsif @type == :executable
        prefix = @config[:exe_prefix]
      else
        prefix = @config[:test_prefix]
      end
      return prefix + remove_ext(@src)
    end

    def find_hdr(src, hdrs)
      base = remove_ext src
      hdrs.each do |h|
        if base == remove_ext(h)
          return h
        end
      end
      return nil
    end

    def remove_ext(fname)
      return File.basename fname, (File.extname fname)
    end

    def need_moc?
      File.open(File.expand_path(@hdr, @dir), 'r').each_line do |line|
        if line =~ /.*Q_OBJECT.*/
          return true
        end
      end
      return false
    end

    def to_cmake_lines
      resolve_deps
      lines = []
      if @moc_src
        lines << %Q{qt4_wrap_cpp(#{@moc_src} #{@hdr})}
      end
      moc_src_str = @moc_src ? "${#{@moc_src}}" : ""
      if @type == :library
        lines << %Q{add_library(#{self.output_name} #{@src} #{moc_src_str})}
        lines << %Q{strict_warning(#{self.output_name})}
      elsif @type == :executable
        lines << %Q{add_executable(#{self.output_name} #{@src} #{moc_src_str})}
        lines << %Q{strict_warning(#{self.output_name})}
      elsif @type == :test
        lines << %Q{onyx_test(#{self.output_name} #{@src} #{moc_src_str})}
        lines << %Q{strict_warning(#{self.output_name})}
      end
      if @deps && (!@deps.empty?)
        lines << %Q{target_link_libraries(#{self.output_name}
  #{@deps.join(' ')}
  #{@config[:additional_libs].join(' ')})}
      end
      return lines
    end
  end

  def initialize(srcs, hdrs, config, dir)
    @srcs, @hdrs, @config, @dir = srcs, hdrs, config, dir
    @targets = []
  end

  def process
    @srcs.each do |src|
      @targets << Target.new(src, @dir, @hdrs, @config)
    end
    write_new_cmake unless @targets.empty?
  end

  private
  def write_new_cmake
    preserved_lines = []
    in_generated_part = false
    written = false
    cmake_fname = File.join @dir, 'CMakeLists.txt'
    File.open(cmake_fname, 'r') do |f|
      f.each_line do |line|
        if line =~ /^# BEGIN_GEN_CMAKE.*/
          preserved_lines << line
          if in_generated_part
            raise SyntaxError, "@{cmake_fname} has nested GEN_CMAKE section!"
          else
            in_generated_part = true
          end
        elsif line =~ /^# END_GEN_CMAKE.*/
          unless in_generated_part
            raise SyntaxError, "@{cmake_fname} has END_GEN_CMAKE but no " +
              "BEGIN_GEN_CMAKE"
          else
            in_generated_part = false
            @targets.each do |t|
              preserved_lines += t.to_cmake_lines
            end
            written = true
          end
          preserved_lines << line
        else
          preserved_lines << line unless in_generated_part
        end
      end
    end
    if in_generated_part
      raise SyntaxError, "@{cmake_fname} has BEGIN_GEN_CMAKE but no " +
        "END_GEN_CMAKE"
    end
    unless written
      preserved_lines << ""
      preserved_lines << "# BEGIN_GEN_CMAKE  ***DO NOT EDIT MANUALLY***"
      @targets.each do |t|
        preserved_lines += t.to_cmake_lines
      end
      preserved_lines << "# END_GEN_CMAKE"
      preserved_lines << ""
    end
    File.open(cmake_fname, 'w') do |f|
      f.puts preserved_lines
    end
  end
end

def process_dir(dir, config)
  srcs = []
  hdrs = []
  dirs = []

  scan_files(dir) do |f|
    if is_config f
      config = config.merge(File.expand_path f, dir)
    elsif is_source f, dir
      srcs << f
    elsif is_header f, dir
      hdrs << f
    elsif File.directory?(File.expand_path f, dir)
      dirs << f
    end
  end
  srcs.sort!
  hdrs.sort!
  dirs.sort!

  cmake_generator = CMakeGenerator.new(srcs, hdrs, config, dir)
  cmake_generator.process
  dirs.each do |d|
    if config.should_process? d
      process_dir (File.expand_path d, dir), config.recursive_config
    end
  end
end


process_dir (File.expand_path '.'), Config.new
