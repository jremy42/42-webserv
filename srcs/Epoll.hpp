#ifndef EPOLL_HPP
# define EPOLL_HPP

# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <vector>
# include <string.h>
# include <map>
# define MAX_BUFF 10000
# define MAX_EVENTS 5

class Epoll
{
	public:
		typedef std::map<int, std::string> map;
		typedef map::iterator iterator;
		Epoll();
		~Epoll();
		Epoll(const Epoll & src);
		const Epoll &operator=(const Epoll &src);
		int add_fd(int fd, int option);
		int fd_ready();
		int print_event();
		int read_fd();
		int write_fd();
	private:
		int epfd, ready, fd;
		struct epoll_event ev;
		struct epoll_event evlist[MAX_EVENTS];
		char buff[MAX_BUFF];
		map r_fd;
		iterator it;
};

#endif
