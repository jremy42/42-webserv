#!/bin/bash

function test_route ()
{
	echo -e "####################################"
	echo -e "Testing"  "$1" "$2" "ref : $TEST_NUMBER"
	curl "$1" "$2" > .ret 2> .err
	diff .ret "$3" > .diff
	test -s .diff  &&  echo -e "\e[31mTest $TEST_NUMBER failed\e[0m" || echo -e "\e[32mTest $TEST_NUMBER passed\e[0m"
	test -s .diff && cat .diff && exit 1
}




test_route "-H host:localhost" "http://localhost:5001" "../www/site1/index.html"
test_route "-H host:localhost" "http://localhost:5001" "../www/site1/index.html"

test_route "-H host:youpi" "http://localhost:8080" "../www/site1/info.php"
test_route "-H host:youpi" "http://localhost:8080" "../www/site1/index.html"


