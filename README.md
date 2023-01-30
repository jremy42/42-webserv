# 42-webserv [![jremy's 42 webserv Score](https://badge42.vercel.app/api/v2/cl27cprhd001109mercwbbu5l/project/2772872)](https://github.com/JaeSeoKim/badge42)

### Description :

This project is about writing an HTTP server.Our server can:

- Read a configuration file.
- Listen on multiple ports simultaneously.
- Multiple servers can be configured on the same port.
- Parse and serve an HTTP request.
- Interact with a web browser.
- The GET, POST, and DELETE methods are implemented.
- Implement CGI in PHP and SHELL.
- Properly handle errors.

### Team : 

[Fred](https://profile.intra.42.fr/users/fle-blay), [Jonathan](https://profile.intra.42.fr/users/jremy) 

### Usage :

```
 	~/$> git clone https://github.com/mathias-mrsn/webserv.git
	~/$> cd webserv
  	~/webserv$> Make
	~/webserv$> ./webserv [configuration file]
```

### configuration :

```
server {
		root 			./www/site1/;
		listen			127.0.0.1:8080; #config port and ip 
		client_max_body_size	8m; 
		#error_page 		404 404.html;
		#error_page 		413 413.html;

		location /{
			root ./www/site1/;
			allowed_method GET DELETE POST;
			autoindex on;
			index index.html;
			cgi .php /usr/bin/php-cgi;
			cgi .sh ./cgi/bash_cgi.sh;
			upload ./www/site1/upload;
		}
}
```

### What have we learned? :

- How to utilize an nginx configuration file.
- Design of a server utilizing the epoll() function and its framework.
- The HTTP protocol in its entirety.
- The overall operation of a server and its interaction with CGI.

### ressources:

config nginx:
https://www.digitalocean.com/community/tutorials/understanding-nginx-server-and-location-block-selection-algorithms

Design code :
https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

http :
https://developer.mozilla.org/fr/docs/Web/HTTP/Resources_and_specifications

