task :default => :build

def has_distcc
  if ENV['DISABLE_DISTCC']
    false
  else
    system("which distcc")
  end
end

def has_ccache
  system('which ccache')
end

def compiler_prefix
  if has_ccache
    "ccache "
  elsif has_distcc
    "distcc "
  else
    ""
  end
end

def cmake_cmd(arch)
  marvell_cc = "/usr/local/arm/arm-marvell-linux-gnueabi/bin/arm-linux-gcc"
  marvell_cxx = "/usr/local/arm/arm-marvell-linux-gnueabi/bin/arm-linux-g++"
  arm_cc = "/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-linux-gcc"
  arm_cxx = "/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/arm-linux-g++"
  if arch == :arm
    "CC='#{compiler_prefix}#{arm_cc}' CXX='#{compiler_prefix}#{arm_cxx}' cmake "
  elsif arch == :marvell
    "CC='#{compiler_prefix}#{marvell_cc}' CXX='#{compiler_prefix}#{marvell_cxx}' cmake "
  elsif arch == :x86
    "CC='#{compiler_prefix}gcc' CXX='#{compiler_prefix}g++' cmake "
  else
    raise "Invalid machine type: #{arch}"
  end
end

def make_cmd(arch)
  if has_distcc
    "make -j18"
  else
    "make -j5"
  end
end

desc "Update submodules from repository"
task :update do
  sh "git submodule update --init --recursive"
end

desc "Remove the build directory."
task :clean do
  sh 'rm -rf build'  # Remove compilation targets and cmake output.
end

directory "build/arm"
directory "build/x86"
directory "build/marvell"

task :shared_env do
  if has_distcc
    ENV['CCACHE_PREFIX'] = 'distcc'
  end
end

task :arm_env do
  path = ENV['PATH']
  ENV['PATH'] = "/opt/onyx/arm/bin:/opt/freescale/usr/local/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin/:#{path}"
  ENV['ONYX_SDK_ROOT'] = '/opt/onyx/arm'
  ENV['PKG_CONFIG_PATH'] = '/opt/onyx/arm/lib/pkgconfig/'
  ENV['QMAKESPEC'] = '/opt/onyx/arm/mkspecs/qws/linux-arm-g++/'
end

task :marvell_env do
  path = ENV['PATH']
  ENV['PATH'] = "/opt/onyx/arm/bin:/usr/local/arm/arm-marvell-linux-gnueabi/bin:#{path}"
  ENV['ONYX_SDK_ROOT'] = '/opt/onyx/arm'
  ENV['PKG_CONFIG_PATH'] = '/opt/onyx/arm/lib/pkgconfig/'
  ENV['QMAKESPEC'] = '/opt/onyx/arm/mkspecs/qws/linux-arm-g++/'
end

namespace :config do
  namespace :x86 do
    task :static => ["build/x86", :shared_env] do
      sh "cd build/x86 && #{cmake_cmd :x86} -DONYX_BUILD_STATIC_LIB:BOOL=ON ../.."
    end

    task :default => ["build/x86", :shared_env] do
      sh "cd build/x86 && #{cmake_cmd :x86} ../.."
    end
  end

  namespace :arm do
    task :static => [:shared_env, :arm_env, "build/arm"] do
      sh "cd build/arm && #{cmake_cmd :arm} -DONYX_BUILD_STATIC_LIB:BOOL=ON -DBUILD_FOR_ARM:BOOL=ON ../.."
    end
    task :default => [:shared_env, :arm_env, "build/arm"] do
      sh "cd build/arm && #{cmake_cmd :arm} -DBUILD_FOR_ARM:BOOL=ON ../.."
    end
  end

  task :marvell => [:shared_env, :marvell_env, "build/marvell"]do
    sh "cd build/marvell && #{cmake_cmd :marvell} -DBUILD_FOR_ARM:BOOL=ON ../.."
  end
end

namespace :build do
  namespace :x86 do
    desc "Build the SDK (x86)."
    task :sdk => "config:x86:static" do
      sh "cd build/x86 && #{make_cmd :x86} -C sdk_readonly"
    end

    desc "Build all packages."
    task :all => "config:x86:default" do
      sh "cd build/x86 && #{make_cmd :x86} -j12"
    end
  end

  namespace :arm do
    desc "Build the ARM version of the SDK."
    task :sdk => ["config:arm:static", :arm_env] do
      sh "cd build/arm && #{make_cmd :arm} -C sdk_readonly"
    end

    desc "Build the ARM version of all packages."
    task :all => ["config:arm:default", :arm_env] do
      sh "cd build/arm && #{make_cmd :arm}"
    end
  end

  task :marvell => ["config:marvell", :marvell_env] do
      sh "cd build/marvell && #{make_cmd :marvell}"
  end

  desc "Build the SDK (x86 and arm)"
  task :sdk => ["build:x86:sdk", "build:arm:sdk"]
end


desc "Build all packages (native compiling)."
task :build => "build:x86:all"

desc "Run all tests."
task :test => :build do
  sh "killall -q -9 Xvfb || true"
  sh "cd build/x86 && xvfb-run -a -w 3 make test"
end

languages = [:en_US, :fr_FR, :de_DE, :de_DE, :zh_CN, :zh_TW, :fi_FI, :es_ES, :it_IT,
             :pl_PL, :sv_SE, :nl_NL, :ru_RU, :gr_GR, :ja_JP, :he_IL, :el_GR, :ko_KR,
             :pt_PT, :pl_PL, :hu_HU, :he_IL, :nb_NO]
ts_files = languages.map {|lang| "translations/onyx_#{lang}.ts"}

desc "Update translation files."
task :lupdate do
  sh "lupdate -recursive . -ts #{ts_files.join ' '}"
end

desc "Generate pm files"
task :lrelease do
  sh "lrelease #{ts_files.join ' '}"
end
