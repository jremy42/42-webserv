# include "Request.hpp"

std::string Request::_requestLineField[REQUEST_LINE_FIELD] = {"method", "request_target", "http_version"};

std::string Request::_headerField[HEADER_FIELD] = {"Host", "User-Agent", "Accept"};

std::string Request::_validRequest[VALID_REQUEST_N] = {"GET", "POST", "DELETE"};

std::string	Request::_stateStr[8] = {"\x1b[32m R_REQUESTLINE\x1b[0m", "\x1b[34m R_HEADER\x1b[0m", "\x1b[34m R_SET_CONFIG\x1b[0m",
"\x1b[35mR_INIT_BODY_FILE\x1b[0m", "\x1b[35mR_BODY\x1b[0m", "\x1b[31mR_END\x1b[0m", "\x1b[31mR_ERROR\x1b[0m", "\x1b[31mR_ZERO_READ\x1b[0m"};

Request::Request(void)
{
	_state = R_REQUESTLINE;
	_clientFd = -1;
	_statusCode = 200;
}

Request::Request(int clientFd, v_config* configList )
{
	_state = R_REQUESTLINE;
	_clientFd = clientFd;
	_statusCode = 200;
	_configList = configList;
}

Request::Request(const Request &src)
{
	*this = src;
}

Request::~Request(void)
{
	unlink(_nameBodyFile.c_str());
}

Request	&Request::operator=(const Request &rhs)
{
	_state = rhs._state;
	_clientFd = rhs._clientFd;
	_header = rhs._header;
	_clientMaxBodySize = rhs._clientMaxBodySize;
	_configList = rhs._configList;
	_bodyFileSize = rhs._bodyFileSize;
	_nameBodyFile = rhs._nameBodyFile;
	_config = rhs._config;
	_maxRead = rhs._maxRead;
	_rawRequestString = rhs._rawRequestString;
	_readRet = rhs._readRet;
	return (*this);
}

int Request::checkRequestLine(void)
{
	for (int i = 0; i < VALID_REQUEST_N; i++) 
	{
		if (_requestLine.find("method")->second == _validRequest[i])
			break;
		if (i == VALID_REQUEST_N -1)
		{
			_statusCode = 405;
			return -1;
		}
	}
	if (_requestLine.find("request_target")->second == "")
	{
		_statusCode = 400;
		return -1;
	}

	if (_requestLine.find("http_version")->second != "HTTP/1.1")
	{
		_statusCode = 400;
		return -1;
	}
	return 0;
}

int	Request::parseRequestLine(string rawRequestLine)
{
	std::istringstream	buf(rawRequestLine);
	string				bufExtract;
	int					i = 0;

	std::cout << "Parse Request Line" << std::endl;
	while (std::getline(buf, bufExtract, ' ') && i < 3)
	{
		_requestLine.insert(std::pair<string, string>(_requestLineField[i], bufExtract));
		std::cout << "[" << _requestLineField[i] << "]";
		std::cout << "[" << _requestLine.find(_requestLineField[i])->second << "]" << std::endl;
		i++;
	}
	if (checkRequestLine() == -1)
		return -1;
	return 0;
}

int Request::checkHeader()
{
	if (_requestLine.find("http_version")->second == "HTTP/1.1"
	&& ( _header.find("host") != _header.end()
	|| _header.find("host")->second == ""))
		return -1;
	return 0;
}

