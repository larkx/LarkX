#!/usr/bin/env bash

cd "`dirname $0`"
#sudo launchctl limit maxfiles 1000000 1000000
ulimit -n 100000

rm -f regression_tests/wallet_backup/wallet_backup.json regression_tests/blockchain_export_fork_graph/*.gv

for regression_test in `ls regression_tests | grep -v "^\\_"`; do
    printf " Running test %-52s\\r" "$regression_test..."
    ./wallet_tests -t regression_tests_without_network/$regression_test > /dev/null 2>&1 || printf "Test %-60s\\n" "$regression_test failed."
    sleep 2
done;
printf "%-65s\\r"
rm -f regression_tests/wallet_backup/wallet_backup.json regression_tests/blockchain_export_fork_graph/*.gv
