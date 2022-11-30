# include "Request.hpp"

std::string Request::_requestLineField[REQUEST_LINE_FIELD] = {"method", "request_target", "http_version"};

std::string Request::_headerField[HEADER_FIELD] = {"Host", "User-Agent", "Accept"};

std::string Request::_validRequest[VALID_REQUEST_N] = {"GET", "POST", "DELETE"};

std::string	Request::_stateStr[9] = {"\x1b[32m R_REQUESTLINE\x1b[0m", "\x1b[34m R_HEADER\x1b[0m", "\x1b[34m R_SET_CONFIG\x1b[0m",
"\x1b[35mR_INIT_BODY_FILE\x1b[0m", "\x1b[35mR_BODY\x1b[0m", "\x1b[35mR_BOUNDARY_HEADER\x1b[0m", "\x1b[31mR_END\x1b[0m", "\x1b[31mR_ERROR\x1b[0m", "\x1b[31mR_ZERO_READ\x1b[0m"};

Request::Request(void)
{
	_state = R_REQUESTLINE;
	_clientFd = -1;
	_statusCode = 200;
	_maxRead = 0;
}

Request::Request(int clientFd, v_config* configList )
{
	_state = R_REQUESTLINE;
	_clientFd = clientFd;
	_statusCode = 200;
	_configList = configList;
	_maxRead = 0;
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
	_rawRequest = rhs._rawRequest;
	_readRet = rhs._readRet;
	return (*this);
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
		_header.insert(std::pair<string, string>(header_key, header_value));
		std::cout << "Inserted :" << " new header key-value : [" << header_key << "][" << header_value << "]" << std::endl;
	}
	if (checkHeader() == -1)
	{
		_statusCode = 400;
		return -1;
	}
	return (0);
}

void Request::_handleRequestLine(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
		std::cout << "Handle Request Line" << std::endl;
	if (_rawRequest.size() > MAX_REQUESTLINE_SIZE)
		throw(std::runtime_error("webserv: request : Request Line is too long"));
	for (; it != ite; it++)
	{
		if (*it == '\r' && it + 1 != ite && *(it + 1) == '\n')
		{
			string rawRequestLine(_rawRequest.begin(), it);
			if(this->parseRequestLine(rawRequestLine) == -1)
				_state = R_ERROR;
			if (_state == R_ERROR)
				return;
			_rawRequest.erase(_rawRequest.begin(), it + 2);
			_state = R_HEADER;
			return;
		}
	}
}

void Request::_handleHeader(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
		std::cout << "Handle header" << std::endl;
	if (_rawRequest.size() > MAX_HEADER_SIZE)
		throw(std::runtime_error("webserv: request : Header is too long"));
	for (; it != ite; it++)
	{
		if (*it == '\r'
			&& it + 1 != ite && *(it + 1) == '\n'
			&& it + 2 != ite && *(it + 2) == '\r'
			&& it + 3 != ite && *(it + 3) == '\n')
		{
			string rawHeader(_rawRequest.begin(), it);
			if(this->parseHeader(rawHeader))
				_state = R_ERROR;
			if (_state == R_ERROR)
				return;
			_rawRequest.erase(_rawRequest.begin(), it + 4);
			_state = R_SET_CONFIG;
			return ;
		}
	} 

}

void Request::_parseContentType(string rawContentType)
{
	std::istringstream	buf(rawContentType);
	string				bufExtract;
	while (std::getline(buf, bufExtract, ';'))
	{
		bufExtract = strtrim(bufExtract, "\f\t\n\r\v ");
		//printTimeDebug(DEBUG_REQUEST, "insert", bufExtract);
		_contentType.push_back(bufExtract);
	}
}

void Request::_initBodyFile(void)
{
	string rawContentType;
	string rawContentLength;


	_nameBodyFile = _tmpFileName("./tmp/webserv");
	printTimeDebug(DEBUG_REQUEST, "initBodyfile with file", _nameBodyFile);
	_fs.open(_nameBodyFile.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app | std::ifstream::in);
	if (_fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		throw(std::runtime_error(std::string("Failed to open tmpfile body") + strerror(errno)));
	}
	if (_header.find("Content-Type") != _header.end())
		rawContentType = _header.find("Content-Type")->second;
	if (rawContentType.empty())
	{
		_state = R_ERROR;
		_statusCode = 400;
		return;
	}
	else
		_parseContentType(rawContentType);
	if (_header.find("Content-Length") != _header.end())
		rawContentLength = _header.find("Content-Length")->second;
	if (rawContentLength.empty())
	{
		_state = R_ERROR;
		_statusCode = 400;
		return;
	}
	else
		_contentLength = atoi(rawContentLength.c_str());
	if (_contentType[0] == "multipart/form-data" && _contentType.size() > 1)
	{
		size_t pose;
		_boundary = _contentType[1];
		pose = _boundary.find("boundary=", 0);
		_boundary = string(_boundary.begin()  + pose + strlen("boundary="), _boundary.end());
		if (_boundary.empty())
		{
			_state = R_ERROR;
			_statusCode = 400;
			return;
		}
	}
	printTimeDebug(DEBUG_REQUEST, "Boundary", _boundary);
	printTimeDebug(DEBUG_REQUEST, "Content-Length", itoa(_contentLength));
	_state = R_BODY;
}

