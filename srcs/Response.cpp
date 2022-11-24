#include "Response.hpp"

Response::m_is Response::_statusCodeMessage = _initStatusCodeMessage();

Response::m_is Response::_initStatusCodeMessage()
{
	m_is ret;
	ret[200] = "OK";
	ret[301] = "Moved Permanently";
	ret[400] = "Bad Request";
	ret[404] = "Not Found";
	ret[405] = "Method Not Allowed";
	ret[408] = "Request Timeout";

	return ret;
}

std::string Response::_errorBodyTemplate = "<html>\n<head><title>Error_placeholder</title></head>\n<body>\n<center><h1>Error_placeholder</h1></center>\n<hr><center>webserv/0.1</center>\n</body>\n</html>";
std::string Response::_autoIndexBodyTemplate = "<html><head><title>Index of /title_placeholder</title></head>\n<body>\n<h1>Index of /title_placeholder</h1><hr><pre>\n</pre><hr>\n</body></html>";

Response::Response(){};

Response::Response(int clientFd, Request *request, const Config *config, int statusCode)
{
	_clientFd = clientFd;
	_request = request;
	_config = config;
	_statusCode = statusCode;
	_state = R_INIT;
	std::cout << "Create response with request with target [" << request->getTarget() << "]" << std::endl;
	std::cout << "\e[31m--------------------Start of Config used for creation--------------------" << std::endl;
	std::cout << *_config;
	std::cout << "---------------------End of Config used for creation---------------------\e[0m" << std::endl;
	memset(_nameOut, 0, 32);
	memset(_nameIn, 0, 32);
	strncpy(_nameIn, "/tmp/webservXXXXXX", 32);
	strncpy(_nameOut, "/tmp/webservXXXXXX", 32);

}

Response::Response(const Response &src)
{
	*this = src;
}

Response::~Response(void)
{
	if (strcmp(_nameIn, "/tmp/webservXXXXXX"))
		unlink(_nameIn);
	if (strcmp(_nameOut, "/tmp/webservXXXXXX"))
		unlink(_nameOut);
}

Response &Response::operator=(const Response &rhs)
{
	_responseReady = rhs._responseReady;
	_clientFd = rhs._clientFd;
	_statusCode = rhs._statusCode;
	_lineStatus = rhs._lineStatus;
	_header = rhs._header;
	_body = rhs._body;
	_fullHeader = rhs._fullHeader;
	_defaultErrorBodyToSend = rhs._defaultErrorBodyToSend;
	_request = rhs._request;
	_config = rhs._config;
	_state = rhs._state;
 //	_fs = rhs._fs;
 //	_ss = rhs._ss;
 //	_nameIn = rhs._nameIn;
 //	_nameOut = rhs._nameOut;
	_bodyLength = rhs._bodyLength;

	return (*this);
}

void Response::_createErrorMessageBody(void)
{
	std::string	actualTarget;
	std::string	selectActualTargetResult;	
	string errorMessage(itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second);
	string requestTarget = _request->getTarget();
	string	errorPage = _config->getErrorPageByLocation(requestTarget, _statusCode);
	string	matchingLocation = _config->getMatchingLocation(requestTarget);

	std::cout << ">>>>>>>errorPage found [" << errorPage << "]" << std::endl;
	
	if (errorPage != "")
	{
	
		selectActualTargetResult = _selectActualTarget(actualTarget, matchingLocation + errorPage);
		std::cout << ">>>>> selectActualTargetResult: [" << selectActualTargetResult << "]" << std::endl; 
		if (selectActualTargetResult != "Index_file_nok" && selectActualTargetResult != "File_nok")
		{
			_createFileStreamFromFile(actualTarget);
			_state = R_FILE_READY;
			return ;
		}
		else
			errorPage = "";
	}
	if (errorPage == "")
	{
		_defaultErrorBodyToSend = _errorBodyTemplate;
		for (int i = 0; i < 2; i++)
		{
			size_t pos = _defaultErrorBodyToSend.find("Error_placeholder");
			_defaultErrorBodyToSend.erase(pos, strlen("Error_placeholder"));
			_defaultErrorBodyToSend.insert(pos, errorMessage);
		}
		//_body = v_c(_defaultErrorBodyToSend.begin(), _defaultErrorBodyToSend.end());
		_ss << _defaultErrorBodyToSend;
		_bodyLength = _defaultErrorBodyToSend.size();
		_header += "content-length: " + itoa(_bodyLength) + "\n";
	}
	if (DEBUG_RESPONSE)
		std::cout << _defaultErrorBodyToSend;
}

