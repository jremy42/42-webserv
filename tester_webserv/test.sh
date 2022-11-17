#!/bin/bash

COUNT=600

while (( COUNT ))  
do
	timeout 60 ./a.out localhost $1 &
	echo $COUNT
	(( COUNT-- ))
done