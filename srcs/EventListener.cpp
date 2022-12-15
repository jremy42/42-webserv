#include "EventListener.hpp"

EventListener::EventListener()
{
	_epfd = epoll_create(100);
	if (_epfd == -1)
		throw(std::runtime_error(strerror(errno)));
    _fd_available = 0;
	if (DEBUG_EVENTLISTENER)
		std::cerr << "create event listener\n";
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

		for(int i = 0; i < MAX_FD; i++)
		{
			_evlist[i] = src._evlist[i];
		}
	}
	return *this;
};

void EventListener::trackNewFd(int fd, int option)
{
    struct epoll_event ev;
	ev.events = option;
	ev.data.fd = fd;
	if (epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw(std::runtime_error(std::string("Webserv:") + strerror(errno)));
};

std::map<int, int> EventListener::fdAvailable()
{
	std::map<int, int> _fdList;
	_fd_available = epoll_wait(_epfd, _evlist, MAX_FD, 0);
	if (_fd_available == -1)
		throw(std::runtime_error(std::string("Webserv:") + strerror(errno)));
	for (int i = 0; i < _fd_available; i++)
		_fdList.insert(std::pair<int, int>(_evlist[i].data.fd, _evlist[i].events));
	return  _fdList;
};

void EventListener::printEvent(int time_sleep) const
{
	for (int i = 0; i < _fd_available; i++)
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
	for (int i = 0; i < MAX_FD; i++)
	{
		if(_evlist[i].data.fd == fd)
			return _evlist[i].events;
	}
    return -1;
};

