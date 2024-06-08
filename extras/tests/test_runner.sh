#!/bin/bash

RED=$(tput setaf 1) GREEN=$(tput setaf 2)
NC=$(tput sgr0)
passtext="${GREEN}PASSED$NC"
failtext="${RED}FAILED$NC"

ret=0
for testprog in "$@"; do
    ./$testprog
    if [ $? -eq 0 ]; then
        printf "%-40s $passtext\n" "$testprog"
    else
        printf "%-40s $failtext\n" "$testprog"
        ret=1
    fi
done

exit $ret
