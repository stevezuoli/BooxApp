#!/usr/bin/env ruby

require 'fileutils'

HEADER = <<-EOS
// -*- mode: c++; c-basic-offset: 4; -*-
// Copyright 2010 Onyx International Inc.
EOS

NOTICE = <<-EOS

// This file is part of the Onyx Device SDK.

// The Onyx Device SDK is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// The Onyx Device SDK is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with the Onyx Device SDK.  If not, see
// <http://www.gnu.org/licenses/>.

EOS

def add_to_file(filename)
  outname = "/tmp/temp.txt"
  infile = File.open(filename, 'r')
  outfile = File.open(outname, 'w')
  done = false
  infile.each_line do |line|
    if infile.lineno == 1
      if line =~ %r{c-basic-offset}
        # overwrite the old mode header
        outfile.puts HEADER[0]
        next
      elsif line =~ %r{Copyright}
        # output entire header, overwriting the old copyright notice.
        outfile.puts HEADER
        outfile.puts NOTICE
        done = true
      else
        outfile.puts HEADER
        outfile.puts NOTICE
        outfile.puts line
        done = true
      end
    elsif infile.lineno == 2 && !done
      if line =~ %r{Copyright}
        # overwriting the old copyright notice.
        outfile.puts HEADER[1]
        outfile.puts NOTICE
      else
        outfile.puts HEADER[1]
        outfile.puts NOTICE
        outfile.puts line
      end
      done = true
    end
    outfile.puts line
  end
  infile.close
  outfile.close
  FileUtils.mv outname, filename
end

add_to_file(ARGV[0])
