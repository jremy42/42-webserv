#!/bin/bash

TEST_NUMBER="0"
EXEC="../webserv"
function test_header ()
{
	(( TEST_NUMBER++ ))
	if [[ "$1" == *"bad_request/timeout/"* ]]
	then
		TIME=6
	fi
	echo -e "####################################"
	echo -e "Testing"  "'$1'" "ref : $TEST_NUMBER"
	(cat "$1" && sleep $TIME)| telnet localhost 8080 > ./.test_stdout 2>./.test_stderr
	RET=$(grep -e "HTTP/1.1 400 Bad Request" -e "HTTP/1.1 405 Method Not Allowed" -e "HTTP/1.1 501 Not Implemented" -e "HTTP/1.1 408 Request Timeout" ./.test_stdout)
	echo -e "####################################"
}

echo "##### LAUNCH TEST#####"

if ! test -z "$1"
then
	TEST_FILES="$1"
	TIME=20
else
	TEST_FILES=`ls bad_request/**/*`
	TIME=0.3
fi

for i in $TEST_FILES
do
	test_header "$i"
	if [ -z "$RET" ]
	then
		echo -e "\e[31mWebserv should have rejected bad	request : $i\e[0m"
		echo "STDOUT"
		test -s ./.test_stdout && cat ./.test_stdout || echo "File empty"
		echo "STDERR"
		test -s ./.test_stderr && cat ./.test_stderr || echo "File empty"
		exit
	fi
done
