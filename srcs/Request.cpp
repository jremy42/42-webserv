# include "Request.hpp"

std::string Request::_requestLineField[REQUEST_LINE_FIELD] = {"method", "request_target", "http_version"};

std::string Request::_headerField[HEADER_FIELD] = {"Host", "User-Agent", "Accept"};

std::string Request::_validRequest[VALID_REQUEST_N] = {"GET", "POST", "DELETE"};

std::string	Request::_stateStr[6] = {"\x1b[32m R_REQUESTLINE\x1b[0m", "\x1b[34m R_HEADER\x1b[0m",
"\x1b[35mR_BODY\x1b[0m", "\x1b[31mR_END\x1b[0m", "\x1b[31mR_ERROR\x1b[0m", "\x1b[31mR_ZERO_READ\x1b[0m"};

Request::Request(void)
{
	_state = R_REQUESTLINE;
	_clientFd = -1;
	_statusCode = 200;
}

Request::Request(int clientFd)
{
	_state = R_REQUESTLINE;
	_clientFd = clientFd;
	_statusCode = 200;
}

Request::Request(const Request &src)
{
	*this = src;
}

Request::~Request(void)
{

}

Request	&Request::operator=(const Request &rhs)
{
	_state = rhs._state;
	_clientFd = rhs._clientFd;
	_header = rhs._header;
	_body = rhs._body;
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
	return (0);
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
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	if (DEBUG_REQUEST)
		std::cout << "Handle Request Line" << std::endl;
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
	for (; it != ite; it++)
	{
		//std::cout << static_cast<int>(*it) << ":[" << *it << "]" << std::endl;
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
			_rawRequest.erase(_rawRequest.begin(), it + 2);
			_state = R_BODY;
			return ;
		}
	}
};

int Request::readClientRequest(void)
{
	std::string	rawRequestLine;
	char		buf[READ_BUFFER_SIZE];
	int			read_ret;
	//char		*next_nl;
	//char		*headerStart;

	if (DEBUG_REQUEST)
		std::cout << "Request State at beginning of readClientRequest :" <<  getStateStr() << std::endl;
	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE);
	if (read_ret == -1)
		throw (std::runtime_error(strerror(errno)));
	if (DEBUG_REQUEST)
	{
		std::cout << "\x1b[33mREAD BUFFER START : [" << read_ret << "] bytes on fd [" << _clientFd
			<< "]\x1b[0m" << std::endl << buf << std::endl
			<< "\x1b[33mREAD BUFFER END\x1b[0m" << std::endl;
	}
	for (int i = 0; i < read_ret; i++)
		_rawRequest.push_back(buf[i]);
	if (_state == R_REQUESTLINE)
		_handleRequestLine();
	if (_state == R_HEADER)
		_handleHeader();
	if (read_ret < READ_BUFFER_SIZE && _state == R_BODY)
		_state = R_END;
	if (read_ret == 0)
		_state = R_ZERO_READ;
	if (DEBUG_REQUEST)
		std::cout << "Request State at end of readClientRequest : [" << _state << "][" <<  getStateStr()
	<< "]" << std::endl;
	return (_state);
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
	return (_requestLine.find("method")->second);
}

std::string Request::getTarget(void) const
{
	return (_requestLine.find("request_target")->second);
}


std::string Request::getProtocol(void) const
{
	return (_requestLine.find("http_version")->second);
}

int	Request::getStatusCode(void) const
{
	return _statusCode;
}

std::string	Request::getHost(void) const
{
	return (_header.find("Host")->second);
}

void Request::reset(void)
{
	
	*this = Request(_clientFd);
}
