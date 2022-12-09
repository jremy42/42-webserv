#!/bin/bash

TEST_NUMBER="0"
EXEC="../webserv"
#VALGRIND="valgrind"
TEST_FILES=""

function test_config ()
{
	(( TEST_NUMBER++ ))
	echo -e "####################################"
	echo -e "Testing"  "'$1'" "ref : $TEST_NUMBER"
    timeout --preserve-status -s SIGINT 1 stdbuf -o0  $VALGRIND $EXEC "$1" >./.test_stdout 2>./.test_stderr
    RET=$?
    echo -e "RET:$RET"
	echo -e "####################################"
}

echo "##### PARSING GLOBAL CONFIG #####"

if ! test -z "$1"
then
	TEST_FILES="$1"
else
	TEST_FILES=`ls bad_conf/**/*`
fi

for i in $TEST_FILES
do
	test_config "$i"
	if test "$RET" -ne 1
	then
		echo -e "\e[31mWebserv should have rejected conf : $i\e[0m"
		echo "STDOUT"
		test -s ./.test_stdout && cat ./.test_stdout || echo "File empty" 
		echo "STDERR"
		test -s ./.test_stderr && cat ./.test_stderr || echo "File empty" 
		exit
	fi
done

rm -rf ./.test_stdout ./.test_stderr
