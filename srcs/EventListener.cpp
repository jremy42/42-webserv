#include "EventListener.hpp"

EventListener::EventListener()
{
	_epfd = epoll_create(100);
    _cli_available = 0;
}
EventListener::~EventListener(){};

EventListener::EventListener(const EventListener & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

const EventListener &EventListener::operator=(const EventListener &src)
{
	if (this != &src)
	{
		_epfd = src.epfd;

		for(int i = 0; i < MAX_CLIENT; i++)
		{
			_evlist[i] = src._evlist[i];
		}
	}
	return *this;
};
int EventListener::trackNewClient(int cli_fd, int option)
{
    struct epoll_event ev;
	ev.events = option;
	ev.data.fd = cli_fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, cli_fd, &ev) == -1)
 		return (-1);
	return (0);
};

int EventListener::clientAvailable()
{
    _cli_available = 0;
	_cli_available = epoll_wait(_epfd, _evlist, MAX_CLIENT, 0);
	return  _cli_available;
};

int EventListener::print_event()
{
	for (int i = 0; i < _cli_available; i++)
	{
		printf(" fd:[%d], events [%s][%s][%s][%s]\n", evlist[i].data.fd,
		(evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
 		(evlist[i].events & EPOLLOUT) ? "EPOLLOUT " : "",
 		(evlist[i].events & EPOLLERR) ? "EPOLLERR " : "",
		(evlist[i].events & EPOLLERR) ? "EPOLLRDHUP " : "");
	}
	sleep(1);
	return 0;
};

const struct epoll_event & EventListener::getClientList() const
{
    return _evlist;
};

