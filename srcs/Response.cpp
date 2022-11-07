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

Response::Response(int clientFd, Request *request, Config *config)
{
	_clientFd = clientFd;
	_request = request;
	_config = config;
	_statusCode = _request->getStatusCode();
	std::cout << "Create response with request with target [" << request->getTarget() << "]" << std::endl;
	std::cout << "\e[31m--------------------Start of Config used for creation--------------------" << std::endl;
	std::cout << *_config;
	std::cout << "---------------------End of Config used for creation---------------------\e[0m" << std::endl;
}

Response::Response(const Response &src)
{
	*this = src;
}

Response::~Response(void)
{
}

Response &Response::operator=(const Response &rhs)
{
	_responseReady = rhs._responseReady;
	_clientFd = rhs._clientFd;
	_statusCode = rhs._statusCode;
	_lineStatus = rhs._lineStatus;
	_header = rhs._header;
	_body = rhs._body;
	_fullResponse = rhs._fullResponse;
	_bodyToSend = rhs._bodyToSend;
	_request = rhs._request;
	_config = rhs._config;

	return (*this);
}

void Response::_createErrorMessageBody(void)
{
	string errorMessage(itoa(_statusCode) + " " + _errorMessage.find(_statusCode)->second);
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
	std::cout << "\e[31m set request with target " << request->getTarget() << "\e[0m\n";
	_request = request;
	_statusCode = _request->getStatusCode();
}

void Response::_createBody(void)
{
	std::string nextLine;
	std::ifstream fs;
	char *buff;
	int length;
	std::string target (_request->getTarget());

	if (target.at(target.length() - 1) == '/')
		target = target + "index.html";
	std::cout << "getrootDir:[" << _config->getRootDir() << "]\n";
	std::string fileName(_config->getRootDir() + target);
	std::cout << "fileName: " << fileName << std::endl;
	fs.open( fileName.c_str(), std::ifstream::in | std::ifstream::binary);

	if (fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		std::cerr << "Failure opening body file '" << strerror(errno) << std::endl;
		_statusCode = 404;
		fs.close();
		return;
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
	_header = "content-length: " + itoa(_body.size()) + "\n";
	if (contentType == "jpg")
		_header += "content-type: image/" + contentType + "\n";


	std::cout << _header << std::endl;
}

void Response::_createFullResponse(void)
{
	_fullResponse = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullResponse.insert(_fullResponse.end(), _header.begin(), _header.end());
	_fullResponse.push_back('\n');
	//_fullResponse.push_back('\n');
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
	if(_statusCode > 200)
		_createErrorMessageBody();
	_createHeader();
	_lineStatus = string(_request->getProtocol() + " " + itoa(_statusCode) + " " + _errorMessage.find(_statusCode)->second + "\r\n");
	_createFullResponse();
	//std::cout << _lineStatus;
	return 0;
}

// int Response::writeClientResponse(void)
// {
// 	int		ret;
// 	// debut de gestion des chunks -> fonction qui ecrit la reponses dans un tableau de buff[WRITE_BUFF_SIZE];
// 	char buff[WRITE_BUFFER_SIZE];
// 	memset(buff, 0, WRITE_BUFFER_SIZE);
// 	int i = 0;
// 	v_c::iterator ite = _fullResponse.end();
// 	for (v_c::iterator it = _fullResponse.begin(); i < WRITE_BUFFER_SIZE && it != ite; i++, it++)
// 		buff[i] = *it;
// 	std::cout << "About to write on fd [" << _clientFd << "]" << std::endl;
// 	ret = send(_clientFd, buff, i, 0);
// 	if (ret == -1)
// 		std::cerr << "Error in writeClientResponse" << std::endl;
// 	else
// 	{
// 		/*
// 		std::cout << "-------------Raw Buffer start------------" << std::endl;
// 		write(1, buff, ret);
// 		std::cout << "-------------Raw Buffer start------------" << std::endl;
// 		*/
// 		_fullResponse.erase(_fullResponse.begin(), _fullResponse.begin() + ret);
// 		std::cout << "Sent bytes : [" << ret << "]. Remaining Content : [" << _fullResponse.size() << "]" <<std::endl;
// 	}
// 	if (_fullResponse.empty())
// 		return (0);
// 	return 1;
// }

int Response::writeClientResponse(void)
{
	int		ret;
	int 	buff_size;
	// debut de gestion des chunks -> fonction qui ecrit la reponses dans un tableau de buff[WRITE_BUFF_SIZE];
	char *buff;
	if (_fullResponse.size() > BUFF_MAX)
	{
		buff = new char[BUFF_MAX];
		buff_size = BUFF_MAX;
	}
	else
	{
		buff = new char[_fullResponse.size()];
		buff_size = _fullResponse.size();
	}
	int i = 0;
	v_c::iterator ite = _fullResponse.end();
	for (v_c::iterator it = _fullResponse.begin(); i < buff_size && it != ite; i++, it++)
		buff[i] = *it;
	std::cout << "buff_size [" << buff_size << "]" << "About to write on fd [" << _clientFd << "]" << std::endl;
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
	delete [] buff;
	if (_fullResponse.empty())
		return (0);
	return 1;
}

void Response::reset(void)
{
	std::cout << "\x1b[31m Clean Response \x1b[0m" << std::endl;
	//*this = Response(_clientFd);
}

