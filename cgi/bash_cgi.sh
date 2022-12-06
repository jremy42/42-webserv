#!/bin/bash

rm -f /tmp/tmp_stderr
rm -f /tmp/tmp_stdout
rm -f /tmp/tmp_stdin
#Copy body to tmp_stdin file
touch /tmp/tmp_stdin
while read -t 0.1 -N 512 -r LINE
do
   echo "$LINE" >> /tmp/tmp_stdin
done

#Open files on fd for redirection
exec 3>/tmp/tmp_stdout
exec 4>/tmp/tmp_stderr
exec 5</tmp/tmp_stdin

#Make saves of standard fd
exec 6<&0
exec 7>&1
exec 8>&2

#change standard fd pointing
exec 1>&3
exec 2>&4
exec 0<&5

#execute CGI
source $1

#Restore saves of standard fd
exec 0<&6
exec 1>&7
exec 2>&8

#Closes unused fd
exec 3>&-
exec 4>&-
exec 5<&-
exec 6<&-
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
echo "<h2>"$(basename "$1")" output :</h2>"
echo "</p>"
echo "<p>"
cat /tmp/tmp_stdout
echo "</p>"
echo "<h2>"$(basename "$1")" error output :</h2>"
echo "</p>"
echo "<p>"
cat /tmp/tmp_stderr
echo "</p>"

echo "</body>"
echo "</html>"

rm -f /tmp/tmp_stderr
rm -f /tmp/tmp_stdout
rm -f /tmp/tmp_stdin
