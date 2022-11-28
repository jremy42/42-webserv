#include "Client.hpp"

std::string	Client::_stateStr[4] = {"S_INIT", "S_REQREAD", "S_RESWRITE", "S_OVER"};

Client::Client()
{
	_clientFd = 0;
	_request = NULL;
	_response = NULL;
	_state = S_INIT;
	_timeoutRequest = 0;
	_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
	std::cout << "A Client is born at [\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
}

Client::Client(int clientFd, v_config* config, Server *myServ, unsigned int host)
{
	_clientFd = clientFd;
	_request = NULL;
	_response = NULL;\
	_configList = _matchingConfigListByHost(config, host);
	std::cout << "create client with fd :" << _clientFd << std::endl;
	std::cout << "====================================================================================" << std::endl;
	std::cout << "create client with configList:" << std::endl;
	std::cout << _configList << std::endl;
	std::cout << "====================================================================================" << std::endl;
	_state = S_INIT;
	_myServ = myServ;
	_timeoutRequest = 0;
	_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
}

Client::~Client()
{
	std::cout << "A client is DEAD at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
}

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
		_request = src._request;
		_response = src._response;
		_configList = src._configList;
		_myServ = src._myServ;
		_timeoutRequest = src._timeoutRequest;
		_timeoutClient = src._timeoutClient;

	}
	return *this;
};

void	Client::setAvailableActions(int epollFlags)
{
	_availableActions = epollFlags;
}

Client::v_config	Client::_matchingConfigListByHost(v_config *configList, unsigned int host)
{
	v_config ret;
	v_config::iterator it = configList->begin();
	for (; it != configList->end(); it++)
	{
		if (it->getHost() == host)
		{
			std::cout << " add new config by host" << std::endl;
			ret.push_back(*it);
		}
	}
	if (ret.empty())
	{
		for (it = configList->begin(); it != configList->end(); it++)
		{
			if (it->getHost() == 0)
			{
				std::cout << " add new config for wildcard" << std::endl;
				ret.push_back(*it);
			}
		}
	}
	std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
	std::cout << "ret: \n";
	std::cout << ret << std::endl;
	std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

	return ret;
}


int Client::executeAction()
{
	int	actionReturnValue;

	//printTimeDebug(DEBUG_CLIENT, "Client State at beginning of executeAction :", getStateStr());
	//printAvailableAction(DEBUG_CLIENT,_clientFd, _availableActions);
	if (_availableActions & EPOLLERR || _availableActions & EPOLLHUP || ft_get_time() > _timeoutClient)
	{
		_state = S_CLOSE_FD;
		return 1;
	}
	if ((_availableActions & EPOLLIN) && (_state == S_INIT))
	{
			_timeoutRequest = ft_get_time() + TIMEOUT_REQUEST;
			_request = new Request(_clientFd, &_configList);
			_state = S_REQREAD;
	}
	if ((_availableActions & EPOLLIN) && _state == S_REQREAD)
	{
		_state = S_REQREAD;
		actionReturnValue = _request->readClientRequest();
		if (DEBUG_CLIENT)
			std::cout << "timeout request [" << _timeoutRequest << "] : " << (ft_get_time() > _timeoutRequest ? "OVER" : "CONTINUE") <<  std::endl;
		if (actionReturnValue == R_END || actionReturnValue == R_ERROR || ft_get_time() > _timeoutRequest)
		{
			if (ft_get_time() > _timeoutRequest)
				_response = new Response(_clientFd, _request, _request->getConfig(), 408); // passer la bonne config
			else
				_response = new Response(_clientFd, _request, _request->getConfig(), _request->getStatusCode()); // passer la bonne config
			_state = S_RESWRITE;
		}
		if (actionReturnValue == R_ZERO_READ)
			_state = S_CLOSE_FD;
	}
	else if ((_availableActions & EPOLLOUT) && _state == S_RESWRITE)
	{
		if (_response->handleResponse() == 0) // 0 si ok et j'ai ecrit, donc je prolonge le timeout
		{
			_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
			_state = S_OVER;
		}
	}
	if(_state == S_OVER)
	{
		delete _request;
		delete _response;
		_state = S_INIT;
	}
	else if(_state == S_CLOSE_FD)
	{
		delete _request;
		_state = S_CLOSE_FD;
	}
	//printTimeDebug(DEBUG_CLIENT, "Client State at end of executeAction", getStateStr());
	//printTimeDebug(DEBUG_CLIENT, "executeAction OUT", "");
	return (1 );
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

Server *Client::getMyServer(void) const
{
	return(_myServ);
}