void Response::setRequest(const Request *request)
{
	std::cout << "\e[31m set request with target " << request->getTarget() << "\e[0m\n";
	_request = request;
	_statusCode = _request->getStatusCode();
}

std::string	Response::_selectActualTarget(string &actualTarget, string requestedTarget)
{
	//std::string		requestedTarget	= _request->getTarget();
	std::string		requestedTargetLocation	= _config->getParamByLocation(requestedTarget, "root").at(0);

	actualTarget = requestedTargetLocation + requestedTarget;	
	if (DEBUG_RESPONSE)
	{
		std::cout << "requested Target : [" << requestedTarget << "]" << std::endl;
		std::cout << "requested Target Location (according to root directives) : [" << requestedTargetLocation << "]" << std::endl;
		std::cout << "actual Target : [" << actualTarget << "]" << std::endl;
		if (!fileExist(actualTarget))
			std::cout << "actual Target does not exists" << std::endl;
		if (fileExist(actualTarget) && isDir(actualTarget))
			std::cout << "actual Target exist and is a Directory" << std::endl;
		else if (fileExist(actualTarget) && !isDir(actualTarget))
			std::cout << "actual Target exist and is a regular file" << std::endl;
	}
	if (fileExist(actualTarget) && !isDir(actualTarget))
	{
		if (DEBUG_RESPONSE)
			std::cout << "Requested file is standard" << std::endl;
		return ("File_ok");
	}
	else if (fileExist(actualTarget) && isDir(actualTarget))
	{
		std::vector<string> indexTryFiles = _config->getParamByLocation(requestedTarget, "index");
		if (DEBUG_RESPONSE)
			std::cout << "Trying files in indexTryFiles :" << indexTryFiles << std::endl;
		std::vector<string>::iterator it = indexTryFiles.begin();
		for (;it != indexTryFiles.end(); it++)
		{
			std::string	testedIndexFile = actualTarget + "/" + *it;
			if (DEBUG_RESPONSE)
				std::cout << "Testing index file :" << testedIndexFile << std::endl;
			if (fileExist(testedIndexFile) && !isDir(testedIndexFile))
			{
				actualTarget = testedIndexFile;
				break;
			}
		}
		if (it != indexTryFiles.end())
		{
			if (DEBUG_RESPONSE)
				std::cout << "Found a suitable index file : [" << *it << "]" << std::endl;
			return ("Index_file_ok");
		}
		else if (_config->getParamByLocation(requestedTarget, "autoindex").at(0) == "on")
		{
			if (DEBUG_RESPONSE)
				std::cout << "No suitable index file but autoindex is on. Returning Listing of directory" << std::endl;
			return ("Do_listing");
		}
		else
	{
			if (DEBUG_RESPONSE)
				std::cout << "No suitable index file and autoindex is off" << std::endl;
			return ("Index_file_nok");
		}
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cout << "No such file or directory" << std::endl;
		return ("File_nok");
	}
}

