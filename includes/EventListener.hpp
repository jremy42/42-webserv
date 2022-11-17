#ifndef EVENTLISTENER_HPP
# define EVENTLISTENER_HPP

# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <cerrno>
# include <vector>
# include <map>
# define MAX_FD 100

# ifndef DEBUG_EVENTLISTENER
#  define DEBUG_EVENTLISTENER 1
# endif

class EventListener
{
	public:
		EventListener();
		~EventListener();
		EventListener(const EventListener &src);
		EventListener &operator=(const EventListener &src);
		void trackNewFd(int fd, int option);
		std::map<int, int> fdAvailable();
		int getClientFlag(int fd) const;
		void printEvent(int time_sleep) const;

	private:
		struct epoll_event	_evlist[MAX_FD];
		int					_epfd;
		int					_fd_available;
};

#endif
