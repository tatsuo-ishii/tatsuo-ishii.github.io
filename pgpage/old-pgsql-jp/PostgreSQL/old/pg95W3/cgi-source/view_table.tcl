# 
# Copyright (C) 1995 Tatsuo Ishii
#

puts "<html>"
puts "<title>"
puts "list of attributes"
puts "</title>"
puts "<body>"

source [glob ~ishii/cgi-bin/pg_query.tcl]

set hostname [lindex $argv 0]
set dbname [lindex $argv 1]
set table_name [lindex $argv 2]

if [catch {set con [pg_connect $dbname -host $hostname]}] {
    puts "Error: cannot connect to $hostname:$dbname."
    return
}

set attr [pgq_get_schema $con $table_name]

if {[info exist attr] == 0} {
    puts "Error: cannot get schema of $table_name"
    return
}

puts "<form method=\"get\" action=\"/usr/people/ishii/cgi-bin/exec_query/$hostname/$dbname/$table_name\">"

puts "<input type=\"submit\" value=\"検索開始\"><p>"
puts "検索結果リストに表示する attribute: "
puts "<select name=\"attribute-show\">"
set cnt 0
foreach i $attr {
    if {$cnt == 0} {
	puts [format "<option selected> %s\n" [lindex $i 0]]
	set cnt 1
    } else {
	puts [format "<option> %s\n" [lindex $i 0]]
    }
}
puts "</select><p>"

foreach i $attr {
    puts [format "<input type=\"text\" name=\"%s\" maxlength=32>%s \[%s\]<br>\n" \
	      [lindex $i 0] [lindex $i 0] [lindex $i 1]]
}

puts "</form>"
puts "</html>"

pg_disconnect $con
