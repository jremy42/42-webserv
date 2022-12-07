#!/bin/bash

TEST_NUMBER="0"
EXEC="../webserv"
VALGRIND="valgrind"

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
	test_config "$1"
	if test "$RET" -ne 1
	then
		echo -e "\e[31mWebserv should have rejected conf : $i\e[0m"
		echo "STDOUT"
		test -s ./.test_stdout && cat ./.test_stdout || echo "File empty" 
		echo "STDERR"
		test -s ./.test_stderr && cat ./.test_stderr || echo "File empty" 
		exit
	fi
else

	for i in `ls bad_conf/**/*`
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

fi

rm -rf ./.test_stdout ./.test_stderr
