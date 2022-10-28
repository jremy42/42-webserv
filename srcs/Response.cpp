#include "Response.hpp"

Response::m_is Response::_errorMessage = _initErrorMessage();

Response::m_is Response::_initErrorMessage()
{
	m_is ret;
	ret[400] = "Bad Request";
	ret[200] = "OK";
	ret[404] = "Not Found";
	ret[405] = "Method Not Allowed";

	return ret;
}

Response::Response(){};

Response::Response(int clientFd)
{
	_clientFd = clientFd;

}

Response::Response(const Response &src) : _request(src._request)
{
	*this = src;
}

Response::~Response(void)
{

}

Response	&Response::operator=(const Response &rhs)
{
	_clientFd = rhs._clientFd;
	_responseReady = rhs._responseReady;
	_statusCode = rhs._statusCode;
	_header = rhs._header;
	_body = rhs._body;

	return (*this);
}

void Response::setRequest(const Request *request)
{
	_request = request;
	_statusCode = _request->getStatusCode();
}

int Response::createResponse(void)
{
	//status-line = HTTP-version SP status-code SP reason-phrase CRLF
	_lineStatus = string(_request->getProtocol() + " "
	+ _itoa(_statusCode) + " "
	+ _errorMessage.find(_statusCode)->second + "\r\n");
	std::cout << _lineStatus;
	return 0;
}

int Response::writeClientResponse(void)
{
	write(_clientFd, _lineStatus.c_str(), _lineStatus.length());
	return 1;
}
std::string _itoa(int statusCode)
{
	std::stringstream out;
	out << statusCode;
	return out.str();
}
