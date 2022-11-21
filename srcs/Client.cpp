#include "Client.hpp"

std::string	Client::_stateStr[4] = {"S_INIT", "S_REQREAD", "S_RESWRITE", "S_OVER"};

Client::Client()
{
	_clientFd = 0;
	_request = NULL;
	_response = NULL;
	_state = S_INIT;
	_configList = NULL;
	_timeoutRequest = 0;
	_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
}

Client::Client(int clientFd, v_config* config, Server *myServ)
{
	_clientFd = clientFd;
	_request = NULL;
	_response = NULL;\
	_configList = config;
	std::cout << "create client with fd :" << _clientFd << std::endl;
	_state = S_INIT;
	_myServ = myServ;
	_timeoutRequest = 0;
	_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
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

Config		*Client::getMatchingConfig(void) const
{
	v_config::iterator							it = _configList->begin();
	v_config::iterator							ite = _configList->end();
	std::vector<std::string>::const_iterator	match;
	std::vector<std::string>					currentCheckedConfig;
	
	std::cout << "_request->getHost() : [" << _request->getHost() << "]" << std::endl;
	for (; it != ite; it++)
	{
		currentCheckedConfig = it->getServerName();
		match = find(currentCheckedConfig.begin(), currentCheckedConfig.end(), _request->getHost());
		if (match != currentCheckedConfig.end())
		{
			std::cout << "found a match for requested host/server_name" << std::endl;
			std::cout << "Matched [" << *match << "]" << std::endl;
			return (&(*it));
		}
	}
	std::cout << "No host matching in config : Defaulting to first host/server_name" << std::endl;
	return (&_configList->begin()[0]);
}
int Client::executeAction()
{
	int	actionReturnValue;
	int	actionMade = 0;

	usleep(50000);
	std::cout << "Client State at beginning of executeAction :" <<  getStateStr() << std::endl;
	printf(" Client_fd:[%d], events [%s][%s][%s][%s][%s]\n", _clientFd,
			(_availableActions & EPOLLIN) ? "EPOLLIN " : "",
			(_availableActions & EPOLLOUT) ? "EPOLLOUT " : "",
			(_availableActions & EPOLLERR) ? "EPOLLERR " : "",
			(_availableActions & EPOLLRDHUP) ? "EPOLLRDHUP " : "",
			(_availableActions & EPOLLHUP) ? "EPOLLHUP " : ""); 
	if (_availableActions & EPOLLERR || _availableActions & EPOLLHUP || ft_get_time() > _timeoutClient)
	{
		_state = S_CLOSE_FD;
		return 1;
	}
	if ((_availableActions & EPOLLIN) && (_state == S_INIT))
	{
			_timeoutRequest = ft_get_time() + TIMEOUT_REQUEST;
			_request = new Request(_clientFd);
			_state = S_REQREAD;
	}
	if ((_availableActions & EPOLLIN) && _state == S_REQREAD)
	{
		_state = S_REQREAD;
		actionReturnValue = _request->readClientRequest(1);
		if (actionReturnValue == R_GET_MAX_BODY_SIZE)
		{
			//A faire uniquement quand reqline & header a ete lu pour connaitre host/server_name 
			std::string	target = _request->getTarget();
			int clientMaxBodySize = atoi(getMatchingConfig()->getServerInfoMap().find("client_max_body_size")->second[0].c_str());
			_request->setClientMaxBodySize(clientMaxBodySize);
			if (DEBUG_CLIENT)
				std::cout << "Setting Client Max Body Size to : [" << clientMaxBodySize << "]" << std::endl;
			//A faire uniquement quand reqline & header a ete lu pour connaitre host/server_name
			_request->setState(R_BODY);
			actionReturnValue = _request->readClientRequest(0);
		}
		std::cout << "timeout request [" << _timeoutRequest << "] : " << (ft_get_time() > _timeoutRequest ? "OVER" : "CONTINUE") <<  std::endl;
		if (actionReturnValue == R_END || actionReturnValue == R_ERROR || ft_get_time() > _timeoutRequest)
		{
			//std::cout << "client getrootDir:[" << _config->getRootDir() << "]\n";
			if (ft_get_time() > _timeoutRequest)
				_response = new Response(_clientFd, _request, getMatchingConfig(), 408); // passer la bonne config
			else
				_response = new Response(_clientFd, _request, getMatchingConfig(), _request->getStatusCode()); // passer la bonne config
			_state = S_RESWRITE;
			_response->createResponse();
		}
		if (actionReturnValue == R_ZERO_READ)
			_state = S_CLOSE_FD;
		actionMade++;
	}
	else if ((_availableActions & EPOLLOUT) && _state == S_RESWRITE)
	{
		if (_response->writeClientResponse() == 0)
		{
			_timeoutClient = ft_get_time() + TIMEOUT_CLIENT;
			_state = S_OVER;
		}
		actionMade++;
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
	//std::cout << "Client State at end of executeAction :" <<  getStateStr() << std::endl;
	return (actionMade);
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
