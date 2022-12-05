#!/bin/bash


#(bash $1)> /tmp/tmpfile1
#cat /tmp/tmpfile1
#rm -f /tmp/tmpfile1

#Copy body to tmp_stdin file
cp /dev/stdin /tmp/tmp_stdin

#Open files on fd for redirection
exec 3>/tmp/tmp_stdout
exec 4>/tmp/tmp_stderr
exec 5</tmp/tmp_stdin

#Make saves of standard fd
exec 0<&6
exec 7>&1
exec 8>&2

#change standard fd pointing
exec 1>&3
exec 2>&4
exec 5<&0

#execute CGI
source $1

#Restore saves of standard fd
exec 0>&6
exec 1>&7
exec 2>&8

#Closes unused fd
exec 3>&-
exec 4>&-
exec 5<&-
exec 6>&-
exec 7>&-
exec 8>&-

echo -e "Content-Type: text\r"
echo -e "\r"

echo "<!DOCTYPE html>"
echo "<html>"
echo "<head>"
echo "<title>Bash-CGI output</title>"
echo "</head>"
echo "<body>"
echo "<p>"
cat /tmp/tmp_stdout
echo "</p>"
echo "</body>"
echo "</html>"

rm -rf /tmp/tmp_stderr
rm -rf /tmp/tmp_stdout
rm -rf /tmp/tmp_stdin
