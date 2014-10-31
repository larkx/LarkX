#!/bin/sh

make -C ../../.. bts_create_key

for i in `seq 0 100`; do
    ../bts_create_key >tmp
    pub=`grep '^public' tmp | cut -d' ' -f 3`
    priv=`grep ' WIF ' tmp | cut -d' ' -f 5`
    echo '{"name":"init'$1-$i'","owner":"'$pub'","delegate_pay_rate":100},'
    echo "init$1-$i $pub $priv" >&2
done >init$1.json 2>private$1.csv

rm -f tmp

echo "Please remove the trailing comma in the last line of init$1.json!"
