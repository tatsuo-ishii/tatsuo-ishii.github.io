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

puts [exec $cgiutils -expires now -ct text/html]

puts "<html>"
puts "<title>query result $table_name</title>"
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

set first 0

foreach i $attr {
    set atrname [lindex $i 0]
    set value [exec $cgiparse -value $atrname]

    if {$value != ""} {
	set type [lindex $i 1]
	if {[string range $type 0 3] != "char" && \
		$type != "text" && $type != "varchar"} {
	    set op "NUMERIC"
	} else {
	    set op "CASE_IGNORE"
	}
	if {$first != 0} {
	    append where " and "
	}
	append where [pg_make_query $value [list [list $atrname $type]] $op]
	set first 1
    }
}

set atrname [exec $cgiparse -value "attribute-show"]

if {$first != 0} {
    set query "select oid, $atrname from $table_name where "
    append query $where
} else {
    puts "no selectionkey"
    pg_disconnect $con
    return
}

puts [format "query: %s<p>\n" $query]

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
puts "$n 件検索されました。詳細を表示するデータを選んで下さい。"
puts "<hr>"

pg_result $res -assign tuples

for {set i 0} {$i < $n} {incr i} {
    puts "<li> "
    puts [format "<a href=\"/usr/people/ishii/cgi-bin/show_detail/$hostname/$dbname/$table_name/%s\">%s</a>\n" $tuples($i,oid) $tuples($i,$atrname)]
}

pg_disconnect $con

puts "</body>"
puts "</html>"
