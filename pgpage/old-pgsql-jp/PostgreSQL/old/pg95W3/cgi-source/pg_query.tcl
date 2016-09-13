##########################################################################
#
# $Header: /usr/mgr/t-ishii/repository/xtosho2/pg_query.tcl,v 1.1.1.1 1995/09/14 05:02:43 t-ishii Exp $
# 
# Copyright (C) 1995 Tatsuo Ishii
#
#
# pg_make_query:
#
# make up a postgres95 simple qualification string 
# corresponding one or more attributes and returns it.
# no surrogates are supported, sorry. arguments are: 
#
# text: user-input query words. if plural words are given, each of them
#	 is conjuncted as "or."
#
# attrs: lists of an attribute and a type pair . 
#	if plural attributes are given, each of them are conjuncted as "or."
#	example: {{city char16} {place text}}
#
# op:	CASE_IGNORE: use "~" for operator and ignore case.(default)
#	TEXT : use "~" for operator and don't ignore case.
#	NUMERIC: regard as numeric type. the "text" arg is parsed as:
#		<value: greater than value
#		>value: lower tnan value
#		v1~v2 or v1-v2: range between v1 and v2
#		note: no space characters are permitted in "text."
#	DATE: alomost same as NUMERIC except that each token is a form
#		of "YYYY/MM/DD" such as  1995/08/01, 1995/8/1.
#		tokens are converted to integer before issuing query.
#		example: 1995/8/1 -> 19950801
#	other: use $op as an operator
##########################################################################
proc pg_make_query {text attrs {op "CASE_IGNORE"} } {

# pre process of input text

    if {$op == "CASE_IGNORE" || $op == "TEXT"} {
	set l [kstring length $text]
	for {set i 0} {$i < $l} {incr i} {
	    set c [kstring index $text $i]
	    if {$c == "\\"} {
		for {set j 0} {$j < 4} {incr j} {
		    append text2 "\\"
		}
	    }
	    append text2 $c
	}
		      
	set s $text2
	if {$op == "CASE_IGNORE"} {
	    set str ""
	    set l [kstring length $s]
	    for {set i 0} {$i < $l} {incr i} {
		set c [kstring index $s $i]
		if {$c >= "a" && $c <= "z"} then {
		    append str "\["
		    append str $c
		    append str [string toupper $c]
		    append str "\]"
		} elseif {$c >= "A" && $c <= "Z"} then {
		    append str "\["
		    append str $c
		    append str [string tolower $c]
		    append str "\]"
		} else {
		    append str $c
		}
	    }
	} else {
	    set str $text2
	}

	set first 1
	foreach i $str {
	    set first_attr 1
	    if {$first == 1} {
		foreach j $attrs {
		    if {$first_attr == 1} {
			append RESULT "([lindex $j 0] ~ \'$i\' "
			set first_attr 0
		    } else {
			append RESULT " or [lindex $j 0] ~ \'$i\' "
		    }
		}
		set first 0
	    } else {
		foreach j $attrs {
		    append RESULT " or [lindex $j 0] ~ \'$i\' "
		}
	    }
	}
	append RESULT ") "
    } elseif {$op == "NUMERIC" || $op == "DATE"} then {
	set i [lindex $attrs 0]
	set attr [lindex $i 0]
	set oper [lindex $i 1]
	set special_str "\[<>~-\]"
	set found 0

	for {set i 0} {$i < [string length $text]} {incr i} {
	    if {[string first [string index $text $i] $special_str] != -1} {
		set found 1
		break
	    }
	}
	if {$found == 1} {
	    set found_tilde [string first "~" $text]
	    set found_range -1
	    if {$found_tilde != -1} then {
		set found_range $found_tilde
	    } else {
		set found_hyphen [string first "-" $text]
		if {$found_hyphen != -1} then {
		    set found_range $found_hyphen
		}
	    }

	    if {[string index $text 0] == ">"} {
		set i [string range $text 1 end]
		if {$op == "DATE"} {
		    set i [string_to_date $i ""]
		}
		append RESULT "($attr > \'$i\'::$oper "
	    } elseif {[string index $text 0] == "<"} then {
		set i [string range $text 1 end]
		if {$op == "DATE"} {
		    set i [string_to_date $i ""]
		}
		append RESULT "($attr < \'$i\'::$oper "
	    } elseif {$found_range != -1} then {
		set r [expr $found_range - 1]
		set i [string range $text 0 $r]
		set r [expr $found_range + 1]		    
		set j [string range $text $r end]
		if {$op == "DATE"} {
		    set i [string_to_date $i ""]
		}
		if {$op == "DATE"} {
		    set j [string_to_date $j ""]
		}
		if {$i > $j} {
		    set w $i
		    set i $j
		    set j $w
		}
		append RESULT "($attr >= \'$i\'::$oper and $attr <= \'$j\'::$oper "
	    }
	} else {
	    set first 1
	    foreach i $text {
		if {$first == 1} {
		    append RESULT "($attr = \'$i\'::$oper "
		    set first 0
		} else {
		    append RESULT " or $attr = \'$i\'::$oper "
		}
	    }
	}
	append RESULT ")"
    } else {
	set first 1
	foreach i $str {
	    set first_attr 1
	    if {$first == 1} {
		foreach j $attrs {
		    if {$first_attr == 1} {
			append RESULT "([lindex $j 0] $op \'$i\'::[lindex $j 1] "
			set first_attr 0
		    } else {
			append RESULT " or [lindex $j 0] $op \'$i\'::[lindex $j 1] "
		    }
		}
		set first 0
	    } else {
		foreach j $attrs {
		    append RESULT " or [lindex $j 0] $op \'$i\'::[lindex $j 1] "
		}
	    }
	    append RESULT ") "
	}
    }
    return $RESULT
}

