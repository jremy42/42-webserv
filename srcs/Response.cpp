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

Response::Response()
{
	if (DEBUG_RESPONSE)
		std::cout << "Response : Default Constructor called" << std::endl; 
}

Response::Response(int clientFd, Request *request, const Config *config, int statusCode)
{
	if (DEBUG_RESPONSE)
		std::cout << "Response : Parametric Constructor called" << std::endl; 
	_clientFd = clientFd;
	_request = request;
	_config = config;
	_statusCode = statusCode;
	_state = R_INIT;
	if (DEBUG_RESPONSE)
	{
		std::cout << "Create response with request with target [" << request->getTarget() << "]" << std::endl;
		std::cout << "\e[31m--------------------Start of Config used for creation--------------------" << std::endl;
		std::cout << *_config;
		std::cout << "---------------------End of Config used for creation---------------------\e[0m" << std::endl;
	}
	_selectActualTarget();
	memset(_nameOut, 0, 32);
	memset(_nameIn, 0, 32);
	strncpy(_nameIn, "/tmp/webservXXXXXX", 32);
	strncpy(_nameOut, "/tmp/webservXXXXXX", 32);
}

Response::Response(const Response &src)
{
	if (DEBUG_RESPONSE)
		std::cout << "Response : Copy Constructor called" << std::endl; 
	*this = src;
}

Response::~Response(void)
{
	if (DEBUG_RESPONSE)
		std::cout << "Response : Default Destructor called" << std::endl; 
	if (strcmp(_nameIn, "/tmp/webservXXXXXX"))
		unlink(_nameIn);
	if (strcmp(_nameOut, "/tmp/webservXXXXXX"))
		unlink(_nameOut);
}

Response &Response::operator=(const Response &rhs)
{
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
	_bodyLength = rhs._bodyLength;

	_rawRequestedTarget = rhs._rawRequestedTarget;
	_requestedTargetRoot = rhs._requestedTargetRoot;
	_rawActualTarget = rhs._rawActualTarget;
	_actualTarget = rhs._actualTarget;
	_queryString = rhs._queryString;
	_pathInfo = rhs._pathInfo;
	_targetStatus = rhs._targetStatus;
	_cgiExecutable = rhs._cgiExecutable;
	_targetExtension = rhs._targetExtension;
	return (*this);
}

void Response::_createErrorMessageBody(void)
{
	string	requestTarget = _request->getTarget();
	string	customErrorPage = _config->getErrorPageByLocation(requestTarget, _statusCode);
	string  matchingLocationRoot = _config->getParamByLocation(requestTarget, "root").at(0);
	string	errorPageFile = matchingLocationRoot + "/" + customErrorPage;

	if (DEBUG_RESPONSE)
	{
		std::cout << "_createErrorMessageBody start" << std::endl;
		std::cout << "requestTarget : [" << requestTarget << "]" << std::endl;
		std::cout << "customErrorPage : [" << customErrorPage << "]" << std::endl;
		std::cout << "matchingLocationRoot : [" << matchingLocationRoot << "]" << std::endl;
		std::cout << "errorPageFile : [" << errorPageFile << "]" << std::endl;
	}
	if (customErrorPage != "" && fileExist(errorPageFile) && !isDir(errorPageFile))
	{
		if (DEBUG_RESPONSE)
			std::cout << "An error page is specified for this error and location" << std::endl; 
		_createFileStreamFromFile(errorPageFile);
		return ;
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cout << "No error page is specified for this error and location -> Building Body from _defaultErrorBodyToSend" << std::endl; 
		string errorMessage(itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second);
		_generateErrorBodyFromTemplate(errorMessage);
		_ss << _defaultErrorBodyToSend;
		_bodyLength = _defaultErrorBodyToSend.size();
		_header += "content-length: " + itoa(_bodyLength) + "\n";
	}
}

