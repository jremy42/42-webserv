!#/bin/bash

TEST_NUMBER="0"
EXEC="./webserv"

function test_config ()
{
	(( TEST_NUMBER++ ))
	echo -e "Testing"  "'$1'" "ref : $TEST_NUMBER"
    timeout 1 SIGKILL $EXEC "$1"
    RET=$?
    echo -e "RET:$RET"
}

echo "##### PARSING GLOBAL CONFIG #####"
test_config './conf/bad_conf/bad_bracket.conf'
test_config './conf/bad_conf/bad_server.conf'
test_config './conf/bad_conf/empty_server.conf'