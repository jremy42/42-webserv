#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# define MAX_CLIENT 5

class Webserv
{
	public: 
		Webserv();
		~Webserv();


	private: 
};

#endif
