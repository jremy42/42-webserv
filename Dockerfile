FROM debian:buster
RUN apt-get update -y && apt-get -y install make clang libreadline-dev
RUN mkdir webserv
COPY ./ webserv/
#COPY ./Makefile webserv/Makefile
#COPY /docker/entrypoint.sh webserv/entrypoint.sh
WORKDIR /webserv
RUN chmod 777 /webserv/entrypoint.sh
#CMD ["yes"]
ENTRYPOINT ["bash","entrypoint.sh"]
CMD [ "webserv" ]