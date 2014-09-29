#!/bin/bash -u

function fail()
{
	echo "*** ERROR *** $(basename "$0")[$1]"
	(( 1 < $# )) && echo "$2"
	exit 2
}

if (( 1 > $# )); then
	echo "Usage: $(basename $0) <C/C++ new file pathname>"
	exit 1
fi

pathname="$(readlink -m "$1")"
directory="$(echo "$(echo "$(dirname "$1")" | sed -r 's:/+$::')"/)"
filename="$(sed -r 's/\.[^.]*$//' <<< "$(basename "$1")")"
extension="$(echo "$(basename "$1")" | sed -r "s:$(echo "$(basename "$1")" | sed -r 's:\.[^.]*$::')::" | sed 's:^\.::')"
header_extension=""
grep -e "^c$" <<< "${extension}" &> /dev/null && header_extension="h"
grep -e "^cpp$" <<< "${extension}" &> /dev/null && header_extension="hpp"
grep -e "^h$" <<< "${extension}" &> /dev/null && extension=""
grep -e "^hpp$" <<< "${extension}" &> /dev/null && extension=""
header_guard_id="$(awk '{print toupper($0)}' <<< "${filename}")_$(awk '{print toupper($0)}' <<< "${header_extension}")_$(uuidgen | awk '{print toupper($0)}' | sed 's/-/_/g')__INCLUDED"

#echo "\$0 = \"$0\""
#echo "\$1 = \"$1\""
#echo "pathname = \"${pathname}\""
#echo "directory = \"${directory}\""
#echo "filename = \"${filename}\""
#echo "extension = \"${extension}\""
#echo "header_extension = \"${header_extension}\""
#echo "header_guard_id = \"${header_guard_id}\""

mkdir -p "${directory}" &> /dev/null || fail ${LINENO} "Failed to create module directory."

# Create header, if necessary.
if [ -n "${header_extension}" ] ; then
target_pathname="${directory}${filename}.${header_extension}"
	[ -e "${target_pathname}" ] && fail ${LINENO} "Not creating \"${target_pathname}\"; file already exists!"
	echo "Writing: \"${target_pathname}\""
	cat > "${target_pathname}" <<EOF_for_inline
/*!
    \\file "${filename}.${header_extension}"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#ifndef ${header_guard_id}
#define ${header_guard_id}


#pragma once


// ...


#endif // #ifndef ${header_guard_id}


/*
    End of "${filename}.${header_extension}"
*/
EOF_for_inline
fi

# Create source file, if necessary.
if [ -n "${extension}" ] ; then
target_pathname="${directory}${filename}.${extension}"
	[ -e "${target_pathname}" ] && fail ${LINENO} "Not creating \"${target_pathname}\"; file already exists!"
	echo "Writing: \"${target_pathname}\""
	cat > "${target_pathname}" <<EOF_for_inline
/*!
    \\file "${filename}.${extension}"

    Copyright (c) 2014 Matt Ervin / imp software (matt@impsoftware.org)
    Formatting: 120 columns, 4 spaces per tab, spaces only (no tab characters)
    Qt Coding Style: http://qt-project.org/wiki/Qt_Coding_Style
    Qt Coding Conventions: http://qt-project.org/wiki/Coding-Conventions
    Doc-tool: Doxygen (http://www.doxygen.com/)

    ((( GNU General Public License Usage )))
    This file may be used under the terms of the GNU General Public License version 3.0 as published by
    the Free Software Foundation and appearing in the file LICENSE included in the packaging of this file.
    Please review the following information to ensure the GNU General Public License version 3.0 requirements
    will be met: http://www.gnu.org/copyleft/gpl.html.
*/


#include <${directory}Defs.${header_extension}>
#include <${directory}${filename}.${header_extension}>


// ...


/*
    End of "${filename}.${extension}"
*/
EOF_for_inline
fi

# End of script.
