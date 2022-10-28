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

std::string Response::_errorBodyTemplate = "<html>\n<head><title>Error_placeholder</title></head>\n<body>\n<center><h1>Error_placeholder</h1></center>\n<hr><center>webserv/0.1</center>\n</body>\n</html>";

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

void Response::_createErrorMessageBody(void)
{
	string errorMessage( _itoa(_statusCode) + " " + _errorMessage.find(_statusCode)->second);
	_bodyToSend = _errorBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = _bodyToSend.find("Error_placeholder");
		_bodyToSend.erase(pos, strlen("Error_placeholder"));
		_bodyToSend.insert(pos, errorMessage);
	}
	std::cerr <<"1\n";
	string::iterator ite = _bodyToSend.end();
	for (string::iterator it = _bodyToSend.begin(); it != ite; it++)
		_body.push_back(*it);
	std::cout << _bodyToSend;

}

void Response::setRequest(const Request *request)
{
	_request = request;
	_statusCode = _request->getStatusCode();
}

int Response::createResponse(void)
{
	//status-line = HTTP-version SP status-code SP reason-phrase CRLF
	_statusCode = 404;
	std::cerr << "0\n";
	if (_statusCode > 200)
		_createErrorMessageBody();
	     //status-line = HTTP-version SP status-code SP reason-phrase CRLF
/*
>>>>>>> 1cbadef460ffa391b5b557a420c879ef8c7e95e9
	_lineStatus = string(_request->getProtocol() + " "
	+ _itoa(_statusCode) + " "
	+ _errorMessage.find(_statusCode)->second + "\r\n");
	std::cout << _lineStatus;
*/
	return 0;
}

int Response::writeClientResponse(void)
{
	// debut de gestion des chunks -> fonction qui ecrit la reponses dans un tableau de buff[WRITE_BUFF_SIZE];
	char 			buff[WRITE_BUFFER_SIZE];
	size_t 			i = 0;
	v_c::iterator 	ite = _body.end();

	memset(buff,0,WRITE_BUFFER_SIZE);
	std::cout <<"2\n";
	for (; i < WRITE_BUFFER_SIZE && i < _lineStatus.length(); i++)
		buff[i] = _lineStatus[i];
	string length("content-length: " + _itoa(_bodyToSend.length()) + "\n");
	std::cerr << length << std::endl;
	for (size_t y = 0; i < WRITE_BUFFER_SIZE && y < length.length(); i++)
		buff[i] = length[y++];
	buff[i] = '\n';
	buff[i++] = '\n';
	for (v_c::iterator it = _body.begin(); i < WRITE_BUFFER_SIZE && it != ite; i++)
		buff[i] = *it++;
	
	buff[i] = 0;
	std::cout <<"3\n";
	std::cout << buff;
	std::cout << "4\n";
	write(_clientFd, buff, strlen(buff));
	_lineStatus.clear();
	_bodyToSend.clear();
	_body.clear();
	return 0;
}
std::string _itoa(int statusCode)
{
	std::stringstream out;
	out << statusCode;
	return out.str();
}