int	Request::parseHeader(string rawHeader)
{
	std::istringstream	buf(rawHeader);
	string				bufExtract;
	string				header_key;
	string				header_value;
	std::size_t			colonPos;

	std::cout << "Parse Header" << std::endl;
	while (std::getline(buf, bufExtract, '\n'))
	{
		colonPos = bufExtract.find(':');
		if (colonPos == std::string::npos
				|| colonPos == bufExtract.length() - 1
				|| colonPos == 1)
		{
			_state = R_ERROR;
			return (-1);
		}
		header_key = string(bufExtract.begin(), bufExtract.begin() + colonPos);
		header_value = string(bufExtract.begin() + colonPos + 1, bufExtract.end());
		//std::cout << "BEFORE" "[" << header_key << "][" << header_value << "]" << std::endl;
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		//std::cout << "AFTER" << "[" << header_key << "][" << header_value << "]" << std::endl;
		for (int i = 0; i < 3; i++)
		{
			if (header_key == _headerField[i])
			{
				_header.insert(std::pair<string, string>(header_key, header_value));
				std::cout << "Inserted :" << " new header key-value : [" << header_key << "][" << header_value << "]" << std::endl;
				break;
			}
		}
	}
	if (checkHeader() == -1)
		return -1;
	return (0);
}

void Request::_handleRequestLine(void)
{
	std::string::iterator ite = _rawRequestString.end();
	std::string::iterator it = _rawRequestString.begin();

	std::cout << "Handle Request Line" << std::endl;
	if (_maxRead > MAX_REQUESTLINE_SIZE)
		throw(std::runtime_error("webserv: request : Request Line is too long"));
	for (; it != ite; it++)
	{
		if (*it == '\r' && it + 1 != ite && *(it + 1) == '\n')
		{
			string rawRequestLine(_rawRequestString.begin(), it);
			if(this->parseRequestLine(rawRequestLine) == -1)
				_state = R_ERROR;
			if (_state == R_ERROR)
				return;
			_rawRequestString.erase(_rawRequestString.begin(), it + 2);
			_state = R_HEADER;
			return;
		}
	}
}

void Request::_handleHeader(void)
{
	string::iterator ite = _rawRequestString.end();
	string::iterator it = _rawRequestString.begin();

	if (DEBUG_REQUEST)
		std::cout << "Handle header" << std::endl;
	if (_maxRead > MAX_HEADER_SIZE)
		throw(std::runtime_error("webserv: request : Header is too long"));
	for (; it != ite; it++)
	{
		//std::cout << static_cast<int>(*it) << ":[" << *it << "]" << std::endl;
		if (*it == '\r'
			&& it + 1 != ite && *(it + 1) == '\n'
			&& it + 2 != ite && *(it + 2) == '\r'
			&& it + 3 != ite && *(it + 3) == '\n')
		{
			string rawHeader(_rawRequestString.begin(), it);
			if(this->parseHeader(rawHeader))
				_state = R_ERROR;
			if (_state == R_ERROR)
				return;
			_rawRequestString.erase(_rawRequestString.begin(), it + 4);
			_state = R_SET_CONFIG;
			return ;
		}
	} 
}

void Request::_initBodyFile(void)
{
	_nameBodyFile = _tmpFileName("./tmp/webserv");

	printTimeDebug(DEBUG_REQUEST, "initBodyfile with file", _nameBodyFile);
	_fs.open(_nameBodyFile.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app);
	if (_fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		throw(std::runtime_error(std::string("Failed to open tmpfile body") + strerror(errno)));
	}
	_state = R_BODY;
}

void Request::_handleBody(void)
{
	if (DEBUG_REQUEST)
	{
		std::cout << "Handle body" << std::endl;
		std::cout << "ClientMaxBodySize:" << _clientMaxBodySize << std::endl; 
	}
	if (getFileSize(_nameBodyFile) > _clientMaxBodySize)
		throw(std::runtime_error("webserv: request : Body exceeds client_max_body_size"));
	_fs << _rawRequestString;
	_rawRequestString.clear();
}

int Request::readClientRequest(void)
{
	std::string	rawRequestLine;
	char		buf[READ_BUFFER_SIZE];
	int			read_ret = 0;
	//char		*next_nl;
	//char		*headerStart;

	if (DEBUG_REQUEST)
		std::cout << "Request State at beginning of readClientRequest :" <<  getStateStr() << std::endl;
	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE);
	if (read_ret == -1)
			throw (std::runtime_error(strerror(errno)));
	std::cout << "read ret[" << read_ret << "]" << std::endl;
	/* if (DEBUG_REQUEST)
	{
		std::cout << "\x1b[33mREAD BUFFER START : [" << read_ret << "] bytes on fd [" << _clientFd
		<< "]\x1b[0m" << std::endl << buf << std::endl
		<< "\x1b[33mREAD BUFFER END\x1b[0m" << std::endl;
	} */
	_readRet = read_ret;
	_maxRead += read_ret;
	_rawRequestString += string(buf);
	return read_ret;
}

