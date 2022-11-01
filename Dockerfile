FROM debian:buster
RUN apt-get update -y && apt-get -y install make clang libreadline-dev curl vim valgrind
WORKDIR /webserv
ENTRYPOINT ["./entrypoint.sh"]
CMD ["./webserv", "conf/default.config"]
