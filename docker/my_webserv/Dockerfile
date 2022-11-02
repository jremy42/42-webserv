FROM debian:buster
RUN apt-get update -y && apt-get -y install make clang valgrind
COPY ./docker/my_webserv/entrypoint.sh /
RUN chmod a+x /entrypoint.sh
WORKDIR /webserv
ENTRYPOINT ["/entrypoint.sh"]
CMD ["./webserv", "conf/default.config"]
