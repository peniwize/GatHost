#!/bin/bash
FILE="$(dirname $0)/Src/Version.hpp"
[ -e "${FILE}" ] || > "${FILE}"
FILEVAL="$(grep 'SVN_VERSION' < "${FILE}" | awk '{print $3;}' | sed 's/"//g')"
SVNVAL="$(pushd "$(dirname $0)" &> /dev/null ; svnversion -n ; popd &> /dev/null)"
if [ "${FILEVAL}" != "${SVNVAL}" ]; then
    echo "!!! UPDATING SVN VERSION FILE !!!"
    echo "#define SVN_VERSION \"${SVNVAL}\"" > "${FILE}"
fi