proc string_to_date {input str} {
    global msg

    set msg ""

    if {$input == ""} return ""

    set rtn ""
    if {[scan $input "%d%*c%d%*c%d" year month day] != 3} {
	append msg "$str 日付のフォーマットは YYYY/MM/DD です。\n"
	return $rtn
    }
    if {$year < 1900} {
	append msg "$str 年が異常($year)\n"
    } else {
	append rtn [format "%04d" $year]
    }
    if {$month < 0 || $month > 12} {
	append msg "$str 月が異常($month)\n"
    } else {
	append rtn [format "%02d" $month]
    }
    if {$day < 0 || $day > 31} {
	append msg "$str 日が異常($day)\n"
    } else {
	append rtn [format "%02d" $day]
    }
    return $rtn
}
    
proc date_to_string {date} {
    if {$date == ""} return ""

    set year [expr $date / 10000]
    set month [expr ($date % 10000)/100]
    set day [expr $date % 100]
    return [format "%04d/%02d/%02d" $year $month $day]
}

#
# quote "'"
#
proc quote_chars {str} {

    set rtn ""

    set l [kstring length $str]
    for {set i 0} {$i < $l} {incr i} {
	set c [kstring index $str $i]

	if {$c == "'"} {
	    append rtn "\\"
	}
	append rtn $c
    }
    return $rtn
}


##########################################################################
# pgq_get_schema:
# read the postgres95 system catalog and
# returns a attribute and type list of specified table.
# 
# arguments are:
#
# con:		connection returned by pg_connect
# table:	table name
##########################################################################
proc pgq_get_schema {con table} {
    if [catch {set res [pg_exec $con "select a.attrelid ,a.attname,t.typname \
	from pg_class c, pg_attribute a,pg_type t \
	where c.relname=\'$table\' and a.attnum > 0 and \
	a.attrelid=c.oid and a.atttypid=t.oid order by attrelid"]}] {
	puts "pgq_get_schema: cannot get attributes of $table"
	return
    }

    if {[string range $res 0 2] != "pgp"} {
	puts "pgq_get_schema: cannot get attributes of $table"
	return
    }

    set n [pg_result $res -numTuples]

    for {set i 0} {$i < $n} {incr i} {
	set tuple [pg_result $res -getTuple $i]
	lappend attr [list [lindex $tuple 1] [lindex $tuple 2]]
    }

    pg_result $res -clear
    return $attr
}