void	Response::_parseRawRequestTarget(void)
{
	_rawRequestedTarget = _request->getTarget();
	_requestedTargetRoot = _config->getParamByLocation(_rawRequestedTarget, "root").at(0);
	_requestedTargetRoot.erase(0, (_requestedTargetRoot[0] == '/' ? 1 : 0));
	_rawActualTarget = _requestedTargetRoot + _rawRequestedTarget;
	_actualTarget = _rawActualTarget;

	std::size_t	posLastSlash = _rawActualTarget.find_last_of("/");
	std::size_t	posLastQuestionMark = _rawActualTarget.find_last_of("?");
	std::string testFile = _rawActualTarget.substr(0, posLastSlash);
	if (fileExist(testFile) && !isDir(testFile))
	{
		_actualTarget = testFile;
		_pathInfo = _rawActualTarget.substr(posLastSlash, posLastQuestionMark - posLastSlash);   
	}
	if (posLastQuestionMark != std::string::npos)
	{
		_queryString = _rawActualTarget.substr(posLastQuestionMark + 1);	
		//posLastQuestionMark = _rawActualTarget.find_last_of("?");
		_actualTarget = _actualTarget.substr(0, posLastQuestionMark);
	}
	std::size_t	posLastDot = _actualTarget.find_last_of(".");
	if (posLastDot != std::string::npos)
	{
		_targetExtension = _actualTarget.substr(posLastDot);
		_cgiExecutable = _config->getCgiByLocation(_rawRequestedTarget, _targetExtension);
	}
	if (DEBUG_RESPONSE)
	{
		std::cout << "_rawRequestedTarget : [" << _rawRequestedTarget << "]" << std::endl;
		std::cout << "_requestedTargetRoot : [" << _requestedTargetRoot << "]" << std::endl;
		std::cout << "_rawActualTarget : [" << _rawActualTarget << "]" << std::endl;
		std::cout << "_actualTarget : [" << _actualTarget << "]" << std::endl;
		std::cout << "_queryString : [" << _queryString << "]" << std::endl;
		std::cout << "_pathInfo : [" << _pathInfo << "]" << std::endl;
		std::cout << "_targetExtension : [" << _targetExtension << "]" << std::endl;
		std::cout << "_cgiExecutable : [" << _cgiExecutable << "]" << std::endl;
		if (!fileExist(_actualTarget))
			std::cout << "actual Target does not exists" << std::endl;
		if (fileExist(_actualTarget) && isDir(_actualTarget))
			std::cout << "actual Target exist and is a Directory" << std::endl;
		else if (fileExist(_actualTarget) && !isDir(_actualTarget))
			std::cout << "actual Target exist and is a regular file" << std::endl;
	}
}

