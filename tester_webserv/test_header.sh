#!/bin/bash

TEST_NUMBER="0"
EXEC="../webserv"

function test_header ()
{
	(( TEST_NUMBER++ ))
	echo -e "####################################"
	echo -e "Testing"  "'$1'" "ref : $TEST_NUMBER"
    cat "$1" | telnet localhost 5002
    RET=$?
    echo -e "RET:$RET"
	echo -e "####################################"
}

echo "##### LAUNCH TEST#####"
for i in `ls bad_request/**/*`
do
	test_header "$i"
done
