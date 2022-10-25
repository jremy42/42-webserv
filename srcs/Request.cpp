# include "Request.hpp"

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

int	Request::parseError(string rawRequestLine)
{
	std::istringstream	buf(rawRequestLine);
	string				bufExtract;
	string				headerList[3] = {"method", "request_uri", "http_version"};
	int					i = 0;

	while (std::getline(buf, bufExtract, ' ') && i < 3)
	{
		_requestLine.insert(std::pair<string, string>(headerList[i], bufExtract));
		std::cout << "[" << headerList[i] << "]";
		std::cout << "[" << _requestLine.find(headerList[i])->second << "]" << std::endl;
		i++;
	}
	return (0);
}

int Request::readClientRequest(void)
{
	std::string	rawRequestLine;
	char		buf[READ_BUFFER_SIZE];
	int			read_ret;
	char		*next_nl;


	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE);
	if (read_ret == -1)
		throw (std::runtime_error(strerror(errno)));
	std::cout << ">" << buf << "<" << std::endl;
	if (_state == R_REQUESTLINE && (next_nl = strchr(buf, '\n')))
	{
		*next_nl = '\0';
		rawRequestLine = buf;
		*next_nl = '\n';
		//_rawRequest(buf);
		_state = R_HEADER;
		if(this->parseError(rawRequestLine))
			_state = R_ERROR;
	}

	return (1);
}