void Response::_createFileStreamFromFile(string actualTarget) // set le header avec taille qui va bien et open le Body
{
	std::cout << "createFsfrom file : open " << actualTarget << std::endl;
	_fs.open(actualTarget.c_str(), std::ifstream::in | std::ifstream::binary);
	if (_fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
	{
		std::cerr << "Failure opening body file '" << strerror(errno) << std::endl;
		_statusCode = 404;
		_fs.close();
		return;
	}
	_fs.seekg(0, _fs.end);
	_bodyLength = _fs.tellg();
	_fs.seekg(0, _fs.beg);
	if (DEBUG_RESPONSE)
		std::cout << "Body length: [" << _bodyLength << "]\n";
 //	buff = new char[length];
 //	_fs.read(buff, length);
 //	_body = v_c(buff, buff + length);
 //	delete buff;
 //	_fs.close();
}
 void Response::_createBodyFromFile(const string &actualTarget) // set le header avec taille qui va bien et open le Body
 {
 	std::ifstream	fs;
 	char			*buff;
 	int				length;
 
 	std::cout << "create Body From file : open " << actualTarget << std::endl;
 	fs.open(actualTarget.c_str(), std::ifstream::in | std::ifstream::binary);
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
 	if (DEBUG_RESPONSE)
 		std::cout << "Body length: [" << length << "]\n";
 	buff = new char[length];
 	fs.read(buff, length);
 	_body = v_c(buff, buff + length);
 	delete buff;
 	fs.close();
 }

std::string	Response::_getExtensionFromTarget(string actualTarget)
{
	std::size_t	posDot;
	std::size_t	posEnd;

	posDot = actualTarget.find_last_of(".");
	std::string endPart = actualTarget.substr(posDot);
	posEnd = endPart.find_first_of("/?");
	posEnd = posEnd != std::string::npos ? posEnd : endPart.size();
	std::string extension = endPart.substr(0, posEnd + 1);
	if (DEBUG_RESPONSE)
		std::cout << "Extension : [" << extension << "]" << std::endl;
	return (extension);
}

void Response::_methodGET(void)
{
	std::string	actualTarget;
	std::string	selectActualTargetResult;
	std::string	cgiExecutable;
	
	selectActualTargetResult = _selectActualTarget(actualTarget, _request->getTarget());
	if (DEBUG_RESPONSE)
		std::cout << "Actual target : [" << actualTarget << "]" << std::endl;
	if (selectActualTargetResult == "Do_listing")
		_createAutoIndex(actualTarget);
	else if (selectActualTargetResult != "Index_file_nok" && selectActualTargetResult != "File_nok")
	{
		std::string rawTarget = _request->getTarget();
		if ((cgiExecutable = _config->getCgiByLocation(rawTarget, _getExtensionFromTarget(actualTarget))) != "")
		{
			if (DEBUG_RESPONSE)
				std::cout << "\e[33mCGI\e[0m" << std::endl;
			if (_state == R_INIT)
				_initCGIfile(actualTarget, cgiExecutable);
			if (_state == R_WAIT_CGI_EXEC)
				_waitCGIfile();
		}
		else
		{
			_createFileStreamFromFile(actualTarget);
			_state = R_FILE_READY;
		}
	}
	else
	{
		_statusCode = 404;
		_createErrorMessageBody();
	}
}

void Response::_methodPOST(void)
{
	std::string	actualTarget;
	std::string	selectActualTargetResult;
	
	selectActualTargetResult = _selectActualTarget(actualTarget, _request->getTarget());
	

	//access sur le fichier droit d'ecriture 
	//if file existe append ?
	// else create file
	// create response
}

/* void Response::_parentPartCgi(int pipefdParentToChild[2], int pipefdChildToParent[2], pid_t pid)
{
	int		status;
	int		ret;
	int		readRet;
	char	readBuf[512];
	v_c		requestBody = _request->getBody();

	if (close(pipefdParentToChild[0]))
			throw(std::runtime_error("Close error" ));
		if (requestBody.size() != 0)
		{
			std::cerr << "Parent sending body : [" << requestBody.size() << "]" << std::endl;
			char *buff = new char[requestBody.size()];
			int i = 0;
			v_c::iterator ite = requestBody.end();
			for (v_c::iterator it = requestBody.begin(); it != ite; i++, it++)
				buff[i] = *it;
			write(pipefdParentToChild[1], buff, i);
		}
		if (close(pipefdParentToChild[1]))
			throw(std::runtime_error("Close error" ));
		if (close(pipefdChildToParent[1]))
			throw(std::runtime_error("Close error" ));
		memset(readBuf, 0, 512);
		while ((readRet = read(pipefdChildToParent[0], readBuf, 511)) > 0)
		{
			//std::cerr << "\e[34mreadBuf [" << readBuf << "]\e[0m" << std::endl;
			_body.insert(_body.end(), readBuf, readBuf + readRet);
		}
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) > 0)
			ret = (WEXITSTATUS(status));
		if (WIFSIGNALED(status) > 0)
			ret = (WTERMSIG(status) + 128);
		std::cerr << "ret : [" << ret << "]" << std::endl;
		if (close(pipefdChildToParent[0]))
			throw(std::runtime_error("Close error" ));
		_extractHeaderFromCgiBody();

}

void Response::_handleCGI(string actualTarget, string cgiExecutable)
{
	pid_t	pid;
	int		pipefdParentToChild[2];
	int		pipefdChildToParent[2];



	if (pipe(pipefdParentToChild))
		throw(std::runtime_error("Pipe error" ));
	if (pipe(pipefdChildToParent))
		throw(std::runtime_error("Pipe error" ));
	if ((pid = fork()) == -1)
		throw(std::runtime_error("Fork error" ));
	if (pid != 0)
	{
		_parentPartCgi(pipefdParentToChild, pipefdChildToParent, pid);
	}
	else
	{
		if (close(pipefdParentToChild[1]))
			throw(std::runtime_error("Close error" ));
		if (close(pipefdChildToParent[0]))
			throw(std::runtime_error("Close error" ));
		if (dup2(pipefdParentToChild[0], STDIN_FILENO) == -1)
			throw(std::runtime_error(std::string("Child DUP2 error 0") + strerror(errno)));
		if (close(pipefdParentToChild[0]))
			throw(std::runtime_error(std::string("Child Close error 0") + strerror(errno)));
		if (dup2(pipefdChildToParent[1], STDOUT_FILENO) == -1)
			throw(std::runtime_error(std::string("Child DUP2 error 1") + strerror(errno)));
		if (close(pipefdChildToParent[1]))
			throw(std::runtime_error(std::string("Child Close error 1") + strerror(errno)));
		char *arg[3];
		arg[0] = const_cast<char *>(cgiExecutable.c_str());
		arg[1] = const_cast<char *>(actualTarget.c_str());
		arg[2] = NULL;
		std::cerr << "actual Target : [" << actualTarget << "] CGI-executable : [" << cgiExecutable << "]" << std::endl;
		execve(cgiExecutable.c_str(), arg, NULL);
		throw(std::runtime_error(std::string("Execve error") + strerror(errno)));
	}
}
 */

void Response::_waitCGIfile(void)
{
	int		status;
	int		ret;
 //	int		readRet;
 //	char	readBuf[512];

	if (waitpid(_pid, &status, WNOHANG) == 0) // 0 car pas finit
		return ;
	else
	{
		std::cout << "A child is DEAD at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		if (WIFEXITED(status) > 0)
			ret = (WEXITSTATUS(status));
		if (WIFSIGNALED(status) > 0)
			ret = (WTERMSIG(status) + 128);
		std::cerr << "ret : [" << ret << "]" << std::endl;
 //		memset(readBuf, 0, 512);
 //		lseek(_outChild, 0, SEEK_SET);
 //		while ((readRet = read(_outChild, readBuf, 511)) > 0)
 //		{
 //			std::cout << "readRet:" << readRet << std::endl;
 //			_body.insert(_body.end(), readBuf, readBuf + readRet);
 //		}
		//_extractHeaderFromCgiBody();
		if (close(_inChild))
			throw(std::runtime_error("Close error inChild" ));
		if (unlink(_nameIn))
			throw(std::runtime_error("unlink error" ));
		if (close(_outChild))
			throw(std::runtime_error("close error outChild" ));
		/*if (unlink(_nameOut))
			throw(std::runtime_error("unlink error" ));*/
		_createFileStreamFromFile(_nameOut);
		std::string extractedHeader; 
		getline(_fs, extractedHeader, '\n');
		while (extractedHeader != "\r")
		{
			_bodyLength -= (extractedHeader.size() + 1);
			_header += extractedHeader + "\n";
			getline(_fs, extractedHeader, '\n');
		}
		_bodyLength -= 2;
		std::cout << "real CGI body length after removing header : [" << _bodyLength << "]" << std::endl;
		_state = R_FILE_READY;
	}
}

void Response::_initCGIfile(string actualTarget, string cgiExecutable)
{
	if ((_inChild = mkstemp(_nameIn)) == -1)
		throw(std::runtime_error(std::string("_nameIn mkstemp error") + strerror(errno)));
	if ((_outChild = mkstemp(_nameOut)) == -1)
		throw(std::runtime_error(std::string("_nameOut mkstemp error") + strerror(errno)));
	v_c		requestBody = _request->getBody();


	if (DEBUG_RESPONSE)
	{
		std::cout << "_nameIN : [" << _nameIn << "]" << std::endl;
		std::cout << "_nameOut : [" << _nameOut << "]" << std::endl;
		std::cout << "nameIn: [" << _nameIn << "]" << std::endl;
		std::cout << "nameOut: [" << _nameOut << "]" << std::endl;
		std::cout << "inchild fd: [" << _inChild << "]" << std::endl;
		std::cout << "outchild fd: [" << _outChild << "]" << std::endl;
		std::cout << "open " << actualTarget << std::endl;
	}
	if (requestBody.size() != 0)
	{
		char *buff = new char[requestBody.size()];
		int i = 0;
		v_c::iterator ite = requestBody.end();
		for (v_c::iterator it = requestBody.begin(); it != ite; i++, it++)
			buff[i] = *it;
		write(_inChild, buff, i);
	}
	if ((_pid = fork()) == -1)
		throw(std::runtime_error("Fork error" ));
	if (_pid != 0)
	{
		std::cout << "A child is born at [\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		_state = R_WAIT_CGI_EXEC;
		return ;
	}
	else
	{
		if (dup2(_inChild, STDIN_FILENO) == -1)
			throw(std::runtime_error(std::string("Child DUP2 error 0") + strerror(errno)));
		if (dup2(_outChild, STDOUT_FILENO) == -1)
			throw(std::runtime_error(std::string("Child DUP2 error 1") + strerror(errno)));
		char *arg[3];
		arg[0] = const_cast<char *>(cgiExecutable.c_str());
		arg[1] = const_cast<char *>(actualTarget.c_str());
		arg[2] = NULL;
		std::cerr << "actual Target : [" << actualTarget << "] CGI-executable : [" << cgiExecutable << "]" << std::endl;
		execve(cgiExecutable.c_str(), arg, NULL);
		throw(std::runtime_error(std::string("Execve error") + strerror(errno)));
	}
}

 //void	Response::_extractHeaderFromCgiBody()
 //{
 //	v_c::iterator	it = _body.begin();
 //	v_c::iterator	ite = _body.end();
 //
 //	for (; it != ite; it++)
 //	{
 //		if (*it == '\r'
 //			&& it + 1 != ite && *(it + 1) == '\n'
 //			&& it + 2 != ite && *(it + 2) == '\r'
 //			&& it + 3 != ite && *(it + 3) == '\n')
 //		{
 //			string rawHeader(_body.begin(), it);
 //			_body.erase(_body.begin(), it + 4);
 //			_header += rawHeader + "\n";
 //		}
 //	}
 //}

void Response::_createHeaderBase(void)
{
	string contentType(_request->getTarget());
	size_t pos = contentType.find_last_of(".");
	if (pos != std::string::npos)
		contentType = string(contentType.begin() + pos + 1, contentType.end());
	_header += "content-length: " + itoa(_body.size()) + "\n";
	if (contentType == "jpg")
		_header += "content-type: image/" + contentType + "\n";
	std::cout << _header << std::endl;
}

void Response::_createFullHeader(void)
{
	_header += "content-length: " + itoa(_bodyLength) + "\n";
	_fullHeader = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullHeader.insert(_fullHeader.end(), _header.begin(), _header.end());
	_fullHeader.push_back('\n');
	std::cout << "Full Header size : [" << _fullHeader.size() << "]" << std::endl;
	//_fullHeader.insert(_fullHeader.end(), _body.begin(), _body.end());
}

void Response::_checkAutorizationForMethod(void)
{
	string requestTarget = _request->getTarget();
	std::vector<string> allowedMethod = _config->getParamByLocation(requestTarget, "allowed_method");

	if (find(allowedMethod.begin(), allowedMethod.end(), _request->getMethod()) == allowedMethod.end())
		_statusCode = 405;
}

void Response::_checkRedirect(void)
{
	string requestTarget = _request->getTarget();
	if( _config->getParamByLocation(requestTarget, "return")[0] == "1")
		return;
	else
	{
		std::cout << "REDIRECT\n";
		_statusCode = atoi(_config->getParamByLocation(requestTarget, "return")[0].c_str());
		_header += "location: " + _config->getParamByLocation(requestTarget, "return")[1] + "\n";
	}
}

int Response::handleResponse(void)
{
	std::cout << "handleResponse IN\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	if (DEBUG_RESPONSE)
		std::cout << "Handle response start. Status [" << _state << "]" << std::endl;
	if (_state != R_WRITE)
	{
		if (DEBUG_RESPONSE)
			std::cout << "Calling _createResponse" << std::endl;
		_createResponse();
	}
	if (_state == R_WRITE)
	{
		std::cout << "Calling _writeClientResponse" << std::endl;
		_writeClientResponse();
	}
	if (DEBUG_RESPONSE)
		std::cout << "Handle response end. Status [" << _state << "]" << std::endl;
	std::cout << "handleResponse OUT\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	if (_state == R_OVER)
		return (0);
	else
		return (1);
}

int Response::_createResponse(void)
{
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	//check methode
	std::cout << "createResponse IN\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;

	if (_state == R_INIT)
	{
		_checkRedirect();
		_checkAutorizationForMethod();
	}
	if(_statusCode > 200 && _state == R_INIT)
	{
		_createErrorMessageBody();
		_state = R_FILE_READY;
	}
	if (_state < R_FILE_READY && _request->getMethod() == "GET")
	{
		if (DEBUG_RESPONSE)
			std::cout << "GET METHOD" << std::endl;
		_methodGET(); // va set le status en R_FILE_READY a la fin du CGI ou directement si regular file
	}
	else if (_state < R_FILE_READY && _request->getMethod() == "POST")
	{

	}
	if (_state == R_FILE_READY)
	{
		std::cout << "Creating line status" << std::endl;
		_lineStatus = string(_request->getProtocol() + " " + itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second + "\r\n");
		//_createHeaderBase();
		std::cout << "Creating Full Header" << std::endl;
		_createFullHeader();
		_state = R_WRITE;
	}
	std::cout << "createResponse OUT\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return (_state);
}

int Response::_writeClientResponse(void)
{
	std::cout << "write	Response IN\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	int		ret;
	int		buff_size;
	// debut de gestion des chunks -> fonction qui ecrit la reponses dans un tableau de buff[WRITE_BUFF_SIZE];
	char *buff;
	int i = 0;
	std::istream	*bodyStreamPtr;

	if (_fs.is_open())
	{
		bodyStreamPtr = &_fs;
		std::cout << "Body is a regular fs" << std::endl;
	}
	else
	{
		bodyStreamPtr = &_ss;
		std::cout << "Body is an error string stream" << std::endl;
	}
	std::istream	&bodyStream = *bodyStreamPtr;
	std::cout << "Begin of Write Client response function" << std::endl;
	if (_fullHeader.size())
	{
		std::cout << "Header not empty -> sending it first" << std::endl;
		buff = new char[_fullHeader.size()];
		buff_size = _fullHeader.size();
		v_c::iterator ite = _fullHeader.end();
		for (v_c::iterator it = _fullHeader.begin(); i < buff_size && it != ite; i++, it++)
			buff[i] = *it;
		std::cout << "buff_size [" << buff_size << "]" << "About to write client response on fd [" << _clientFd << "]" << std::endl;
		ret = send(_clientFd, buff, i, 0);
		if (ret == -1)
			std::cerr << "Error in writeClientResponse" << std::endl;
		else
		{
			_fullHeader.erase(_fullHeader.begin(), _fullHeader.begin() + ret);
			std::cout << "Sent bytes : [" << ret << "]. Remaining Content : [" << _fullHeader.size() << "]" <<std::endl;
		}
		delete [] buff;
	}
	else if (_fullHeader.empty())
	{
		std::cout << "Header IS empty -> sending Body" << std::endl;
		char	*bufBody;
		if (_bodyLength < WRITE_BUFFER_SIZE)
			buff_size = _bodyLength;
		else
			buff_size = WRITE_BUFFER_SIZE;
		bufBody = new char[buff_size];
		bodyStream.read(bufBody, buff_size);
		std::cout << "read [" << bodyStream.gcount() << "] from body file" << std::endl;
		std::cout << "Sending chunk of body to client" << std::endl;
		ret = send(_clientFd, bufBody, bodyStream.gcount(), 0);
		_bodyLength -= ret;
		if (ret == -1)
			std::cerr << "Error in writeClientResponse in Body state" << std::endl;
		if (ret != bodyStream.gcount())
		{
			std::cout << "\e[32mLazy client : only [" << ret << "] out of [" << bodyStream.gcount() << "]\e[0m" << std::endl;
		}
		if (_bodyLength == 0)
		{
			std::cout << "No more body data to read on body fd. Closing fd" << std::endl;
			try {

				std::ifstream	&fsForClose = dynamic_cast<std::ifstream &>(bodyStream); 
				std::cout << "Closing fs" << std::endl;
				fsForClose.close();
				}
			catch (std::exception &e)
			{
				(void)e;
				std::cout << "No need to close filestream : Body is default error body" << std::endl;
			}
			_state = R_OVER;
		}
		else
			bodyStream.seekg(-(bodyStream.gcount() - ret), bodyStream.cur);
		delete [] bufBody;
	}
	std::cout << "End of Write Client response function" << std::endl;
	std::cout << "writeResponse OUT\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return 1;
}

void Response::reset(void)
{
	std::cout << "\x1b[31m Clean Response \x1b[0m" << std::endl;
}

std::map<std::string, unsigned int>	Response::_populateDirectoryMap(const char *path)
{
	struct dirent	*currentDir;
	DIR				*dp;
	std::map<string, unsigned int> dirMap;

	dp = opendir(path);
	if (!dp)
		return (dirMap);
	currentDir = readdir(dp);
	while (currentDir)
	{
		string nextDirEntry = currentDir->d_name;
		if (nextDirEntry[0] != '.' || nextDirEntry == "..")
		{
			if (currentDir->d_type == DT_DIR)
				nextDirEntry += "/";
			dirMap.insert(std::pair<string, unsigned int> (nextDirEntry, currentDir->d_type));
		}
		currentDir = readdir(dp);
	}
	closedir(dp);
	return (dirMap);
}

std::string	Response::_generateHTMLBodyWithPath(void)
{
	string HTMLbody = _autoIndexBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = HTMLbody.find("/title_placeholder");
		HTMLbody.erase(pos, strlen("/title_placeholder"));
		HTMLbody.insert(pos, _request->getTarget());
	}
	return (HTMLbody);
}

