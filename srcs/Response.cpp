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

Response &Response::operator=(const Response &rhs)
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
	string errorMessage(_itoa(_statusCode) + " " + _errorMessage.find(_statusCode)->second);
	_bodyToSend = _errorBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = _bodyToSend.find("Error_placeholder");
		_bodyToSend.erase(pos, strlen("Error_placeholder"));
		_bodyToSend.insert(pos, errorMessage);
	}
	_body = v_c(_bodyToSend.begin(), _bodyToSend.end());
	std::cout << _bodyToSend;
}

void Response::setRequest(const Request *request)
{
	_request = request;
	_statusCode = _request->getStatusCode();
}

void Response::_createBody(void)
{
	std::string nextLine;
	std::ifstream fs;
	char *buff;
	int length;
	std::string fileName("./www" + _request->getTarget() + "" );
	std::cout << "fileName: " << fileName << std::endl;
	fs.open( fileName.c_str(), std::ifstream::in | std::ifstream::binary);

	if (fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		std::cerr << "Failure opening body file '" << strerror(errno) << std::endl;
		fs.close();
	}
	fs.seekg(0, fs.end);
	length = fs.tellg();
	fs.seekg(0, fs.beg);
	std::cout << "length: [" << length << "]\n";
	buff = new char[length];
	fs.read(buff, length);
	_body = v_c(buff, buff + length);
	delete buff;
	fs.close();
}


void Response::_createHeader(void)
{
	string contentType(_request->getTarget());
	size_t pos = contentType.find_last_of(".");
	if (pos != std::string::npos)
		contentType = string(contentType.begin() + pos + 1, contentType.end());
	_header = "content-length: " + _itoa(_body.size()) + "\n";
	if (contentType == "jpg")
		_header += "content-type: image/" + contentType + "\n";


	std::cout << _header << std::endl;
}

void Response::_createFullResponse(void)
{
	_fullResponse = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullResponse.insert(_fullResponse.end(), _header.begin(), _header.end());
	_fullResponse.push_back('\n');
	_fullResponse.push_back('\n');
	_fullResponse.insert(_fullResponse.end(), _body.begin(), _body.end());
	//std::cout << _fullResponse << std::endl;
}

int Response::createResponse(void)
{
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	//_statusCode = 404;
	//std::cerr << "0\n";

	if (_statusCode == 200)
		_createBody();
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	else
		_createErrorMessageBody();
	_createHeader();
	_lineStatus = string(_request->getProtocol() + " " + _itoa(_statusCode) + " " + _errorMessage.find(_statusCode)->second + "\r\n");
	_createFullResponse();
	//std::cout << _lineStatus;
	return 0;
}

int Response::writeClientResponse(void)
{
	int		ret;
	// debut de gestion des chunks -> fonction qui ecrit la reponses dans un tableau de buff[WRITE_BUFF_SIZE];
	char buff[WRITE_BUFFER_SIZE];
	memset(buff, 0, WRITE_BUFFER_SIZE);
	int i = 0;
	v_c::iterator ite = _fullResponse.end();
	for (v_c::iterator it = _fullResponse.begin(); i < WRITE_BUFFER_SIZE && it != ite; i++, it++)
		buff[i] = *it;
	std::cout << "About to write on fd [" << _clientFd << "]" << std::endl;
	ret = send(_clientFd, buff, i, 0);
	if (ret == -1)
		std::cerr << "Error in writeClientResponse" << std::endl;
	else
	{
		/*
		std::cout << "-------------Raw Buffer start------------" << std::endl;
		write(1, buff, ret);
		std::cout << "-------------Raw Buffer start------------" << std::endl;
		*/
		_fullResponse.erase(_fullResponse.begin(), _fullResponse.begin() + ret);
		std::cout << "Sent bytes : [" << ret << "]. Remaining Content : [" << _fullResponse.size() << "]" <<std::endl;
	}
	if (_fullResponse.empty())
		return (0);
	return 1;
}

void Response::reset(void)
{
	*this = Response(_clientFd);
}

std::string _itoa(int statusCode)
{
	std::stringstream out;
	out << statusCode;
	return out.str();
}

