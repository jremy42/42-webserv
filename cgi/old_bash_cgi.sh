#!/bin/bash

echo -e "Content-Type: text\r"
echo -e "\r"

(bash $1)> /tmp/tmpfile1
cat /tmp/tmpfile1
rm -f /tmp/tmpfile1
