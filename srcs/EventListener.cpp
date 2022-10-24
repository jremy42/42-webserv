#include "EventListener.hpp"

EventListener::EventListener()
{
	_epfd = epoll_create(100);
    _cli_available = 0;
	std::cout << "create event listener\n";
}

EventListener::~EventListener(){};

EventListener::EventListener(const EventListener & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

EventListener &EventListener::operator=(const EventListener &src)
{
	if (this != &src)
	{
		_epfd = src._epfd;

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
	_cli_available = epoll_wait(_epfd, _evlist, MAX_CLIENT, 0);
	return  _cli_available;
};

void EventListener::printEvent(int time_sleep) const
{
	for (int i = 0; i < _cli_available; i++)
	{
		printf(" fd:[%d], events [%s][%s][%s][%s][%s]\n", _evlist[i].data.fd,
		(_evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
 		(_evlist[i].events & EPOLLOUT) ? "EPOLLOUT " : "",
 		(_evlist[i].events & EPOLLERR) ? "EPOLLERR " : "",
		(_evlist[i].events & EPOLLRDHUP) ? "EPOLLRDHUP " : "",
		(_evlist[i].events & EPOLLHUP) ? "EPOLLHUP " : "");
	}
	sleep(time_sleep);
};

int EventListener::getClientFlag(int fd) const
{

	for (int i = 0; i < MAX_CLIENT; i++)
	{
		if(_evlist[i].data.fd == fd)
			return _evlist[i].events;
	}
    return -1;
};

