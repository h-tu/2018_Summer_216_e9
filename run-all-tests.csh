#!/bin/csh

set elt = 100
set thread = 1
set i = 1
set ten = 10
set tmp = 0

while ( $thread <= 10 )
    while( $elt <= 1000000000)
        echo "elt: $elt thead: $thread task: 1"
        ./threads "$elt" "$thread" "1" "1" "Y"
	while( $i <= 10)
	    @ tmp += $elt
	    @ i ++
	end
        @ elt = $tmp
	set i = 1
	set tmp = 0
    end
    set elt = 100
    set tmp = 0
    set i = 1
    @ thread ++
end

set elt = 100
set thread = 1
set i = 1
set ten = 10
set tmp = 0

while ( $thread <= 10 )
    while( $elt <= 1000000000)
        echo "elt: $elt thead: $thread task: 2"
        ./threads "$elt" "$thread" "1" "2" "Y"
	while( $i <= 10)
	    @ tmp += $elt
	    @ i ++
	end
        @ elt = $tmp
	set i = 1
	set tmp = 0
    end
    set elt = 100
    set tmp = 0
    set i = 1
    @ thread ++
end
