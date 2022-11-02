FROM nginx
RUN mkdir www
COPY ./www/ /www/
COPY ./docker/nginx/nginx.conf /etc/nginx/nginx.conf
ENTRYPOINT ["nginx", "-g", "daemon off;"]