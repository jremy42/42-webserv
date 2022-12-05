FROM debian:buster
RUN apt-get update -y && apt-get -y install make clang valgrind php-cgi
COPY ./docker/my_webserv/entrypoint.sh /
RUN chmod a+x /entrypoint.sh
WORKDIR /webserv
ENTRYPOINT ["/entrypoint.sh"]
CMD [ "./test_config.sh" ]
#CMD ["./webserv", "conf/default.config"]