void Request::_handleBody(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
	{
		std::cout << "Handle body" << std::endl;
		std::cout << "ClientMaxBodySize:" << _clientMaxBodySize << std::endl; 
	}
	if (getFileSize(_nameBodyFile) > _clientMaxBodySize)
		throw(std::runtime_error("webserv: request : Body exceeds client_max_body_size"));
	for (; it != ite; it++)
	{
		_fs << *it;
		_contentLength--;
	}
	_fs.flush();
	_rawRequest.clear();
}

int Request::readClientRequest(void)
{
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
	if (DEBUG_REQUEST)
	{
		std::cout << "\x1b[33mREAD BUFFER START : [" << read_ret << "] bytes on fd [" << _clientFd
		<< "]\x1b[0m" << std::endl << buf << std::endl
		<< "\x1b[33mREAD BUFFER END\x1b[0m" << std::endl;
	} 
	for (int i = 0; i < read_ret; i++)
		_rawRequest.push_back(buf[i]);
	_readRet = read_ret;
	_maxRead += read_ret;
	return read_ret;
}

void Request::_extractFileFromBody(void)
{
	string				bufExtract;
	string				header_key;
	string				header_value;
	std::size_t			colonPos;
	string				newBodyFile;
	std::fstream		fsNewBodyFile;

	newBodyFile = _tmpFileName("./tmp/webservNewBodyFile");
	fsNewBodyFile.open(newBodyFile.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app);
	if (fsNewBodyFile.good())
		std::cout << "Successfully opened new body file "<< std::endl;
	else
		throw(std::runtime_error(std::string("Failed to open tmpfile body") + strerror(errno)));
	while(getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == (string("--" + _boundary + "\r")))
		{
			std::cout << "coutinue" << std::endl;
			continue;
		}
		if (bufExtract == "\r")
			break;
		colonPos = bufExtract.find(':');
		if (colonPos == std::string::npos
				|| colonPos == bufExtract.length() - 1
				|| colonPos == 1)
		{
			_statusCode = 400;
			_state = R_ERROR;
			std::cout << "[" << string("--" + _boundary) << "]" << std::endl;
			std::cout << "[" << bufExtract << "]" << std::endl;
			std::cout << "bad request ici" << std::endl;
			exit(1) ;
		}
		header_key = string(bufExtract.begin(), bufExtract.begin() + colonPos);
		header_value = string(bufExtract.begin() + colonPos + 1, bufExtract.end());
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		_boundaryHeader.insert(std::pair<string, string>(header_key, header_value));
		std::cout << "Inserted :" << " new header key-value in Boundary header : [" << header_key << "][" << header_value << "]" << std::endl;

	}
	while (getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == string("--" + _boundary + "--" + "\r"))
			break;		
		bufExtract += "\n";
		std::cout << "inserty[" << bufExtract << std::endl;
		fsNewBodyFile << bufExtract;
	}
	fsNewBodyFile.unget();
	fsNewBodyFile.ignore(1,'\n');
	fsNewBodyFile.flush();
	fsNewBodyFile.close();
	//unlink(_nameBodyFile.c_str());
	_nameBodyFile = newBodyFile;
	printTimeDebug(1, "boundary header:", "");
	std::cout << _boundaryHeader << std::endl;
}

void Request::_handleBoundary(void)
{
	_fs.seekg(0);
	while(_fs.tellg() > 0)
	{
		std::cout << _fs.tellg() << std::endl;
		_extractFileFromBody();
	}
	_fs.close();
	exit(1);
}

int	Request::handleRequest(void)
{
	int ret = 0;

	ret = readClientRequest();

	if (_state == R_REQUESTLINE)
		_handleRequestLine();
	if (_state == R_HEADER)
		_handleHeader();
	if (_state == R_SET_CONFIG || _state == R_ERROR)
		_setConfig();
	if (_state == R_INIT_BODY_FILE)
		_initBodyFile();
	if (_state == R_BODY)
		_handleBody();
	if (ret == 0)
		_state = R_ZERO_READ;
	if (_contentLength <= 0 && _state == R_BODY && _boundary.size() > 0)
		_handleBoundary();
	if ( _contentLength <= 0 && _state == R_BODY)
		_state = R_END;	

	//if (DEBUG_REQUEST)																
	std::cout << "Request State at end of readClientRequest : [" << _state << "][" <<  getStateStr()
			<< "]" << std::endl;
	std::cout << "Max read = [" << _maxRead << "]" << std::endl;
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
	v_config::const_iterator	it = _configList->begin();
	v_config::const_iterator	ite = _configList->end();
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

Request::m_ss Request::getHeader(void) const
{
	return _header;
}
