# include "Request.hpp"

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

	std::cout << "Parse Request Line" << std::endl;
	while (std::getline(buf, bufExtract, ' ') && i < 3)
	{
		_requestLine.insert(std::pair<string, string>(_requestLineField[i], bufExtract));
		std::cout << "[" << _requestLineField[i] << "]";
		std::cout << "[" << _requestLine.find(_requestLineField[i])->second << "]" << std::endl;
		i++;
	}
	return (0);
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
		std::cout << "BEFORE" "[" << header_key << "][" << header_value << "]" << std::endl;
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		std::cout << "AFTER" << "[" << header_key << "][" << header_value << "]" << std::endl;
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
	return (0);
}


void Request::_handleRequestLine(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

	std::cout << "Handle Request Line" << std::endl;
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
			return;
		}
	}
}

void Request::_handleHeader(void)
{
	v_c_it ite = _rawRequest.end();
	v_c_it it = _rawRequest.begin();

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


	memset(buf, 0, sizeof(buf));
	read_ret = read(_clientFd, buf, READ_BUFFER_SIZE);
	if (read_ret == -1)
		throw (std::runtime_error(strerror(errno)));
	std::cout << "READ BUFFER START" << std::endl << buf << std::endl << "READ BUFFER END" << std::endl;
	for (int i = 0; i < read_ret; i++)
		_rawRequest.push_back(buf[i]);

	if (_state == R_REQUESTLINE )
		_handleRequestLine();
	if (_state == R_HEADER)
		_handleHeader();

	return (1);
}

std::string	&strtrim(std::string &str, const std::string &charset)
{
	std::size_t first 					= str.find_first_not_of(charset);
	std::size_t last 					= str.find_last_not_of(charset);
	std::string::iterator begin			= str.begin();
	std::string::iterator end			= str.end();

	if (first != std::string::npos)
		begin += first;
	if (last != std::string::npos)
		end -= str.length() - last - 1;
	str = std::string(begin, end);
	return (str);
}