void	Response::_selectActualTarget(void)
{
	_parseRawRequestTarget();
	if (fileExist(_actualTarget) && !isDir(_actualTarget))
	{
		if (DEBUG_RESPONSE)
			std::cout << "Requested file is standard" << std::endl;
		_targetStatus = "File_ok";
	}
	else if (fileExist(_actualTarget) && isDir(_actualTarget))
	{
		std::vector<string> indexTryFiles = _config->getParamByLocation(_rawRequestedTarget, "index");
		if (DEBUG_RESPONSE)
			std::cout << "Trying files in indexTryFiles :" << indexTryFiles << std::endl;
		std::vector<string>::iterator it = indexTryFiles.begin();
		for (;it != indexTryFiles.end(); it++)
		{
			std::string	testedIndexFile = _requestedTargetRoot + _rawRequestedTarget + "/" + *it;
			if (DEBUG_RESPONSE)
				std::cout << "Testing index file :" << testedIndexFile << std::endl;
			if (fileExist(testedIndexFile) && !isDir(testedIndexFile))
			{
				_actualTarget = testedIndexFile;
				break;
			}
		}
		if (it != indexTryFiles.end())
		{
			if (DEBUG_RESPONSE)
				std::cout << "Found a suitable index file : [" << *it << "]" << std::endl;
			_targetStatus = "Index_file_ok";
		}
		else if (_config->getParamByLocation(_rawRequestedTarget, "autoindex").at(0) == "on")
		{
			if (DEBUG_RESPONSE)
				std::cout << "No suitable index file but autoindex is on. Returning Listing of directory" << std::endl;
			_targetStatus = "Do_listing";
		}
		else
		{
			if (DEBUG_RESPONSE)
				std::cout << "No suitable index file and autoindex is off" << std::endl;
			_targetStatus = "Index_file_nok";
		}
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cout << "No such file or directory" << std::endl;
		_targetStatus = "File_nok";
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
}

 //std::string	Response::_getExtensionFromTarget(string actualTarget)
 //{
 //	std::size_t	posDot;
 //	std::size_t	posEnd;
 //
 //	posDot = actualTarget.find_last_of(".");
 //	std::string endPart = actualTarget.substr(posDot);
 //	//posEnd = endPart.find_first_of("/?");
 //	//posEnd = posEnd != std::string::npos ? posEnd : endPart.size();
 //	std::string extension = endPart.substr(0, posEnd + 1);
 //	return (extension);
 //}

void Response::_methodGET(void)
{
	if (_targetStatus == "Do_listing")
	{
		_createAutoIndex(_actualTarget);
		_state = R_FILE_READY;
	}
	else if (_targetStatus != "Index_file_nok" && _targetStatus != "File_nok")
	{
		std::string rawTarget = _request->getTarget();
		if (_cgiExecutable != "")
		{
			if (DEBUG_RESPONSE && _state == R_INIT)
				std::cout << "\e[33mCGI\e[0m" << std::endl;
			if (_state == R_INIT)
				_initCGIfile();
			if (_state == R_WAIT_CGI_EXEC)
				_waitCGIfile();
		}
		else
		{
			_createFileStreamFromFile(_actualTarget);
			_state = R_FILE_READY;
		}
	}
	else
	{
		_statusCode = 404;
		_createErrorMessageBody();
		_state = R_FILE_READY;
	}
}

void Response::_methodPOST(void)
{
	//access sur le fichier droit d'ecriture 
	//if file existe append ?
	// else create file
	// create response
}

void Response::_extractHeaderFromCgiOutputFile(void)
{
	std::string extractedHeader; 

	getline(_fs, extractedHeader, '\n');
	while (extractedHeader != "\r")
	{
		_bodyLength -= (extractedHeader.size() + 1);
		_header += extractedHeader + "\n";
		getline(_fs, extractedHeader, '\n');
	}
	_bodyLength -= 2;
	if (DEBUG_RESPONSE)
		std::cout << "real CGI body length after removing header : [" << _bodyLength << "]" << std::endl;
}

void Response::_waitCGIfile(void)
{
	int		status;
	int		ret;

	if (waitpid(_pid, &status, WNOHANG) == 0)
	{
		if (DEBUG_RESPONSE > 1)
			std::cout << "A child is still working at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		return ;
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cout << "A child is done working at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		if (WIFEXITED(status) > 0)
			ret = (WEXITSTATUS(status));
		if (WIFSIGNALED(status) > 0)
			ret = (WTERMSIG(status) + 128);
		std::cerr << "ret : [" << ret << "]" << std::endl;
		if (close(_inChild))
			throw(std::runtime_error("Close error inChild" ));
		if (unlink(_nameIn))
			throw(std::runtime_error("unlink error" ));
		if (close(_outChild))
			throw(std::runtime_error("close error outChild" ));
		_createFileStreamFromFile(_nameOut);
		_extractHeaderFromCgiOutputFile();
		_state = R_FILE_READY;
	}
}

void Response::_initCGIfile(void)
{
	//v_c		requestBody = _request->getBody();
	string bodyFile = _request->getTmpBodyFile();
	long bodyFileSize = getFileSize(bodyFile);

	if ((_inChild = mkstemp(_nameIn)) == -1)
		throw(std::runtime_error(std::string("_nameIn mkstemp error") + strerror(errno)));
	if ((_outChild = mkstemp(_nameOut)) == -1)
		throw(std::runtime_error(std::string("_nameOut mkstemp error") + strerror(errno)));
	if (DEBUG_RESPONSE)
	{
		std::cout << "_nameIN : [" << _nameIn << "]" << std::endl;
		std::cout << "_nameOut : [" << _nameOut << "]" << std::endl;
		std::cout << "nameIn: [" << _nameIn << "]" << std::endl;
		std::cout << "nameOut: [" << _nameOut << "]" << std::endl;
		std::cout << "inchild fd: [" << _inChild << "]" << std::endl;
		std::cout << "outchild fd: [" << _outChild << "]" << std::endl;
		std::cout << "open " << _actualTarget << std::endl;
	}
	if (bodyFileSize != 0)
	{
		//char *buff = new char[bodyFileSize];
		//int i = 0;
		//v_c::iterator ite = requestBody.end();
		//for (v_c::iterator it = requestBody.begin(); it != ite; i++, it++)
		//	buff[i] = *it;
		//write(_inChild, buff, i);
	}
	if ((_pid = fork()) == -1)
		throw(std::runtime_error("Fork error" ));
	if (_pid != 0)
	{
		if (DEBUG_RESPONSE)
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
		arg[0] = const_cast<char *>(_cgiExecutable.c_str());
		arg[1] = const_cast<char *>(_actualTarget.c_str());
		arg[2] = NULL;
		if (DEBUG_RESPONSE)
			std::cerr << "actual Target : [" << _actualTarget << "] CGI-executable : [" << _cgiExecutable << "]" << std::endl;
		execve(_cgiExecutable.c_str(), arg, NULL);
		throw(std::runtime_error(std::string("Execve error") + strerror(errno)));
	}
}

void Response::_createFullHeader(void)
{
	_header += "content-length: " + itoa(_bodyLength) + "\n";
	_fullHeader = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullHeader.insert(_fullHeader.end(), _header.begin(), _header.end());
	_fullHeader.push_back('\n');
	if (DEBUG_RESPONSE)
		std::cout << "Full Header size : [" << _fullHeader.size() << "]" << std::endl;
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
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
	{
		std::cout << "handleResponse IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		std::cout << "Handle response start. Status [" << _state << "]" << std::endl;
	}
	if (_state != R_WRITE)
	{
		if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
			std::cout << "Calling _createResponse" << std::endl;
		_createResponse();
	}
	if (_state == R_WRITE)
	{
		std::cout << "Calling _writeClientResponse" << std::endl;
		_writeClientResponse();
	}
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cout << "Handle response end. Status [" << _state << "]" << std::endl;
	
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cout << "handleResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	if (_state == R_OVER)
		return (0);
	else
		return (1);
}

int Response::_createResponse(void)
{
	// status-line = HTTP-version SP status-code SP reason-phrase CRLF
	//check methode
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cout << "createResponse IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;

	if (_state == R_INIT)
	{
		_checkRedirect();
		_checkAutorizationForMethod();
	}
	if (_statusCode > 200 && _state == R_INIT)
	{
		_createErrorMessageBody();
		_state = R_FILE_READY;
	}
	if (_state < R_FILE_READY && _request->getMethod() == "GET")
	{
		if (DEBUG_RESPONSE > 2)
			std::cout << "GET METHOD" << std::endl;
		_methodGET(); // va set le status en R_FILE_READY a la fin du CGI ou directement si regular file
	}
	else if (_state < R_FILE_READY && _request->getMethod() == "POST")
	{
		//POST a implementer !!!
	}
	if (_state == R_FILE_READY)
	{
		_lineStatus = string(_request->getProtocol() + " " + itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second + "\r\n");
		_createFullHeader();
		_state = R_WRITE;
	}
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cout << "createResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return (_state);
}

std::istream *Response::_selectBodySourceBetweenFileAndStringStream(void)
{
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
	return (bodyStreamPtr);
}

void Response::_sendHeaderToClient(void)
{
	int		ret;
	int		buff_size;
	char	*buff;
	int		i = 0;

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

void Response::_sendBodyToClient(void)
{
	int		ret;
	int		buff_size = 0;
	char	*bufBody;
	std::istream	*bodyStreamPtr = _selectBodySourceBetweenFileAndStringStream();
	std::istream	&bodyStream = *bodyStreamPtr;

	std::cout << "Header IS empty -> sending Body" << std::endl;
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
		try
		{
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

int Response::_writeClientResponse(void)
{

	if (DEBUG_RESPONSE)
		std::cout << "write	Response IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	std::cout << "Begin of Write Client response function" << std::endl;
	if (_fullHeader.size())
		_sendHeaderToClient();
	else if (_fullHeader.empty())
		_sendBodyToClient();
	if (DEBUG_RESPONSE)
		std::cout << "writeResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return 1;
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

void	Response::_generateErrorBodyFromTemplate(std::string &errorMessage)
{
	_defaultErrorBodyToSend = _errorBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = _defaultErrorBodyToSend.find("Error_placeholder");
		_defaultErrorBodyToSend.erase(pos, strlen("Error_placeholder"));
		_defaultErrorBodyToSend.insert(pos, errorMessage);
	}
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
	//Ajout des fichiers
	for (std::map<string, unsigned int>::reverse_iterator it = dirMap.rbegin(); it != dirMap.rend(); it++)
	{
		if (it->second != DT_DIR)
		{
			std::stringstream out;
			// A clean, mais fonctionnel (A mettre dans une fonction a minima)
			out << std::left << std::setw(80 + string("<a href=\"" + it->first + "\">" + "\">").size())
				<< "<a href=\"" + cleanTargetDir + it->first + "\">" + it->first + "</a>"
				<< std::setw(40) <<  getFileSizeStr(cleanPathToDir + it->first) + " bytes" << std::endl;
			// A clean, mais fonctionnel (A mettre dans une fonction a minima)
			size_t pos = HTMLbody.find("<pre>\n");
			pos += string("<pre>\n").size();
			HTMLbody.insert(pos, out.str());
		}
	}
	//Ajout des fichiers
	//Ajout des dossiers
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
	//Ajout des dossiers
	if (DEBUG_RESPONSE)
	{
		std::cout << "\e[33m-----------Autoindex BODY-----------" << std::endl;
		std::cout << HTMLbody << std::endl;
		std::cout << "-----------Autoindex BODY-----------\e[0m" << std::endl;
	}
	_ss << HTMLbody;
	_bodyLength = HTMLbody.size();
	_header += "content-length: " + itoa(_bodyLength) + "\n";
	return (1);
}

