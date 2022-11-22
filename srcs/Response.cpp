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

Response::Response(int clientFd, Request *request, Config *config, int statusCode)
{
	_clientFd = clientFd;
	_request = request;
	_config = config;
	_statusCode = statusCode;
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
			_createBodyFromFile(actualTarget);
		else
			errorPage = "";
	}
	if (errorPage == "")
	{
		_bodyToSend = _errorBodyTemplate;
		for (int i = 0; i < 2; i++)
		{
			size_t pos = _bodyToSend.find("Error_placeholder");
			_bodyToSend.erase(pos, strlen("Error_placeholder"));
			_bodyToSend.insert(pos, errorMessage);
		}
		_body = v_c(_bodyToSend.begin(), _bodyToSend.end());
	}
	if (DEBUG_RESPONSE)
		std::cout << _bodyToSend;
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

void Response::_createBodyFromFile(const string &actualTarget)
{
	std::ifstream	fs;
	char			*buff;
	int				length;

	std::cout << "open " << actualTarget << std::endl;
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
	std::cout << "Extension : [" << extension << "]" << std::endl;
	return (extension);
}

void Response::_methodGET(void)
{
	std::string	actualTarget;
	std::string	selectActualTargetResult;
	std::string	cgiExecutable;
	
	selectActualTargetResult = _selectActualTarget(actualTarget, _request->getTarget());
	std::cout << "Actual target : [" << actualTarget << "]" << std::endl;
	if (selectActualTargetResult == "Do_listing")
		_createAutoIndex(actualTarget);
	else if (selectActualTargetResult != "Index_file_nok" && selectActualTargetResult != "File_nok")
	{
		std::string rawTarget = _request->getTarget();
		if ((cgiExecutable = _config->getCgiByLocation(rawTarget, _getExtensionFromTarget(actualTarget))) != "")
		{
			std::cout << "\e[33mCGI\e[0m" << std::endl;
			_handleCGI(actualTarget, cgiExecutable);
		}
		else
			_createBodyFromFile(actualTarget);
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

void Response::_handleCGI(string actualTarget, string cgiExecutable)
{
	pid_t	pid;
	int		pipefdParentToChild[2];
	int		pipefdChildToParent[2];
	int		status;
	int		ret;
	int		readRet;
	char	readBuf[512];
	v_c		requestBody = _request->getBody();

	if (pipe(pipefdParentToChild))
		throw(std::runtime_error("Pipe error" ));
	if (pipe(pipefdChildToParent))
		throw(std::runtime_error("Pipe error" ));
	if ((pid = fork()) == -1)
		throw(std::runtime_error("Fork error" ));
	if (pid != 0)
	{
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
void	Response::_extractHeaderFromCgiBody()
{
	v_c::iterator	it = _body.begin();
	v_c::iterator	ite = _body.end();

	for (; it != ite; it++)
	{
		if (*it == '\r'
			&& it + 1 != ite && *(it + 1) == '\n'
			&& it + 2 != ite && *(it + 2) == '\r'
			&& it + 3 != ite && *(it + 3) == '\n')
		{
			string rawHeader(_body.begin(), it);
			_body.erase(_body.begin(), it + 4);
			_header += rawHeader + "\n";
		}
	}
}

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

void Response::_createFullResponse(void)
{
	_fullResponse = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullResponse.insert(_fullResponse.end(), _header.begin(), _header.end());
	_fullResponse.push_back('\n');
	_fullResponse.insert(_fullResponse.end(), _body.begin(), _body.end());
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

int Response::createResponse(void)
{
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	//check methode
	_checkRedirect();
	_checkAutorizationForMethod();	
	if(_statusCode > 200)
		_createErrorMessageBody();
	else if (_request->getMethod() == "GET")
	{
		_methodGET();
	}
	_createHeaderBase();
	_lineStatus = string(_request->getProtocol() + " " + itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second + "\r\n");
	_createFullResponse();
	return 0;
}

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