int Request::getState(void) const
{
	return (_state);
}

std::string &Request::getStateStr(void) const
{
	return( _stateStr[_state]);
}

std::string Request::getMethod(void) const
{
	if(_requestLine.find("method") == _requestLine.end())
		return ("GET");
	return (_requestLine.find("method")->second);
}

std::string Request::getTarget(void) const
{
	if (_requestLine.find("request_target") == _requestLine.end())
		return ("/");
	return (_requestLine.find("request_target")->second);
}


std::string Request::getProtocol(void) const
{
	if(_requestLine.find("http_version") == _requestLine.end())
		return ("HTTP/1.1");
	return (_requestLine.find("http_version")->second);
}

int	Request::getStatusCode(void) const
{
	return _statusCode;
}

std::string	Request::getHost(void) const
{
	if (_header.find("Host") == _header.end())
		return ("");
	return (_header.find("Host")->second);
}

void Request::reset(void)
{
	*this = Request(_clientFd, _configList);
}

void	Request::setClientMaxBodySize(int clientMaxBodySize)
{
	_clientMaxBodySize = clientMaxBodySize;
}

void	Request::setState(int state)
{
	_state = state;
}


int	Request::handleRequest(void)
{
	int ret = 0;

	ret = readClientRequest();

	if (_state == R_REQUESTLINE)
		_handleRequestLine();
	if (_state == R_HEADER)
		_handleHeader();
	if (_state == R_SET_CONFIG)
		_setConfig();
	if (_state == R_INIT_BODY_FILE)
		_initBodyFile();
	if (_state == R_BODY)
		_handleBody();
	if (ret == 0)
		_state = R_ZERO_READ;
	//else if (ret < READ_BUFFER_SIZE && _state == R_BODY)
	//	_state = R_END;	

	//if (DEBUG_REQUEST)																
	std::cout << "Request State at end of readClientRequest : [" << _state << "][" <<  getStateStr()
			<< "]" << std::endl;
	return (_state);

}

const Config	*Request::getRequestConfig(void) const
{
	return _config;
}


void Request::_setConfig(void)
{
	_config = getMatchingConfig();
	_clientMaxBodySize = atoi(_config->getServerInfoMap().find("client_max_body_size")->second[0].c_str());
	if (_requestLine.find("method")->second == "POST")
		_state = R_INIT_BODY_FILE;
	else
		_state = R_END;
}

const Config *Request::getConfig(void) const
{
	return _config;
}

std::string Request::getTmpBodyFile(void) const
{
	return _nameBodyFile;
}


const Config	*Request::getMatchingConfig(void) const
{
	v_config::const_iterator							it = _configList->begin();
	v_config::const_iterator							ite = _configList->end();
	std::vector<std::string>::const_iterator	match;
	std::vector<std::string>					currentCheckedConfig;

	printTimeDebug(DEBUG_REQUEST, "host", _header.find("Host")->second);
	for (; it != ite; it++)
	{
		currentCheckedConfig = it->getServerName();
		match = find(currentCheckedConfig.begin(), currentCheckedConfig.end(), _header.find("Host")->second);
		if (match != currentCheckedConfig.end())
		{
			printTimeDebug(DEBUG_REQUEST, "found a match for requested host/server_name", "");
			printTimeDebug(DEBUG_REQUEST, "Matched", *match);
			return (&(*it));
		}
	}
	printTimeDebug(DEBUG_REQUEST, "No host matching in config : Defaulting to first host/server_name", "");
	return (&_configList->begin()[0]);
}