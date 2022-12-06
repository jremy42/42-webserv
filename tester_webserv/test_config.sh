#!/bin/bash

TEST_NUMBER="0"
EXEC="../webserv"

function test_config ()
{
	(( TEST_NUMBER++ ))
	echo -e "####################################"
	echo -e "Testing"  "'$1'" "ref : $TEST_NUMBER"
    timeout 1 $EXEC "$1"
    RET=$?
    echo -e "RET:$RET"
	echo -e "####################################"
}

echo "##### PARSING GLOBAL CONFIG #####"
for i in `ls bad_conf/**/*`
do
	test_config "$i"
done