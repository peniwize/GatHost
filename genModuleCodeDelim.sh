#!/bin/bash -u

function fail()
{
	echo "*** ERROR *** $(basename "$0")[$1]"
	(( 1 < $# )) && echo "$2"
	exit 2
}

for clipboard in primary secondary clipboard; do
	xclip -i -selection ${clipboard} <<EOF_for_inline
// [----------------(120 columns)---------------> Module Code Delimiter <---------------(120 columns)----------------]


EOF_for_inline
done

# End of script.
