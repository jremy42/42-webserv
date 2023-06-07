
<div align="center">
<h1 align="center">
<img src="https://raw.githubusercontent.com/PKief/vscode-material-icon-theme/ec559a9f6bfd399b82bb44393651661b08aaf7ba/icons/folder-markdown-open.svg" width="100" />
<br>
42-webserv <a href="https://github.com/JaeSeoKim/badge42"><img src="https://badge42.vercel.app/api/v2/cl27cprhd001109mercwbbu5l/project/2772872" alt="jremy's 42 webserv Score" /></a>
</h1>
<h3 align="center">📍 It's time to understand why URLs start with HTTP!</h3>
<h3 align="center">⚙️ Developed with the software and tools below:</h3>

<p align="center">
<img src="https://img.shields.io/badge/GNU%20Bash-4EAA25.svg?style=for-the-badge&logo=GNU-Bash&logoColor=white" alt="GNU%20Bash" />
<img src="https://img.shields.io/badge/JavaScript-F7DF1E.svg?style=for-the-badge&logo=JavaScript&logoColor=black" alt="JavaScript" />
<img src="https://img.shields.io/badge/HTML5-E34F26.svg?style=for-the-badge&logo=HTML5&logoColor=white" alt="HTML5" />
<img src="https://img.shields.io/badge/PHP-777BB4.svg?style=for-the-badge&logo=PHP&logoColor=white" alt="PHP" />
<img src="https://img.shields.io/badge/Docker-2496ED.svg?style=for-the-badge&logo=Docker&logoColor=white" alt="Docker" />
<img src="https://img.shields.io/badge/Markdown-000000.svg?style=for-the-badge&logo=Markdown&logoColor=white" alt="Markdown" />
</p>
</div>


## 📍 Overview

The 42-webserv project is a C++ web server that provides a robust and efficient solution for handling client requests. It includes features such as server and location configuration, support for HTTP methods (such as GET, POST, and DELETE), handling of cookies and file uploads, and CGI support. Its value proposition lies in its ability to handle large volumes of client requests while providing the necessary flexibility to customize server configuration according to specific needs.

---

## 💫 Features

- Read a configuration file.
- Listen on multiple ports simultaneously.
- Multiple servers can be configured on the same port.
- Parse and serve an HTTP request.
- Interact with a web browser.
- The GET, POST, and DELETE methods are implemented.
- Implement CGI in PHP and SHELL.
- Properly handle errors.

---


<img src="https://raw.githubusercontent.com/PKief/vscode-material-icon-theme/ec559a9f6bfd399b82bb44393651661b08aaf7ba/icons/folder-github-open.svg" width="80" />

## 📂 Project Structure


```bash
repo
├── Makefile
├── README.md
├── cgi
│   └── bash_cgi.sh
├── includes
│   ├── Client.hpp
│   ├── Config.hpp
│   ├── EventListener.hpp
│   ├── Location.hpp
│   ├── Multipart.hpp
│   ├── Request.hpp
│   ├── Response.hpp
│   ├── Server.hpp
│   ├── Webserv.hpp
│   └── _utils.hpp
├── srcs
    ├── Client.cpp
    ├── Config.cpp
    ├── EventListener.cpp
    ├── Location.cpp
    ├── Multipart.cpp
    ├── Request.cpp
    ├── Response.cpp
    ├── Server.cpp
    ├── Webserv.cpp
    ├── _utils.cpp
    └── main.cpp


```

---

<img src="https://raw.githubusercontent.com/PKief/vscode-material-icon-theme/ec559a9f6bfd399b82bb44393651661b08aaf7ba/icons/folder-src-open.svg" width="80" />


---

## 🚀 Getting Started

### ✅ Prerequisites

Before you begin, ensure that you have the following prerequisites installed:
- Make
- gcc

### 🖥 Installation

1. Clone the 42-webserv repository:
```sh
git clone https://github.com/jremy42/42-webserv
```

2. Change to the project directory:
```sh
cd 42-webserv
```

3. compiling the project:
```sh
make
```

4. launch webserv:
```sh
./webserv ./webserv [configuration file]
```

### 🤖 Using 42-webserv

1. launch webserv:
```sh
./webserv ./webserv [configuration file]
```

configuration file :
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
- root specifies the root directory for serving files.

- listen defines the IP address and port number for the server to listen on.

- client_max_body_size sets the maximum allowed size for client request bodies.

- error_page specifies the error pages to be displayed for specific HTTP error codes.

- location / defines the location block for the root directory. It specifies various directives within it:
    - allowed_method lists the HTTP methods allowed for this location. Only GET, DELETE, and POST are allowed.
    - autoindex enables the automatic generation of directory listings when no index file is found.
    - index specifies the default index file to serve if available.
    - cgi configures the handling of CGI scripts with their corresponding interpreter.
    - upload specifies the directory where uploaded files will be stored.

### 🧪 Running Tests

for testing parsing config :
```sh
./tester_webserv/test_config.sh
```

for testing request :
```sh
./tester_webserv/test_header.sh
```

for testing route configuration : 
```sh
./tester_webserv/test_route.sh
---
## 👏 Acquired knowledge

HTTP Protocol:

- ✅  HTTP (Hypertext Transfer Protocol)
    - Application protocol for distributed, collaborative, and hypermedia information systems
    - Communication between client and server using HTTP

- ✅  Web Server:
    - Storing, processing, and delivering web pages to clients
    - Responding to client requests with the content of requested resources
    - Serving HTML documents, including images, stylesheets, and scripts

- ✅ Client-Server Communication:
    - Initiating communication with a server through HTTP requests
    - Server responding with the requested resource or an error message
    - Handling different HTTP methods like GET, POST, and DELETE
    - Support for cookies and session management


- ✅ Server Configuration:
    - Using a configuration file to set up the server
    - Configuring server properties such as port and host
    - Defining server names, error pages, and other settings
    - Enabling/disabling directory listing and redirecting HTTP requests

- ✅ File Handling:
    - Serving static websites and downloading files
    - Setting default files for directory requests
    - Executing CGI (Common Gateway Interface) scripts based on file extensions
    - Handling fragmented requests and CGI output
    - Multiple CGI management

- ✅ Server Resilience:
    - Implementing non-blocking I/O using select(), poll(), epoll(), or equivalent
    - Ensuring the server does not block indefinitely
    - Stress testing the server for availability under high load

## 😎 Team :

[Fred](https://profile.intra.42.fr/users/fle-blay), [Jonathan](https://profile.intra.42.fr/users/jremy) 
