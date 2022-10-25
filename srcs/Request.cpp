# include "Request.hpp"

Request::Request(int clientFd)
{
	_clientFd = clientFd;
	_header.push
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
	_clientFd = rhs._clientFd;
	_header = rhs._header;
	_body = rhs._body;
	return (*this);
}

int Request::readClientRequest(void)
{

}