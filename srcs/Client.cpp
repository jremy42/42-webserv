#include "Client.hpp"

std::string	Client::_stateStr[4] = {"S_INIT", "S_REQREAD", "S_RESWRITE", "S_OVER"};

Client::Client() : _request(-1)
{
	_clientFd = 0;
	_state = S_INIT;
}

Client::Client(int clientFd) : _request(clientFd), _response(clientFd)
{
	_clientFd = clientFd;
	std::cout << "create client with fd :" << _clientFd << std::endl;
	_state = S_INIT;
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
	int	actionReturnValue;

	std::cout << "Client State at beginning of executeAction :" <<  getStateStr() << std::endl;
	printf(" Client_fd:[%d], events [%s][%s][%s][%s][%s]\n", _clientFd,
		(_availableActions & EPOLLIN) ? "EPOLLIN " : "",
 		(_availableActions & EPOLLOUT) ? "EPOLLOUT " : "",
 		(_availableActions & EPOLLERR) ? "EPOLLERR " : "",
		(_availableActions & EPOLLRDHUP) ? "EPOLLRDHUP " : "",
		(_availableActions & EPOLLHUP) ? "EPOLLHUP " : "");
	if ((_availableActions & EPOLLIN)
		&& (_state == S_INIT || _state == S_REQREAD))
	{
		_state = S_REQREAD;
		actionReturnValue = _request.readClientRequest();
		if (actionReturnValue == R_END || actionReturnValue == R_ERROR)
			_state = S_RESWRITE;
	}
	else if((_availableActions & EPOLLOUT) && _state == S_RESWRITE)
	{
		
		_response.setRequest(&_request);
		_response.createResponse();
		_response.writeClientResponse();
		//Code idem if precedent mais pour la response
		std::cout << "Response write a implementer" << std::endl;
	}
	std::cout << "Client State at end of executeAction :" <<  getStateStr() << std::endl;
	return 0;
}

int Client::getClientFd(void) const
{
	return _clientFd;
}

int Client::getState(void) const
{
	return (_state);
}

std::string &Client::getStateStr(void) const
{
	return(_stateStr[_state]);
}
