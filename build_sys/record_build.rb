#!/usr/bin/env ruby

require 'rubygems'
require 'sqlite3'
require 'pp'

db = SQLite3::Database.new( "#{ENV['HOME']}/builds.db" )
db.execute("create table if not exists builds " +
           "(branch, kernel, profile, build_name, build_num, build_id)")
db.execute("insert into builds " +
           "(branch, kernel, profile, build_name, build_num, build_id) " +
           "values ( :branch, :kernel, :profile, :build_name, :build_num, " +
           ":build_id )",
           "branch" => ENV['SOURCE_BRANCH'],
           "kernel" => ENV['KERNEL_FLAVOR'],
           "profile" => ENV['PROFILE'],
           "build_name" => ENV['JOB_NAME'],
           "build_num" => ENV['BUILD_NUMBER'],
           "build_id" => ENV['BUILD_ID'])