int Response::_createAutoIndex(const string &pathToDir)
{
	const char 						*path = pathToDir.c_str();
	std::map<string, unsigned int>	dirMap = _populateDirectoryMap(path);
	string							HTMLbody = _generateHTMLBodyWithPath();
	string							cleanTargetDir = _request->getTarget();
	string cleanPathToDir = pathToDir + (pathToDir[pathToDir.size() - 1] != '/' ? "/" : "");


	cleanTargetDir += cleanTargetDir[cleanTargetDir.size() - 1] != '/' ? "/" : "";
	for (std::map<string, unsigned int>::reverse_iterator it = dirMap.rbegin(); it != dirMap.rend(); it++)
	{
		if (it->second != DT_DIR)
		{
			std::stringstream out;
			// A clean, mais fonctionnel (A mettre dans une fonction a minima)
			//std::cout << "name:[" << it->first << "] type" << itoa(it->second) << "size:[" << getFileSize(it->first) << std::endl;
			out << std::left << std::setw(80 + string("<a href=\"" + it->first + "\">" + "\">").size())
				<< "<a href=\"" + cleanTargetDir + it->first + "\">" + it->first + "</a>"
				<< std::setw(40) <<  getFileSize(cleanPathToDir + it->first) + " bytes" << std::endl;
			// A clean, mais fonctionnel (A mettre dans une fonction a minima)
			size_t pos = HTMLbody.find("<pre>\n");
			pos += string("<pre>\n").size();
			HTMLbody.insert(pos, out.str());
		}
	}
	for (std::map<string, unsigned int>::reverse_iterator it = dirMap.rbegin(); it != dirMap.rend(); it++)
	{	
		if (it->second == DT_DIR)
		{	
			std::cout << "name:[" << it->first << "] type" << itoa(it->second) << "size:[" << std::endl;
			size_t pos = HTMLbody.find("<pre>\n");
			pos += string("<pre>\n").size();
			HTMLbody.insert(pos, "<a href=\"" + it->first + "\">" + it->first + "</a>\n");
		}
	}
	if (DEBUG_RESPONSE)
	{
		std::cout << "\e[33m-----------Autoindex BODY-----------" << std::endl;
		std::cout << HTMLbody << std::endl;
		std::cout << "-----------Autoindex BODY-----------\e[0m" << std::endl;
	}
	_body = v_c(HTMLbody.begin(), HTMLbody.end());
	return (1);
}
