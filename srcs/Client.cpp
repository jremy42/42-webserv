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
	printTimeDebug(DEBUG_CLIENT, "A client is born", "");
}

Client::Client(int clientFd, v_config* config, Server *myServ, unsigned int host)
{
	_clientFd = clientFd;
	_request = NULL;
	_response = NULL;
	_configList = _matchingConfigListByHost(config, host);
	printTimeDebug(DEBUG_CLIENT, "A client is born", "");
	printLog(1, _clientFd, 1, "connection accepted");
	if (DEBUG_CLIENT)
	{
		std::cerr << "create client with fd :" << _clientFd << std::endl;
		std::cerr << "====================================================================================" << std::endl;
		std::cerr << "create client with configList:" << std::endl;
		std::cerr << _configList << std::endl;
		std::cerr << "====================================================================================" << std::endl;
	}
	_state = S_INIT;
	_myServ = myServ;
	_timeoutRequest = 0;
	_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
}

Client::~Client()
{
	close(_clientFd);
	delete _response;
	delete _request;
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
	for (it = configList->begin(); it != configList->end(); it++)
		{
			if (it->getHost() == 0)
			{
				printTimeDebug(DEBUG_CLIENT, " add new config for wildcard", "");
				ret.push_back(*it);
			}
		}
	for (it = configList->begin(); it != configList->end(); it++)
	{
		if (it->getHost() == host)
		{
			printTimeDebug(DEBUG_CLIENT, " add new config by host", "");
			ret.push_back(*it);
		}
	}
	if (DEBUG_CLIENT)
	{
		std::cerr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
		std::cerr << "ret: \n";
		std::cerr << ret << std::endl;
		std::cerr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
	}
	return ret;
}


int Client::executeAction()
{
	int	actionReturnValue;

	if (DEBUG_CLIENT)
	{
		printTimeDebug(DEBUG_CLIENT, "Client State at beginning of executeAction :", getStateStr());
		printAvailableAction(DEBUG_CLIENT,_clientFd, _availableActions);
	}
	if (_availableActions & EPOLLOUT && ((ft_get_time() > _timeoutClient && _state == S_RESWRITE ) || (ft_get_time() > _timeoutRequest && _state == S_REQREAD)))
	{
		if (_response != NULL)
			delete _response;
		_response = new Response(_clientFd, _request, _request->getConfig(), 408);
		_timeoutClient = ft_get_time() + TIMEOUT_CLIENT / 2;
		_state = S_RESWRITE;
		return 1;
	}
	if (_availableActions & EPOLLERR || _availableActions & EPOLLHUP || ft_get_time() > _timeoutClient)
	{
		if (_response != NULL)
		{
			delete _response;
			_response = NULL;
		}
		if (_request != NULL)
		{
			delete _request;
			_request = NULL;
		}
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
		actionReturnValue = _request->handleRequest();
		if (DEBUG_CLIENT)
			std::cerr << "timeout request [" << _timeoutRequest << "] : " << (ft_get_time() > _timeoutRequest ? "OVER" : "CONTINUE") <<  std::endl;	
		if (actionReturnValue == R_ZERO_READ)
			_state = S_CLOSE_FD;
		else if (actionReturnValue == R_END || actionReturnValue == R_ERROR )
		{
			printLog(1,_clientFd, 1, _request->getLog().c_str());
			if (ft_get_time() > _timeoutRequest)
				_response = new Response(_clientFd, _request, _request->getConfig(), 408); 
			else
				_response = new Response(_clientFd, _request, _request->getConfig(), _request->getStatusCode()); 
			_state = S_RESWRITE;
		}
	}
	else if ((_availableActions & EPOLLOUT) && _state == S_RESWRITE)
	{
		int retHandleResponse = _response->handleResponse();
		if (retHandleResponse == 0) 
		{
			_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
			_state = S_OVER;
		}
		else if ( retHandleResponse < 0)
		{
			printLog(1,_clientFd, 2, "RESPONSE :", _response->getlineStatus().c_str());
			delete _response;
			_response = NULL;
			_state = S_CLOSE_FD;
		}
	}
	if(_state == S_OVER)
	{
		printLog(1,_clientFd, 2, "RESPONSE :", _response->getlineStatus().c_str());
		delete _request;
		_request = NULL;
		delete _response;
		_response = NULL;
		_state = S_INIT;
	}
	if(_state == S_CLOSE_FD)
	{
		delete _request;
		_request = NULL;
	}
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
