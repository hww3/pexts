#!/bin/bash

$PIKE --show-paths 2>&1 |
    awk '

($1=="Include") {
	p="PIKE_INCLUDES"
	a[p] = "-I" $4
	next
}

($1=="Module") {
	p="PIKE_MODULES"
	a[p] = $4
	next
}

($1=="Program") {
	p=""
	next
}

($1=="Master") {
	p=""
	next
}

($1 ~ "^/") {
	if( p == "PIKE_INCLUDES" )
		a[p] = a[p] " -I" $1
	next
}

END {
	for( i in a )
		print i "=" a[i]
	match(a["PIKE_INCLUDES"],"/pike[0-9.]*/");
	pci = "-I/usr/include" substr(a["PIKE_INCLUDES"],RSTART,RLENGTH);
	print "PIKE_CINCLUDES=" pci
}
'

