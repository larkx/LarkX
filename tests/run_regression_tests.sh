#!/usr/bin/env bash

cd "`dirname $0`"
#sudo launchctl limit maxfiles 1000000 1000000
ulimit -n 100000

rm -f regression_tests/wallet_backup/wallet_backup.json

for regression_test in `ls regression_tests | grep -v "^\\_"`; do
    printf " Running test $regression_test...                                       \\r"
    ./wallet_tests -t regression_tests_without_network/$regression_test > /dev/null 2>&1 || printf "Test $regression_test failed.                            \\n"
    sleep 2
done;
printf "                                                                            \\r"
rm -f regression_tests/wallet_backup/wallet_backup.json
