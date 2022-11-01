#include "Epoll.hpp"

Epoll::Epoll()
{
	epfd = epoll_create(100);
	http_fd  = open("request", O_RDONLY | O_APPEND, 00644);
	int r = read(http_fd, wbuff, 1000);
	printf("r = %d\n",r);
	wbuff[r] = 0;

}
Epoll::~Epoll(){};

Epoll::Epoll(const Epoll & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

const Epoll &Epoll::operator=(const Epoll &src)
{
	if (this != &src)
	{
		epfd = src.epfd;
		ready = src.ready;
		fd = src.fd;
		ev = src.ev;
		for(int i = 0; i < MAX_EVENTS; i++)
		{
			evlist[i] = src.evlist[i];
		}
		for(int i = 0; i < MAX_BUFF; i++)
		{
			buff[i] = src.buff[i];
		}
	}
	return *this;
};
int Epoll::add_fd(int fd, int option)
{
	ev.events = option;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
 		return (-1);
	return (0);
};

int Epoll::fd_ready()
{
	int rv = 0;
	ready = 0;
	ready = epoll_wait(epfd, evlist, MAX_EVENTS, 0);
	for (int i = 0; i < ready; i++)
	{
		if  (evlist[i].events & EPOLLIN || evlist[i].events & EPOLLOUT)
			rv++;
	}
	//printf("ready = %d\n", ready);
	return  rv;
};

int Epoll::print_event()
{
	for (int i = 0; i < ready; i++)
	{
		// printf(" fd:[%d], events [%s][%s][%s][%s]\n", evlist[i].data.fd,
		// (evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
 		// (evlist[i].events & EPOLLOUT) ? "EPOLLOUT " : "",
 		// (evlist[i].events & EPOLLERR) ? "EPOLLERR " : "",
		// (evlist[i].events & EPOLLERR) ? "EPOLLRDHUP " : "");
	}
	//sleep(1);
	return 0;
};

int Epoll::read_fd()
{
	int r;
	std::string request;

	r = 0;

	for (int i = 0; i < ready; i++)
	{
		
		if (evlist[i].events & EPOLLIN)
		{
			r = read(evlist[i].data.fd, buff, MAX_BUFF);
			if (r == -1)
				return (-1);
			buff[r] = 0;
			//printf("r =[%d]\n", r);
			std::string request(buff);
			std::cout << request;
			//if (request == "get\n")
			//{
				if (r == 0)
					close(evlist[i].data.fd);
				else
				{
					r_fd.insert(std::make_pair(evlist[i].data.fd, "get"));
					//printf("insert\n");
					//printf("%s\n", buff);
				}
		}
		if( evlist[i].events & (EPOLLHUP | EPOLLERR))
		{
			printf(" closing fd %d\n", evlist[i].data.fd);
			r_fd.erase((evlist[i].data.fd));
			close(evlist[i].data.fd);
		}
	}
	return 0;
};

int Epoll::write_fd()
{
	int r;

	r = 0;

	for (int i = 0; i < ready; i++)
	{
		if (evlist[i].events & EPOLLOUT && r_fd.find(evlist[i].data.fd) != r_fd.end())
		{
			//printf("buffer = [%s]\n", wbuff);
			write(evlist[i].data.fd, wbuff, strlen(wbuff));
			r_fd.erase((evlist[i].data.fd));
			//close(evlist[i].data.fd);
		}
	}
	return 0;
};