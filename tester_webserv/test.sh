#!/bin/bash

COUNT=600

while (( COUNT ))  
do
	timeout 60 ./a.out $1 $2 &
	echo $COUNT
	(( COUNT-- ))
done
