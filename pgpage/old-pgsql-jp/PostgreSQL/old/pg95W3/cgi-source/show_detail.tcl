# 
# Copyright (C) 1995 Tatsuo Ishii
#

set cgiparse [glob ~ishii/cgi-bin/cgiparse]
set cgiutils [glob ~ishii/cgi-bin/cgiutils]

global env

set l [split [string range $env(PATH_INFO) 1 end] /]
set hostname [lindex $l 0]
set dbname [lindex $l 1]
set table_name [lindex $l 2]
set oid [lindex $l 3]

puts [exec $cgiutils -expires now -ct text/html]
puts "<html>"
puts "<title>detail data of $table_name</title>"
puts "<body>"

source [glob ~ishii/cgi-bin/pg_query.tcl]

if [catch {set con [pg_connect $dbname -host $hostname]}] {
    puts "Error: cannot connect to $hostname:$dbname."
    return
}

set attr [pgq_get_schema $con $table_name]

if {[info exist attr] == 0} {
    puts "Error: cannot get schema of $table_name"
    return
}

set query "select * from $table_name where "

append query [pg_make_query $oid [list [list "oid" "oid"]] "NUMERIC"]

if [catch {set res [pg_exec $con $query]}] {
    puts "Error: cannot select table: $table_name"
    pg_disconnect $con
    return
}

if {[string range $res 0 2] != "pgp"} {
    puts "Error: cannot select table: $table_name"
    pg_disconnect $con
    return
}

set n [pg_result $res -numTuples]

pg_result $res -assign tuples

for {set i 0} {$i < $n} {incr i} {
    puts "<hr>"
    foreach j $attr {
	set atrname [lindex $j 0]
	puts [format "<dt> <b>%s</b>" $atrname]
	puts [format "<dd> %s" $tuples($i,$atrname)]
    }
}

pg_disconnect $con

puts "</body>"
puts "</html>"
