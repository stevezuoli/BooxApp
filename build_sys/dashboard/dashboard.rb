require 'rubygems'
require 'sinatra'
require 'sqlite3'
require 'pp'

site = "http://build.i.page2page.net:8080/"
get '/' do
  db = SQLite3::Database.new( "#{ENV['HOME']}/builds.db" )
  rows = db.execute("select distinct branch from builds;")
  bnames = []
  rows.each {|row| bnames << row[0]}
  @branches = {}
  bnames.each do |bname|
    branch = {}
    rows = db.execute("select distinct profile from builds " +
                      "where branch='#{bname}'")
    branch['profiles'] = []
    rows.each {|row| branch['profiles'] << row[0]}

    rows = db.execute("select distinct kernel from builds " +
                      "where branch='#{bname}'")
    branch['kernels'] = []
    rows.each {|row| branch['kernels'] << row[0]}

    branch['builds'] = {}
    branch['kernels'].each do |kernel|
      branch['builds'][kernel] = {}
      branch['profiles'].each do |profile|
        rows = db.execute("select build_num, build_id from builds " +
                          "where branch='#{bname}' and kernel='#{kernel}' " +
                          "and profile='#{profile}' order by build_num " +
                          "desc limit 1")
        build_num, build_id = rows[0]
        branch['builds'][kernel][profile] = {:id => build_id, :num => build_num}
#        html += "<td><a href=\"#{site}job/BooxImage/#{build_num}/\">#{build_id}</a></td>"
      end
    end
    @branches[bname] = branch
  end
  pp @branches
  haml :index
end
