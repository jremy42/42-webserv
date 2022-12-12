#!/bin/bash

function test_route ()
{
	echo -e "####################################"
	echo -e "Testing"  "$1" "$2" "ref : $TEST_NUMBER"
	FILE="$1";
	shift
	curl -L $* > .ret 2> .err
	diff .ret $FILE > .diff
	test -s .diff  &&  echo -e "\e[31mTest $TEST_NUMBER failed\e[0m" || echo -e "\e[32mTest $TEST_NUMBER passed\e[0m"
	test -s .diff && cat .diff && exit 1
}




test_route 	"../www/site1/index.html" "http://localhost:5001" 
test_route 	"../www/site1/simple.html" "http://localhost:5001/simple.html" 
test_route 	"auto_index_photo" "http://localhost:5001/photo" 
test_route 	"../www/site1/post.html" "http://localhost:5001/post.html" 
test_route 	"../www/site1/delete.html" "http://localhost:5001/delete.html" 
test_route 	"echo_sh" "http://localhost:5001/echo.sh" 
test_route 	"404_default" "http://localhost:5001/coucou" 
test_route 	"../www/site1/404.html" "http://localhost:5001/404custom/toto" 

test_route 	"../www/site2/index.html" "http://e2r1p10:5001" 
test_route 	"../www/site2/simple.html" "http://e2r1p10:5001/simple.html" 
test_route 	"auto_index_photo" "http://e2r1p10:5001/photo" 
test_route 	"../www/site2/post.html" "http://e2r1p10:5001/post.html" 
test_route 	"../www/site2/delete.html" "http://e2r1p10:5001/delete.html" 
test_route 	"echo_sh" "http://e2r1p10:5001/echo.sh" 
test_route 	"404_default" "http://e2r1p10:5001/coucou" 
test_route 	"../www/site2/404.html" "http://e2r1p10:5001/404custom/toto" 


test_route 	"../www/site5/index.html" "http://172.17.0.1:5001" 
test_route 	"../www/site5/simple.html" "http://172.17.0.1:5001/simple.html" 
test_route 	"auto_index_photo" "http://172.17.0.1:5001/photo" 
test_route 	"../www/site5/post.html" "http://172.17.0.1:5001/post.html" 
test_route 	"../www/site5/delete.html" "http://172.17.0.1:5001/delete.html" 
test_route 	"../www/site5/echo.sh" "http://172.17.0.1:5001/echo.sh" 
test_route 	"../www/site5/info.php" "http://172.17.0.1:5001/info.php" 
test_route 	"404_default" "http://172.17.0.1:5001/coucou" 
test_route 	"404_default" "http://172.17.0.1:5001/404custom/toto" 

test_route "../www/test_page/dir2/2" "-H Host:test_page" "http://localhost:5001/dir1/dir3/2" 
test_route "../www/test_page/1" "-H Host:test_page" "http://localhost:5001/redirect1"  
