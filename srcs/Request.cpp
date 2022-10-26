# include "../Includes/Request.hpp"

std::string Request::_requestLineField[3] = {"method", "request_uri", "http_version"};

std::string Request::_headerField[3] = {"Host", "User-Agent", "Accept"};

std::string Request::_validRequest[3] = {"GET", "POST", "DELETE"};


Request::Request(void)
{
	_state = R_REQUESTLINE;
}

Request::Request(int clientFd)
{
	_state = R_REQUESTLINE;
	_clientFd = clientFd;
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

int	Request::parseRequestLine(string rawRequestLine)
{
	std::istringstream	buf(rawRequestLine);
	string				bufExtract;
	int					i = 0;

	while (std::getline(buf, bufExtract, ' ') && i < 3)
	{
		_requestLine.insert(std::pair<string, string>(_requestLineField[i], bufExtract));
		std::cout << "[" << _requestLineField[i] << "]";
		std::cout << "[" << _requestLine.find(_requestLineField[i])->second << "]" << std::endl;
		i++;
	}
	return (0);
}


void Request::_handleRequestLine(void)
{

	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	for (; it != ite; it++)
	{
		if (*it == '\r' && it + 1 != ite && *(it + 1) == '\n')
		{
			string rawRequestLine(_rawRequest.begin(), it);
			if(this->parseRequestLine(rawRequestLine))
				_state = R_ERROR;
			if (_state == R_ERROR)
				return;
			_rawRequest.erase(_rawRequest.begin(), it + 2);
			_state = R_HEADER;
		}
	}
}

void Request::_handleHeader(void)
{

};

int Request::readClientRequest(void)
{
	std::string	rawRequestLine;
	char		buf[READ_BUFFER_SIZE];
	int			read_ret;
	//char		*next_nl;
	//char		*headerStart;


	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE);
	if (read_ret == -1)
		throw (std::runtime_error(strerror(errno)));
	std::cout << ">" << buf << "<" << std::endl;
	for (int i = 0; i < read_ret; i++)
		_rawRequest.push_back(buf[i]);

	if (_state == R_REQUESTLINE )
		_handleRequestLine();
	if (_state == R_HEADER)
		_handleHeader();

	return (1);
}
