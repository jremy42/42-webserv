#ifndef EVENTLISTENER_HPP
# define EVENTLISTENER_HPP

# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# define MAX_CLIENT 5

class EventListener
{
	public:
		EventListener();
		~EventListener();
		EventListener(const EventListener &src);
		EventListener & operator=(const EventListener &src);
		int trackNewClient(int cli_fd, int cli_option);
		int clientAvailable();
		const struct epoll_event & getClientList() const;
		void printEvent(int time_sleep) const;

	private:
		struct epoll_event	_evlist[MAX_CLIENT];
		int					_epfd;
		int					_cli_available;
};

#endif
