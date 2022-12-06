# include "Request.hpp"

std::string Request::_requestLineField[REQUEST_LINE_FIELD] = {"method", "request_target", "http_version"};

std::string Request::_headerField[HEADER_FIELD] = {"Host", "User-Agent", "Accept"};

std::string Request::_validRequest[VALID_REQUEST_N] = {"GET", "POST", "DELETE"};

std::string	Request::_stateStr[10] = {"\x1b[32m R_REQUESTLINE\x1b[0m", "\x1b[34m R_HEADER\x1b[0m", "\x1b[34m R_SET_CONFIG\x1b[0m",
"\x1b[35mR_INIT_BODY_FILE\x1b[0m" ,"\x1b[35mR_BODY\x1b[0m", "\x1b[35mR_BOUNDARY_HEADER\x1b[0m", "\x1b[31mR_END\x1b[0m", "\x1b[31mR_ERROR\x1b[0m", "\x1b[31mR_ZERO_READ\x1b[0m"};

Request::Request(void)
{
	_state = R_REQUESTLINE;
	_clientFd = -1;
	_statusCode = 200;
	_totalRead= 0;
}

Request::Request(int clientFd, v_config* configList )
{
	_state = R_REQUESTLINE;
	_clientFd = clientFd;
	_statusCode = 200;
	_configList = configList;
	_totalRead= 0;
	_nameBodyFile = "";
	_header.insert(std::pair<string, string>("Host", "no host"));

}

Request::Request(const Request &src)
{
	*this = src;
}

Request::~Request(void)
{
	if (_nameBodyFile.size() > 0)
	{
		_fs.close();
		if (unlink(_nameBodyFile.c_str()) == -1)
		{
			std::cerr << "unlink error" << std::endl;
			std::cerr << "errno: " << strerror(errno) << std::endl;
		}
	}
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
	_totalRead= rhs._totalRead;
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
		_statusCode = 505;
		return -1;
	}
	return 0;
}

int	Request::parseRequestLine(string rawRequestLine)
{
	std::istringstream	buf(rawRequestLine);
	string				bufExtract;
	int					i = 0;

	while (std::getline(buf, bufExtract, ' ') && i < 3)
	{
		_requestLine.insert(std::pair<string, string>(_requestLineField[i], bufExtract));
		if (DEBUG_REQUEST)
		{
			std::cout << "[" << _requestLineField[i] << "]";
			std::cout << "[" << _requestLine.find(_requestLineField[i])->second << "]" << std::endl;
		}
		i++;
	}
	if (checkRequestLine() == -1)
		return -1;
	return 0;
}

int Request::checkHeader()
{
	if (_requestLine.find("http_version")->second == "HTTP/1.1"
	&& (_header.find("Host")->second == "no host"))
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

	while (std::getline(buf, bufExtract, '\n'))
	{
		colonPos = bufExtract.find(':');
		if (colonPos == std::string::npos
				|| colonPos == bufExtract.length() - 1
				|| colonPos == 1)
		{
			_statusCode = 400;
			_state = R_ERROR;
			return (-1);
		}
		header_key = string(bufExtract.begin(), bufExtract.begin() + colonPos);
		header_value = string(bufExtract.begin() + colonPos + 1, bufExtract.end());
		//std::cout << "BEFORE" "[" << header_key << "][" << header_value << "]" << std::endl;
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		//std::cout << "AFTER" << "[" << header_key << "][" << header_value << "]" << std::endl;
		if (header_key == "Host" && _header.find("Host")->second == "no host")
			_header.erase("Host");
		_header.insert(std::pair<string, string>(header_key, header_value));
		if (DEBUG_REQUEST)
			std::cerr << "Inserted :" << " new header key-value : [" << header_key << "][" << header_value << "]" << std::endl;
	}
	if (checkHeader() == -1)
	{
		_statusCode = 400;
		_state = R_ERROR;
		return -1;
	}
	return (0);
}

