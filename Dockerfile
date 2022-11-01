FROM debian:buster
RUN apt-get update -y && apt-get -y install make clang libreadline-dev curl vim valgrind
WORKDIR /webserv
ENTRYPOINT ["./entrypoint.sh"]
CMD [ "valgrind --leak-check=full",\
		"--show-leak-kinds=all",\
		"./webserv",\
		"conf/default.config" ]
