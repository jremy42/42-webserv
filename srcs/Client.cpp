#include "Client.hpp"

Client::Client()
{
	_clientFd = 0;
}

Client::Client(int clientFd)
{
	_clientFd = clientFd;
	std::cout << "create client with fd :" << _clientFd << std::endl;
}

Client::~Client(){};

Client::Client(const Client & src)
{
	if (this != &src)
	{
		*this = src;
	}
};

Client &Client::operator=(const Client &src)
{
	if (this != &src)
	{
		_clientFd = src._clientFd;
		_state = src._state;
		_availableActions = src._availableActions;

	}
	return *this;
};

void	Client::setAvailableActions(int epollFlags)
{
	_availableActions = epollFlags;
}

int Client::executeAction()
{
	printf(" Client_fd:[%d], events [%s][%s][%s][%s][%s]\n", _clientFd,
		(_availableActions & EPOLLIN) ? "EPOLLIN " : "",
 		(_availableActions & EPOLLOUT) ? "EPOLLOUT " : "",
 		(_availableActions & EPOLLERR) ? "EPOLLERR " : "",
		(_availableActions & EPOLLRDHUP) ? "EPOLLRDHUP " : "",
		(_availableActions & EPOLLHUP) ? "EPOLLHUP " : "");
	return 0;
}

int Client::getClientFd(void) const
{
	return _clientFd;
}