void Request::_handleRequestLine(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
		std::cerr << "Handle Request Line" << std::endl;
	if (_rawRequest.size() > MAX_REQUESTLINE_SIZE)
	{
				_state = R_ERROR;
				_statusCode = 400;
				return ;
	}
	for (; it != ite; it++)
	{
		if (!isprint(*it) && (*it) != '\r' && (*it) != '\n')
		{
			std::cerr << "Request Line is not printable with char :["<< (int)(*it) << "]"<< std::endl;
			_state = R_ERROR;
			_statusCode = 501;
			return ;
		}
		if (*it == '\r' && it + 1 != ite && *(it + 1) == '\n')
		{
			string rawRequestLine(_rawRequest.begin(), it);
			_rawRequestLine = rawRequestLine;
			if(this->parseRequestLine(rawRequestLine) == -1)
			{
				_state = R_ERROR;
				_statusCode = 400;
			}
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
	{
		_state = R_ERROR;
		_statusCode = 400;
		return ;
	}
	for (; it != ite; it++)
	{
		if (*it == '\r'
				&& it + 1 != ite && *(it + 1) == '\n'
				&& it + 2 != ite && *(it + 2) == '\r'
				&& it + 3 != ite && *(it + 3) == '\n')
		{
			string rawHeader(_rawRequest.begin(), it);
			if (this->parseHeader(rawHeader))
			{
				_state = R_ERROR;
				_statusCode = 400;
				return ;
			}
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
	_nameBodyFile = _tmpFileName("./tmp/webserv");
	printTimeDebug(DEBUG_REQUEST, "initBodyfile with file", _nameBodyFile);
	_fs.open(_nameBodyFile.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::app | std::ifstream::in);
	if (_fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		throw(std::runtime_error(std::string("Failed to open tmpfile body :") + strerror(errno)));
	}
	printTimeDebug(DEBUG_REQUEST, "Boundary", _boundary);
	printTimeDebug(DEBUG_REQUEST, "Content-Length", itoa(_contentLength));
	_state = R_BODY;
}

int Request::_parseHeaderForBody(void)
{

	string rawContentType = _header.find("Content-Type") != _header.end() ? _header.find("Content-Type")->second : "";
	string rawContentLength = _header.find("Content-Length") != _header.end() ? _header.find("Content-Length")->second : "";
	string rawTransferEncoding = _header.find("Transfer-Encoding") != _header.end() ? _header.find("Transfer-Encoding")->second : "";

	if (rawContentType.empty() || rawTransferEncoding == "chunked" || rawContentLength.empty())
	{
		_state = R_ERROR;
		_statusCode = 400;
		return 0;
	}
	_parseContentType(rawContentType);
	_contentLength = atoi(rawContentLength.c_str());
	if (_contentLength > _clientMaxBodySize)
	{
		_statusCode = 413;
		_state = R_ERROR;
		return 0;
	}
	if (!_contentType.empty() &&_contentType[0] !=  "multipart/form-data")
		return (1);
	if (!_contentType.empty() &&_contentType[0] == "multipart/form-data" && _contentType.size() > 1)
	{
		size_t pose;
		_boundary = _contentType[1];
		pose = _boundary.find("boundary=", 0);
		_boundary = string(_boundary.begin()  + pose + strlen("boundary="), _boundary.end());
		if (_boundary.empty())
		{
			_state = R_ERROR;
			_statusCode = 400;
			return 0;
		}
		return 1;
	}
	else
	{
		_state = R_END;
		return 0;
	}
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
	{
		_statusCode = 413;
		_state = R_ERROR;
		return;
	}
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
	unsigned char		buf[READ_BUFFER_SIZE];
	ssize_t				read_ret = 0;

	if (DEBUG_REQUEST)
		std::cout << "Request State at beginning of readClientRequest :" <<  getStateStr() << std::endl;
	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE - 1);
	if (read_ret == -1)
		throw (std::runtime_error(strerror(errno)));
	if (DEBUG_REQUEST)
	{
		std::cerr << "\x1b[33mREAD BUFFER START : [" << read_ret << "] bytes on fd [" << _clientFd
		<< "]\x1b[0m" << std::endl;
		std::cerr << "RAW PRINT OF BUFFER START" << std::endl;
		std::cerr << "\e[32mAttempt char decode start\e[0m" << std::endl;
		for (int i = 0; i < read_ret; i++)
		{
			if (buf[i] != '\r')
				fprintf(stderr, "%1c", buf[i]);
		}
		std::cerr << std::endl;
		std::cerr << "\e[32mAttempt char decode end\e[0m" << std::endl;
		std::cerr << "\e[33mRaw char without decode start\e[0m" << std::endl;
		for (int i = 0; i < read_ret; i++)
		{
			if (isprint(buf[i]))
				fprintf(stderr, "%4c", buf[i]);
			else
				fprintf(stderr, "%4u", buf[i]);
			if (i % 16 == 0 && i > 0)
				fprintf(stderr, "\n");
		}
		std::cerr << std::endl;
		std::cerr << "\e[33mRaw char without decode end\e[0m" << std::endl;
		std::cerr << "\e[34mRaw char without decode start HEXA\e[0m" << std::endl;
		for (int i = 0; i < read_ret; i++)
		{
			fprintf(stderr, "%4X", buf[i]);
			if (i % 16 == 0 && i > 0)
				fprintf(stderr, "\n");
		}
		std::cerr << std::endl;
		std::cerr << "\e[34mRaw char without decode end HEXA\e[0m" << std::endl;
		std::cerr << "RAW PRINT OF BUFFER END" << std::endl;
		std::cerr << "Before buffer print" << std::endl;
		std::cerr << buf << std::endl;
		std::cerr << "After buffer print" << std::endl;
		std::cerr << "\x1b[33mREAD BUFFER END\x1b[0m" << std::endl;
	}
	for (int i = 0; i < read_ret; i++)
		_rawRequest.push_back(buf[i]);
	_readRet = read_ret;
	_totalRead+= read_ret;
	return read_ret;
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
	if ( _contentLength <= 0 && _state == R_BODY)
		_state = R_END;
	if (DEBUG_REQUEST)
	{															
		std::cout << "Request State at end of readClientRequest : [" << _state << "][" <<  getStateStr() << "]" << std::endl;
		std::cout << "Max read = [" << _totalRead<< "]" << std::endl;
	}
	return (_state);

}

const Config	*Request::getRequestConfig(void) const
{
	return _config;
}

int Request::_checkAutorizationForMethod(void)
{
	string requestTarget = this->getTarget();
	std::vector<string> allowedMethod = _config->getParamByLocation(requestTarget, "allowed_method");

	if (find(allowedMethod.begin(), allowedMethod.end(), this->getMethod()) == allowedMethod.end())
	{
		_statusCode = 405;
		return 0;
	}
	return 1;
}

void Request::_setConfig(void)
{
	if (DEBUG_REQUEST)
		std::cerr << _header << std::endl;
	_config = getMatchingConfig();
	_clientMaxBodySize = atoi(_config->getServerInfoMap().find("client_max_body_size")->second[0].c_str());
	if (_checkAutorizationForMethod())
	{
		if (_requestLine.find("method") != _requestLine.end() 
		&& _requestLine.find("method")->second == "POST" && _parseHeaderForBody())
		{
			_state = R_INIT_BODY_FILE;
			return;
		}
	}
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
	v_config::const_iterator					it = _configList->begin();
	v_config::const_iterator					ite = _configList->end();
	std::vector<std::string>::const_iterator	match;
	std::vector<std::string>					currentCheckedConfig;
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

Request::string Request::getBoundaryDelim(void) const
{
	return _boundary;
}

Request::string Request::getUploadDir(void) const
{
	string requestTarget = this->getTarget();
	return _config->getParamByLocation(requestTarget, "upload")[0];
}

Request::string Request::getBodyFile(void)
{
	return _nameBodyFile;
}

std::string Request::getLog(void)
{
	std::string logRequest;

	logRequest = "REQUEST : " + _rawRequestLine + "/ host:" + this->getHost();

	return logRequest;
}

Request::v_s Request::getContentType(void) const
{
	return _contentType;
}

/* void Request::_handleBodyChunked(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
	{
		std::cout << "Handle body" << std::endl;
		std::cout << "ClientMaxBodySize:" << _clientMaxBodySize << std::endl; 
	}
	if (getFileSize(_nameBodyFile) > _clientMaxBodySize)
	{
		_statusCode = 413;
		_state = R_ERROR;
		return;
	}
	std::cout << "NextChunkSize:" << _nextChunkSize << std::endl;
	for (; it != ite; it++)
	{
		if (_nextChunkSize == -1 )
		{
			if (DEBUG_REQUEST)
				std::cout << "getNextSize" << std::endl;
			for (; it + 1 != ite; it++)
			{
				if (*it == '\r' && *(it + 1) == '\n')
				{
					std::cout << "rawChunkSize: [" << _rawChunkedSize << "]" << std::endl;
					_nextChunkSize = strtol(_rawChunkedSize.c_str(), NULL, 16);
					_rawChunkedSize.clear();
					std::cout << "_nextChunkSize: [" << _nextChunkSize << "]" << std::endl;
					if (_nextChunkSize == 0)
					{
						_state = R_END;
						_fs.flush();
						_rawRequest.clear();
						return;
					}
					it += 1;
				break;
				}
				else
					_rawChunkedSize.push_back(*it);
			}
			continue;
		}
		_fs << *it;
		_nextChunkSize--;
	}
	_fs.flush();
	_rawRequest.clear();
} */
