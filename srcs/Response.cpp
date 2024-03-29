#include "Response.hpp"
extern int g_rv;

Response::m_is Response::_statusCodeMessage = _initStatusCodeMessage();

Response::m_is Response::_initStatusCodeMessage()
{
	m_is ret;
	ret[200] = "OK";
	ret[201] = "Created";
	ret[202] = "Accepted";
	ret[203] = "Non-Authoritative Information";
	ret[204] = "No Content";
	ret[205] = "Reset Content";
	ret[206] = "Partial Content";
	ret[300] = "Multiple Choices";
	ret[301] = "Moved Permanently";
	ret[302] = "Found";
	ret[400] = "Bad Request";
	ret[401] = "Unauthorized";
	ret[402] = "Payment Required";
	ret[403] = "Forbidden";
	ret[404] = "Not Found";
	ret[405] = "Method Not Allowed";
	ret[406] = "Not Acceptable";
	ret[407] = "Proxy Authentication Required";
	ret[408] = "Request Timeout";
	ret[409] = "Conflict";
	ret[410] = "Gone";
	ret[411] = "Length Required";
	ret[412] = "Precondition Failed";
	ret[413] = "Payload Too Large";
	ret[414] = "URI Too Long";
	ret[415] = "Unsupported Media Type";
	ret[416] = "Range Not Satisfiable";
	ret[417] = "Expectation Failed";
	ret[418] = "I'm a teapot";
	ret[421] = "Misdirected Request";
	ret[422] = "Unprocessable Entity";
	ret[423] = "Locked";
	ret[424] = "Failed Dependency";
	ret[500] = "Internal Server Error";
	ret[501] = "Not Implemented";
	ret[502] = "Bad Gateway";
	ret[503] = "Service Unavailable";
	ret[504] = "Gateway Timeout";
	ret[505] = "HTTP Version Not Supported";
	ret[506] = "Variant Also Negotiates";
	ret[507] = "Insufficient Storage";
	ret[508] = "Loop Detected";
	return ret;
}

std::string Response::_errorBodyTemplate = "<html>\n<head><title>Error_placeholder</title></head>\n<body>\n<center><h1>Error_placeholder</h1></center>\n<hr><center>webserv/1.0</center>\n</body>\n</html>\n";
std::string Response::_autoIndexBodyTemplate = "<html><head><title>Index of /title_placeholder</title></head>\n<body>\n<h1>Index of /title_placeholder</h1><hr><pre>\n</pre><hr>\n</body></html>\n";

Response::m_ss Response::_initCgiMetaVar()
{
	m_ss ret;
	ret["AUTH_TYPE"] = "";
	ret["CONTENT_LENGTH"] = "";
	ret["CONTENT_TYPE"] = "";
	ret["GATEWAY_INTERFACE"] = "";
	ret["PATH_INFO"] = "";
	ret["PATH_TRANSLATED"] = "";
	ret["QUERY_STRING"] = "";
	ret["REMOTE_ADDR"] = "";
	ret["REMOTE_HOST"] = "";
	ret["REMOTE_IDENT"] = "";
	ret["REMOTE_USER"] = "";
	ret["REQUEST_METHOD"] = "";
	ret["SCRIPT_NAME"] = "";
	ret["SERVER_NAME"] = "";
	ret["SERVER_PORT"] = "";
	ret["SERVER_PROTOCOL"] = "";
	ret["SERVER_SOFTWARE"] = "";
	return ret;
}

void Response::_setProtocolSpecificMetavar(void)
{
	m_ss requestHeader = _request->getHeader();
	m_ss::iterator it = requestHeader.begin();
	m_ss::iterator ite = requestHeader.end();

	for (; it != ite; it++)
	{
		if (it->first != "Authorization" && it->first != "Content-Length" && it->first != "Content-Type")
		{
			std::string key = it->first;
			for (std::size_t i = 0; i < key.size(); i++)
				key[i] = (key[i] == '-') ? '_' : toupper(key[i]);
			key = "HTTP_" + key;
			_cgiMetaVar[key] = it->second;
		}
	}
}

void Response::_setCgiMetaVar(void)
{
	m_ss requestHeader = _request->getHeader();
	m_ss::iterator it;

	_cgiMetaVar["REDIRECT_STATUS"] = "true";
	_cgiMetaVar["AUTH_TYPE"] = (it = requestHeader.find("Authorization")) != requestHeader.end() ? subStringBeforeFirstDelim(it->second, ' ') : "";
	_cgiMetaVar["CONTENT_LENGTH"] = (_requestBodyFileSize != 0) ? ltoa(_requestBodyFileSize) : "";
	_cgiMetaVar["CONTENT_TYPE"] = (it = requestHeader.find("Content-Type")) != requestHeader.end() ? it->second : "";
	_cgiMetaVar["GATEWAY_INTERFACE"] = "CGI/1.1";
	_cgiMetaVar["PATH_INFO"] = _PATH_INFO;
	_cgiMetaVar["PATH_TRANSLATED"] = _requestedTargetRoot + _PATH_INFO;
	_cgiMetaVar["QUERY_STRING"] = _QUERY_STRING;
	_cgiMetaVar["REMOTE_ADDR"] = getClientAddrFromSocket(_clientFd);
	_cgiMetaVar["REMOTE_HOST"] = getClientHostnameAndService(_clientFd).first;
	_cgiMetaVar["REMOTE_USER"] = (it = requestHeader.find("Authorization")) != requestHeader.end() ? subStringAfterFirstDelim(it->second, ' ') : "";
	_cgiMetaVar["REQUEST_METHOD"] = _request->getMethod();
	_cgiMetaVar["SCRIPT_NAME"] = subStringAfterLastDelim(_actualTarget, '/');
	_cgiMetaVar["SCRIPT_FILENAME"] = _actualTarget != "" ? _actualTarget : "/";
	_cgiMetaVar["SERVER_NAME"] = _request->getHost();
	_cgiMetaVar["SERVER_PORT"] = getRequestedPortFromSocket(_clientFd);
	_cgiMetaVar["SERVER_PROTOCOL"] = _request->getProtocol();
	_cgiMetaVar["SERVER_SOFTWARE"] = "Jonny's and Fredo's WeBsErV";
}

char **Response::_createEnvArray(void)
{
	int size = 0;
	char **env;
	m_ss::iterator ite = _cgiMetaVar.end();

	for (m_ss::iterator it = _cgiMetaVar.begin(); it != ite; it++)
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Checking : [" << it->first << "] -> [" << it->second << "]" << std::endl;
		if (it->second != "")
			++size;
	}
	if (DEBUG_RESPONSE)
		std::cerr << "Env Array non-empty values : [" << size << "]" << std::endl;
	env = new char *[size + 1];
	env[size] = NULL;
	for (m_ss::iterator it = _cgiMetaVar.begin(); it != ite; it++)
	{
		if (it->second != "")
		{
			*env = new char[it->first.size() + it->second.size() + 2];
			memset(*env, 0, it->first.size() + it->second.size() + 2);
			strcpy(*env, it->first.c_str());
			strcat(*env, "=");
			strcat(*env, it->second.c_str());
			++env;
		}
	}
	env -= size;
	int i = 0;
	while (env[i])
	{
		if (DEBUG_RESPONSE)
			std::cerr << "CGI Env[" << i << "] : [" << env[i] << "]m" << std::endl;
		++i;
	}
	return (env);
}

Response::Response()
{
	if (DEBUG_RESPONSE)
		std::cerr << "Response : Default Constructor called" << std::endl;
}

Response::Response(int clientFd, Request *request, const Config *config, int statusCode)
{
	if(config == NULL)
		throw std::runtime_error("Response : Config is NULL");
	if (DEBUG_RESPONSE)
		std::cerr << "Response : Parametric Constructor called" << std::endl;
	_clientFd = clientFd;
	_request = request;
	_config = config;
	_statusCode = statusCode;
	_state = R_INIT;
	_pid = -2;
	_bodyLength = 0;
	_outChild = -1;
	_inChild = -1;
	_ourNameOut = "";
	if (DEBUG_RESPONSE)
	{
		std::cerr << "Create response with request with target [" << request->getTarget() << "]" << std::endl;
		std::cerr << "\e[31m--------------------Start of Config used for creation--------------------" << std::endl;
		std::cerr << *_config;
		std::cerr << "---------------------End of Config used for creation---------------------\e[0m" << std::endl;
	}
	_selectActualTarget();
	memset(_nameOut, 0, 32);
	strncpy(_nameOut, "/tmp/webservXXXXXX", 32);
	_requestBodyFile = _request->getTmpBodyFile();
	_requestBodyFileSize = (_requestBodyFile != "") ? getFileSize(_requestBodyFile) : 0;
}

Response::Response(const Response &src)
{
	if (DEBUG_RESPONSE)
		std::cerr << "Response : Copy Constructor called" << std::endl;
	*this = src;
}

Response::~Response(void)
{
	if (DEBUG_RESPONSE)
		std::cerr << "Response : Default Destructor called" << std::endl;
	if (_ourNameOut != "")
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Response : Deleting temp file [" << _ourNameOut << "] and close fd :" <<_outChild << std::endl;
		if (close(_outChild))
			std::cerr << "Warning : Response close error" << std::endl;
		if (unlink(_ourNameOut.c_str()))
			std::cerr << "Warning : Response unlink error" << std::endl;
	}
	if (_pid != -1)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
	}
}

Response &Response::operator=(const Response &rhs)
{
	_clientFd = rhs._clientFd;
	_statusCode = rhs._statusCode;
	_state = rhs._state;
	_lineStatus = rhs._lineStatus;
	_header = rhs._header;
	_body = rhs._body;
	_fullHeader = rhs._fullHeader;
	_defaultErrorBodyToSend = rhs._defaultErrorBodyToSend;
	_request = rhs._request;
	_config = rhs._config;
	_bodyLength = rhs._bodyLength;

	_matchingLocation = rhs._matchingLocation;
	_rawRequestedTarget = rhs._rawRequestedTarget;
	_requestedTargetRoot = rhs._requestedTargetRoot;
	_rawActualTarget = rhs._rawActualTarget;
	_actualTarget = rhs._actualTarget;
	_targetExtension = rhs._targetExtension;
	_targetStatus = rhs._targetStatus;
	_cgiExecutable = rhs._cgiExecutable;

	_PATH_INFO = rhs._PATH_INFO;
	_QUERY_STRING = rhs._QUERY_STRING;
	_cgiMetaVar = rhs._cgiMetaVar;

	_requestBodyFile = rhs._requestBodyFile;
	_requestBodyFileSize = rhs._requestBodyFileSize;

	_pid = rhs._pid;
	strcpy(_nameOut, rhs._nameOut);
	_inChild = rhs._inChild;
	_outChild = rhs._outChild;
	_ourNameOut = rhs._ourNameOut;
	_postFileName = rhs._postFileName;
	return (*this);
}

std::string Response::getlineStatus(void)
{
	string lineStatusTrim = strtrim(_lineStatus, "\t\n\r ");
	return lineStatusTrim;
}
void Response::_createErrorMessageBody(void)
{
	string customErrorPage;
	//string customErrorPage = _config->getErrorPageByLocation(_rawRequestedTarget, _statusCode);
	if (_config->getLocation().find(_matchingLocation) != _config->getLocation().end())
		customErrorPage = _config->getLocation().find(_matchingLocation)->second.getErrorPageWithCode(_statusCode);
	if (customErrorPage == "")
		customErrorPage = _config->getErrorPageWithCode(_statusCode);

	string errorPageFile = _requestedTargetRoot + "/" + customErrorPage;

	if (DEBUG_RESPONSE)
	{
		std::cerr << "_createErrorMessageBody start" << std::endl;

		std::cerr << "customErrorPage : [" << customErrorPage << "]" << std::endl;
		std::cerr << "errorPageFile : [" << errorPageFile << "]" << std::endl;
	}
	if (customErrorPage != "" && fileExist(errorPageFile) && !isDir(errorPageFile))
	{
		if (DEBUG_RESPONSE)
			std::cerr << "An error page is specified for this error and location" << std::endl;
		_createFileStreamFromFile(errorPageFile);
		return;
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cerr << "No error page is specified for this error and location -> Building Body from _defaultErrorBodyToSend" << std::endl;
		if (_statusCodeMessage.find(_statusCode) == _statusCodeMessage.end())
			_statusCode = 500;
		string errorMessage(itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second);
		_generateErrorBodyFromTemplate(errorMessage);
		_ss << _defaultErrorBodyToSend;
		_bodyLength = _defaultErrorBodyToSend.size();
	}
}

int Response::_urlDecodeString(string &strToDecode)
{
	std::size_t i = 0;
	std::size_t strSize = strToDecode.size();
	std::string base = "0123456789abcdef";

	for (; i + 2 < strSize; i++)
	{
		if (strToDecode[i] == '%' && isxdigit(strToDecode[i + 1]) && isxdigit(strToDecode[i + 2]))
		{
			char decodedChar = (base.find(tolower(strToDecode[i + 1])) << 4) + base.find(tolower(strToDecode[i + 2]));
			strToDecode.erase(i, 3);
			strToDecode.insert(strToDecode.begin() + i, decodedChar);
		}
	}
	return (1);
}

void Response::_cleanRawRequestTarget(void)
{
	std::stack<std::string> targetParts;
	std::stringstream _rawRequestedTargetSteam(_rawRequestedTarget);
	std::string buf;

	while (getline(_rawRequestedTargetSteam, buf, '/'))
	{
		if (!_rawRequestedTargetSteam.eof())
			buf += "/";
		if (buf == "../" && !targetParts.empty())
			targetParts.pop();
		else if (buf != "./")
			targetParts.push(buf);
	}
	_rawRequestedTarget.erase();
	while (!targetParts.empty())
	{
		_rawRequestedTarget = targetParts.top() + _rawRequestedTarget;
		targetParts.pop();
	}
	if (_rawRequestedTarget.empty())
		_rawRequestedTarget = "/";
	if (DEBUG_RESPONSE)
		std::cerr << "clean target: [" << _rawRequestedTarget << "]" << std::endl;
}

void Response::_parseRawRequestTarget(void)
{
	_rawRequestedTarget = _request->getTarget();
	_cleanRawRequestTarget();
	_urlDecodeString(_rawRequestedTarget);
	_matchingLocation = _config->getMatchingLocation(_rawRequestedTarget);
	_requestedTargetRoot = _config->getParamByLocation(_rawRequestedTarget, "root").at(0);
	_requestTargetPartMatchedWithLocation = _rawRequestedTarget.substr(0, _matchingLocation.size());
	_requestTargetPartMatchedWithLocation += _requestTargetPartMatchedWithLocation[_requestTargetPartMatchedWithLocation.size() - 1] == '/' ? "" : "/";
	if (_matchingLocation.size() > _rawRequestedTarget.size())
		throw std::runtime_error("Matching location is bigger than raw request target");
	std::string requestTargetWithOutLocation = _rawRequestedTarget.substr(_matchingLocation.size(), _rawRequestedTarget.size());
	requestTargetWithOutLocation.erase(0, (requestTargetWithOutLocation[0] == '/' ? 1 : 0));
 	if (_requestedTargetRoot.size() > 0)
		_requestedTargetRoot += _requestedTargetRoot[_requestedTargetRoot.size() - 1] == '/' ? "" : "/"; 
	_rawActualTarget = _requestedTargetRoot + requestTargetWithOutLocation;
	_actualTarget = _rawActualTarget;


	std::size_t posFirstSlash = _rawActualTarget.find_first_of("/", _requestedTargetRoot.size());
	std::size_t posLastQuestionMark = _rawActualTarget.find_last_of("?");
	if (posLastQuestionMark != std::string::npos)
	{
		_QUERY_STRING = _rawActualTarget.substr(posLastQuestionMark + 1);
		_actualTarget = _actualTarget.substr(0, posLastQuestionMark);

	}
	_checkReturnDir = _rawActualTarget.substr(0, posLastQuestionMark);

	while (posFirstSlash != std::string::npos)
	{
		std::string testFile = _rawActualTarget.substr(0, posFirstSlash);

		if (fileExist(testFile) && !isDir(testFile))
		{
			_actualTarget = testFile;
			_PATH_INFO = _rawActualTarget.substr(posFirstSlash, posLastQuestionMark - posFirstSlash);
			break;
		}
		posFirstSlash = _rawActualTarget.find_first_of("/", posFirstSlash + 1);
	}

	std::size_t posLastDot = _actualTarget.find_last_of(".");
	if (posLastDot != std::string::npos)
	{
		_targetExtension = _actualTarget.substr(posLastDot);

		//_cgiExecutable = _config->getCgiByLocation(_rawRequestedTarget, _targetExtension);
		if (_config->getLocation().find(_matchingLocation) != _config->getLocation().end())
			_cgiExecutable = _config->getLocation().find(_matchingLocation)->second.getCgiExecWithExtension(_targetExtension);
		else
			_cgiExecutable = "";
		
	}
	_urlDecodeString(_PATH_INFO);
	if (DEBUG_RESPONSE_TARGET)
	{
		std::cout << "_rawRequestedTarget: " << _rawRequestedTarget << std::endl;
		std::cout << "_matchingLocation: " << _matchingLocation << std::endl;
		std::cerr << "_rawRequestedTarget : [" << _rawRequestedTarget << "]" << std::endl;
		std::cerr << "_requestedTargetRoot : [" << _requestedTargetRoot << "]" << std::endl;
		std::cerr << "_rawActualTarget : [" << _rawActualTarget << "]" << std::endl;
		std::cout <<"_checkReturnDir: [" <<  _checkReturnDir << "]" << std::endl;
		std::cerr << "_actualTarget : [" << _actualTarget << "]" << std::endl;
		std::cerr << "_QUERY_STRING : [" << _QUERY_STRING << "]" << std::endl;
		std::cerr << "_PATH_INFO : [" << _PATH_INFO << "]" << std::endl;
		std::cerr << "_targetExtension : [" << _targetExtension << "]" << std::endl;
		std::cerr << "_cgiExecutable : [" << _cgiExecutable << "]" << std::endl;
		std::cerr << "_matchingLocation : [" << _matchingLocation << "]" << std::endl;
		std::cerr << "requestTargetWithOutLocation : [" << requestTargetWithOutLocation << "]" << std::endl;
		if (!fileExist(_actualTarget))
			std::cerr << "actual Target does not exists" << std::endl;
		if (fileExist(_actualTarget) && isDir(_actualTarget))
			std::cerr << "actual Target exist and is a Directory" << std::endl;
		else if (fileExist(_actualTarget) && !isDir(_actualTarget))
			std::cerr << "actual Target exist and is a regular file" << std::endl;
	}
}

void Response::_selectActualTarget(void)
{
	_parseRawRequestTarget();
	_rawRequestedTargetWithOutQuery = _rawRequestedTarget.substr(0, _rawRequestedTarget.find_last_of("?"));
	if (!accessFileParentDir(_actualTarget))
	{
		_targetStatus = "Forbidden";
		_statusCode = 403;
		return;
	}
	if (fileExist(_actualTarget) && !isDir(_actualTarget))
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Requested file is standard" << std::endl;
		_targetStatus = "File_ok";
	}
	else if (fileExist(_actualTarget) && isDir(_actualTarget) && _rawRequestedTargetWithOutQuery[_rawRequestedTargetWithOutQuery.size() - 1] != '/')
	{
		_targetStatus = "Redirect_directory";
	}
	else if (fileExist(_actualTarget) && isDir(_actualTarget) && _actualTarget.size() > 0 &&_actualTarget[_actualTarget.size() - 1] == '/')
	{
		std::vector<string> indexTryFiles = _config->getParamByLocation(_rawRequestedTarget, "index");
		if (DEBUG_RESPONSE)
			std::cerr << "Trying files in indexTryFiles :" << indexTryFiles << std::endl;
		std::vector<string>::iterator it = indexTryFiles.begin();
		for (; it != indexTryFiles.end(); it++)
		{
			std::string testedIndexFile = _actualTarget + "/" + *it;
			if (DEBUG_RESPONSE)
				std::cerr << "Testing index file :" << testedIndexFile << std::endl;
			if (fileExist(testedIndexFile) && !isDir(testedIndexFile))
			{
				_actualTarget = testedIndexFile;
				break;
			}
		}
		if (it != indexTryFiles.end())
		{
			if (DEBUG_RESPONSE)
				std::cerr << "Found a suitable index file : [" << *it << "]" << std::endl;
			_targetStatus = "Index_file_ok";
		}
		else if (_config->getParamByLocation(_rawRequestedTarget, "autoindex").at(0) == "on")
		{
			if (DEBUG_RESPONSE)
				std::cerr << "No suitable index file but autoindex is on. Returning Listing of directory" << std::endl;
			_targetStatus = "Do_listing";
		}
		else
		{
			if (DEBUG_RESPONSE)
				std::cerr << "No suitable index file and autoindex is off" << std::endl;
			_targetStatus = "Index_file_nok";
		}
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cerr << "No such file or directory" << std::endl;
		_targetStatus = "File_nok";
	}
}

void Response::_createFileStreamFromFile(string actualTarget)
{
	if (DEBUG_RESPONSE)
		std::cerr << "createFsfrom file : open " << actualTarget << std::endl;
	_fs.open(actualTarget.c_str(), std::ifstream::in | std::ifstream::binary);
	if (_fs.good())
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Successfully opened body file " << std::endl;
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Failure opening body file '" << strerror(errno) << std::endl;
		_statusCode = 403;
		_fs.close();
		return;
	}
	_fs.seekg(0, _fs.end);
	_bodyLength = _fs.tellg();
	_fs.seekg(0, _fs.beg);
	if (DEBUG_RESPONSE)
		std::cerr << "Body length: [" << _bodyLength << "]\n";
}

void Response::_methodGET(void)
{
	if (_targetStatus == "Do_listing")
	{
		_createAutoIndex(_actualTarget);
		_state = R_FILE_READY;
	}
	else if (_targetStatus != "Index_file_nok" && _targetStatus != "File_nok" && _targetStatus != "Forbidden")
	{
		std::string rawTarget = _request->getTarget();
		if (_cgiExecutable != "")
		{
			if (DEBUG_RESPONSE && _state == R_INIT)
				std::cerr << "\e[33mCGI\e[0m" << std::endl;
			if (_state == R_INIT)
				_initCGIfile();
			if (_state == R_WAIT_CGI_EXEC)
				_waitCGIfile();
		}
		else
		{
			_createFileStreamFromFile(_actualTarget);
			if (_statusCode > 400)
			{
				_createErrorMessageBody();
				_state = R_FILE_READY;
			}
			else
			{
				_state = R_FILE_READY;
			}
		}
	}
	else
	{
		if (_targetStatus == "Forbidden")
			_statusCode = 403;
		else
			_statusCode = 404;
		_createErrorMessageBody();
		_state = R_FILE_READY;
	}
}

void Response::_monoPartFile(void)
{
	std::fstream retrieveRequestBody;
	retrieveRequestBody.open(_requestBodyFile.c_str(), std::ofstream::binary | std::ifstream::in);
	if (!retrieveRequestBody.good())
		throw(std::runtime_error(string("Monopart: cannot open body file: ") + strerror(errno)));
	_postFileName = _request->getUploadDir() + "/" + tmpFileName("file_") + itoa(ft_get_time_sec());
	std::fstream fileToWrite;
	if (DEBUG_RESPONSE)
		std::cerr << "file name : " << _postFileName << std::endl;
	fileToWrite.open(_postFileName.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::app);
	if (!fileToWrite.good())
		throw(std::runtime_error(string("Monopart: cannot open file to write: ") + strerror(errno)));
	fileToWrite << retrieveRequestBody.rdbuf();
	fileToWrite.close();
	retrieveRequestBody.close();
}

void Response::_chunkedPartFile(void)
{
	std::fstream retrieveRequestChunkedBody;
	std::string buff;

	if (DEBUG_RESPONSE)
		std::cerr << "Chunked : RequestChunkedBodyfilename : " << _requestBodyFile << std::endl;
	retrieveRequestChunkedBody.open(_requestBodyFile.c_str(), std::ofstream::binary | std::ifstream::in);
	if (!retrieveRequestChunkedBody.good())
		throw(std::runtime_error(string("Chunked: cannot open body file: ") + strerror(errno)));
	_postFileName = _request->getUploadDir() + "/" + tmpFileName("file_") + itoa(ft_get_time_sec());
	if (DEBUG_RESPONSE)
		std::cerr << "file name : " << _postFileName << std::endl;
	std::fstream fileToWrite;
	fileToWrite.open(_postFileName.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::app);
	if (!fileToWrite.good())
	{
		retrieveRequestChunkedBody.close();
		throw(std::runtime_error(string("Chunked: cannot create file in upload dir. (Hint : check existence and permission for upload dir) : ") + strerror(errno)));
	}

	while (getline(retrieveRequestChunkedBody, buff, '\n'))
	{
		unsigned long size = strtol(buff.c_str(), NULL, 16);
		if (size > MAX_BODY_SIZE_HARD_LIMIT)
		{
			fileToWrite.close();
			retrieveRequestChunkedBody.close();
			unlink(_postFileName.c_str());
			throw(std::runtime_error(string("Chunked: bad request : wrong chunked part")));
		}
		if (size == 0)
			break;
		char *buffChunked = new char[size];
		retrieveRequestChunkedBody.read(buffChunked, size);
		if (!retrieveRequestChunkedBody.good())
		{
			delete[] buffChunked;
			fileToWrite.close();
			retrieveRequestChunkedBody.close();
			unlink(_postFileName.c_str());
			throw(std::runtime_error(string("Chunked: cannot read body file: ") + strerror(errno)));
		}
		fileToWrite.write(buffChunked, size);
		if (!fileToWrite.good())
		{
			delete[] buffChunked;
			fileToWrite.close();
			retrieveRequestChunkedBody.close();
			unlink(_postFileName.c_str());
			throw(std::runtime_error(string("Chunked: cannot write Newfile: ") + strerror(errno)));
		}
		delete[] buffChunked;
	}

	fileToWrite.close();
	retrieveRequestChunkedBody.close();
}

void Response::_methodPOST(void)
{

	if (_cgiExecutable != "")
	{
		if (DEBUG_RESPONSE && _state == R_INIT)
			std::cerr << "\e[33mCGI\e[0m" << std::endl;
		if (_state == R_INIT)
			_initCGIfile();
		if (_state == R_WAIT_CGI_EXEC)
			_waitCGIfile();
	}
	else
	{
		try
		{
			if (_request->getContentType().size() > 0 && _request->getContentType()[0] != "multipart/form-data")
			{
				if (_request->getTransfertEncoding() == "chunked")
					_chunkedPartFile();
				else
					_monoPartFile();
				_statusCode = 201;
				_state = R_FILE_READY;
				_ss << "File uploaded: ["  << _postFileName << "]\n";
				_bodyLength = _ss.str().size();
			}
			else
			{
				if (_request->getBoundaryDelim() == "")
					throw(std::runtime_error("bad request : No boundary delimiter found"));
				Multipart multipart(_requestBodyFile, _request->getBoundaryDelim(), _request->getUploadDir());
				multipart.createFilesFromBody();
				_state = R_FILE_READY;
				_ss << multipart.getReturnMessage();
				_bodyLength = multipart.getReturnMessage().size();
				if (multipart.getError())
					_statusCode = 206;
				else
					_statusCode = 201;
			}
		}
		catch (std::exception &e)
		{
			std::string ret(e.what());
			if (ret.find("bad request : ") != std::string::npos)
				_statusCode = 400;
			else
				_statusCode = 500;
			std::cerr << "Error in POST method : " << e.what() << std::endl;
			_createErrorMessageBody();
			_state = R_FILE_READY;
		}
	}
}

void Response::_methodDELETE(void)
{
	int ret = 0;

	if (_cgiExecutable != "")
	{
		if (DEBUG_RESPONSE && _state == R_INIT)
			std::cerr << "\e[33mCGI\e[0m" << std::endl;
		if (_state == R_INIT)
			_initCGIfile();
		if (_state == R_WAIT_CGI_EXEC)
			_waitCGIfile();
	}
	else 
	{
		if (_targetStatus != "File_nok" && _targetStatus != "Forbidden" && isDir(_actualTarget))
			ret = rmdir(_actualTarget.c_str());
		else if (_targetStatus == "File_ok")
			ret = unlink(_actualTarget.c_str());
		else
		{
			_statusCode = 404;
			_createErrorMessageBody();
			_state = R_FILE_READY;
			return;
		}
		if (ret == 0)
		{
			_statusCode = 200;
			_ss << _actualTarget << " : Successfully deleted\n";
		}
		else
		{
			_statusCode = 500;
			_ss << _actualTarget << " : " << strerror(errno) << std::endl;
		}
		_bodyLength = _ss.str().size();
		_state = R_FILE_READY;
	}
}

void Response::_extractHeaderFromCgiOutputFile(void)
{
	std::string extractedHeader;

	getline(_fs, extractedHeader, '\n');
	while (extractedHeader != "\r")
	{
		if (subStringBeforeFirstDelim(extractedHeader, ':') == "Status")
		{
			_statusCode = atoi(subStringAfterFirstDelim(extractedHeader, ':').c_str());
			if (DEBUG_RESPONSE)
				std::cerr << "Status code : " << _statusCode << std::endl;
		}
		_bodyLength -= (extractedHeader.size() + 1);
		_header += extractedHeader + "\n";
		if (!getline(_fs, extractedHeader, '\n'))
			return;
	}
	_bodyLength -= 2;

	if (DEBUG_RESPONSE)
		std::cerr << "real CGI body length after removing header : [" << _bodyLength << "]" << std::endl;
}

void Response::_waitCGIfile(void)
{
	int status;
	int ret;

	if (waitpid(_pid, &status, WNOHANG) == 0)
	{
		if (DEBUG_RESPONSE > 1)
			std::cerr << "A child is still working at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		return;
	}
	else
	{
		if (DEBUG_RESPONSE)
			std::cerr << "A child is done working at [\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		if (WIFEXITED(status) > 0)
			ret = (WEXITSTATUS(status));
		if (WIFSIGNALED(status) > 0)
			ret = (WTERMSIG(status));
		if (DEBUG_RESPONSE)
			std::cerr << "ret : [" << ret << "]" << std::endl;
		if (_requestBodyFileSize != 0 && close(_inChild))
			throw(std::runtime_error("Close error inChild"));
		if (DEBUG_RESPONSE)
			std::cerr << "client fd:" << _clientFd <<"| \e[33m pid [" << _pid << "] close outchild fd: [" << _outChild << "]\e[0m" << std::endl;
		if (ret > 0)
		{
			_statusCode = 500;
			_state = R_INIT;
			return;
		}
		_createFileStreamFromFile(_ourNameOut);
		_extractHeaderFromCgiOutputFile();
		_state = R_FILE_READY;
	}
}

void Response::_initCGIfile(void)
{
	_ourNameOut = tmpFileName("/tmp/");
	if ((_outChild = open(_ourNameOut.c_str(), O_RDWR | O_CREAT, 0777)) == -1)
		throw(std::runtime_error(std::string("_nameOut open error: ") + strerror(errno)));
	if (DEBUG_RESPONSE)
		std::cerr <<"client fd:"<< _clientFd << " | pid :" << _pid <<"\e[32m open outchild fd:" << _outChild << "\e[0m" << " nameOut:" << _ourNameOut << std::endl;
	if (DEBUG_RESPONSE)
	{
		std::cerr << "nameOut: [" << _ourNameOut << "]" << std::endl;
		std::cerr << "inchild fd: [" << _inChild << "]" << std::endl;
		std::cerr << "outchild fd: [" << _outChild << "]" << std::endl;
		std::cerr << "open " << _actualTarget << std::endl;
	}
	if (_requestBodyFileSize != 0)
	{
		if ((_inChild = open(_requestBodyFile.c_str(), O_RDONLY)) == -1)
			throw(std::runtime_error(std::string("_open error request bodyfile: ") + strerror(errno)));
	}
	if ((_pid = fork()) == -1)
		throw(std::runtime_error("Fork error"));
	if (_pid != 0)
	{
		if (DEBUG_RESPONSE)
			std::cerr << "A child is born at [\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		_state = R_WAIT_CGI_EXEC;
		return;
	}
	else
	{
		_pid = -1;
		if (_requestBodyFileSize != 0 && dup2(_inChild, STDIN_FILENO) == -1)
			throw(std::runtime_error(std::string("Child error : DUP2 error 0 : ") + strerror(errno)));
		if (dup2(_outChild, STDOUT_FILENO) == -1)
			throw(std::runtime_error(std::string("Child error : DUP2 error 1 : ") + strerror(errno)));
		_cgiMetaVar = _initCgiMetaVar();
		_setCgiMetaVar();
		_setProtocolSpecificMetavar();
		char *arg[3];
		arg[0] = const_cast<char *>(_cgiExecutable.c_str());
		arg[1] = const_cast<char *>(_actualTarget.c_str());
		arg[2] = NULL;
		if (DEBUG_RESPONSE)
			std::cerr << "actual Target : [" << _actualTarget << "] CGI-executable : [" << _cgiExecutable << "]" << std::endl;
		execve(_cgiExecutable.c_str(), arg, _createEnvArray());
		throw(std::runtime_error(std::string("Child error : error : ") + strerror(errno)));
	}
}

void Response::_createFullHeader(void)
{
	_header += "content-length: " + itoa(_bodyLength) + "\n";
	_handleCookie();
	_fullHeader = v_c(_lineStatus.begin(), _lineStatus.end());
	_fullHeader.insert(_fullHeader.end(), _header.begin(), _header.end());
	_fullHeader.push_back('\n');
	if (DEBUG_RESPONSE)
		std::cerr << "Full Header size : [" << _fullHeader.size() << "]" << std::endl;
}

void Response::_handleCookie(void)
{
	m_ss	requestHeader = _request->getHeader();
	string	headerCookieVal = requestHeader.find("Cookie") != requestHeader.end() ? requestHeader.find("Cookie")->second : "";

	if (headerCookieVal != "")
	{
		printLog(1, _clientFd, 1, string("Header Cookie : [" + headerCookieVal + "]").c_str());
		if (SHOW_COOKIE)
			std::cerr << "Request Header Cookie : [" << headerCookieVal << "]" << std::endl;
	}
	else
	{
		if (SHOW_COOKIE)
			std::cerr << "Request Header Cookie is not set" << std::endl;
	}
	if (headerCookieVal.find("Webserv-cookie=") != std::string::npos)
	{
		if (SHOW_COOKIE > 1)
			std::cerr << "Updating the Webserv cookie value" << std::endl;
		int oldCookieVal = atoi(headerCookieVal.substr(headerCookieVal.find("Webserv-cookie=") + 15).c_str());
		if (SHOW_COOKIE > 1)
			std::cerr << "Old Value was : [" << oldCookieVal << "]" << std::endl;
		_header += string("Set-Cookie: Webserv-cookie=") + itoa(oldCookieVal + 1) + "; Max-Age=10 ; Path=/\n";
	}
	else
	{
		if (SHOW_COOKIE > 1)
			std::cerr << "Setting the Webserv cookie" << std::endl;
		_header += "Set-Cookie: Webserv-cookie=0; Max-Age=10 ; Path=/\n";
	}

}
void Response::_checkAutorizationForMethod(void)
{
	string requestTarget = _request->getTarget();
	std::vector<string> allowedMethod = _config->getParamByLocation(requestTarget, "allowed_method");

	if (find(allowedMethod.begin(), allowedMethod.end(), _request->getMethod()) == allowedMethod.end())
		_statusCode = 405;
}

void Response::_returnDir(void)
{
	if (_header.find("location: ") != string::npos)
		return;
	if (_targetStatus == "Redirect_directory")
	{
		_statusCode = 301;
		_header += "location: " + _rawRequestedTarget + "/";
		if (_QUERY_STRING.size() > 0) 
			_header += "?" + _QUERY_STRING;
		_header += "\n";
	}
}

void Response::_checkRedirect(void)
{
	if (_header.find("location: ") != string::npos)
		return;
	string requestTarget = _request->getTarget();
	if (_config->getParamByLocation(requestTarget, "return")[0] == "no redirect")
		return;
	else
	{
		if (DEBUG_RESPONSE)
			std::cerr << "REDIRECT\n";
		_statusCode = atoi(_config->getParamByLocation(requestTarget, "return")[0].c_str());
		_header += "location: " + _config->getParamByLocation(requestTarget, "return")[1] + "\n";
	}
}

int Response::handleResponse(void)
{
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
	{
		std::cerr << "handleResponse IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;
		std::cerr << "Handle response start. Status [" << _state << "]" << std::endl;
	}
	if (_state != R_WRITE)
	{
		if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
			std::cerr << "Calling _createResponse" << std::endl;
		_createResponse();
	}
	if (_state == R_WRITE)
	{
		if (DEBUG_RESPONSE)
			std::cerr << "Calling _writeClientResponse" << std::endl;
		_writeClientResponse();
	}
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cerr << "Handle response end. Status [" << _state << "]" << std::endl;

	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cerr << "handleResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	if (_state == R_OVER && _statusCode < 400)
		return (0);
	else if (_state == R_OVER)
		return (-1);
	else
		return (1);
}

void Response::_discardStaticContentWithPathInfo(void)
{
	if (_cgiExecutable == "" && _PATH_INFO.size() > 0 && _targetStatus == "File_ok")
		_statusCode = 404;
}

int Response::_createResponse(void)
{

	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cerr << "createResponse IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl;

	if (_state == R_INIT)
	{		
		_checkRedirect();
		_returnDir();
		_checkAutorizationForMethod();
		_discardStaticContentWithPathInfo();
	}
	if (_statusCode > 308 && _state == R_INIT)
	{
		_createErrorMessageBody();
		_state = R_FILE_READY;
	}
	else if (_statusCode >= 300 && _statusCode <= 308 && _state == R_INIT)
	{
		_state = R_FILE_READY;
	}
	if (_state < R_FILE_READY && _request->getMethod() == "GET")
	{
		if (DEBUG_RESPONSE > 2)
			std::cerr << "GET METHOD" << std::endl;
		_methodGET(); 
	}
	else if (_state < R_FILE_READY && _request->getMethod() == "POST")
	{
		_methodPOST();
	}
	else if (_state < R_FILE_READY && _request->getMethod() == "DELETE")
	{
		_methodDELETE();
	}
	if (_state == R_FILE_READY)
	{
		_lineStatus = string(string("HTTP/1.1") + " " + itoa(_statusCode) + " " + _statusCodeMessage.find(_statusCode)->second + "\r\n");
		_createFullHeader();
		_state = R_WRITE;
	}
	if (DEBUG_RESPONSE && _state != R_WAIT_CGI_EXEC)
		std::cerr << "createResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return (_state);
}

std::istream *Response::_selectBodySourceBetweenFileAndStringStream(void)
{
	std::istream *bodyStreamPtr;

	if (_fs.is_open())
	{
		bodyStreamPtr = &_fs;
		if (DEBUG_RESPONSE)
			std::cerr << "Body is a regular fs" << std::endl;
	}
	else
	{
		bodyStreamPtr = &_ss;
		if (DEBUG_RESPONSE)
			std::cerr << "Body is an error string stream" << std::endl;
	}
	return (bodyStreamPtr);
}

void Response::_sendHeaderToClient(void)
{
	int ret;
	int buff_size;
	char *buff = NULL;
	int i = 0;

	if (DEBUG_RESPONSE)
		std::cerr << "Header not empty -> sending it first" << std::endl;
	buff = new char[_fullHeader.size()];
	buff_size = _fullHeader.size();
	v_c::iterator ite = _fullHeader.end();
	for (v_c::iterator it = _fullHeader.begin(); i < buff_size && it != ite; i++, it++)
		buff[i] = *it;
	if (DEBUG_RESPONSE)
		std::cerr << "buff_size [" << buff_size << "]"
				  << "About to write client response on fd [" << _clientFd << "]" << std::endl;
	ret = send(_clientFd, buff, i, MSG_NOSIGNAL);
	if (ret == -1)
	{
		delete[] buff;
		buff = NULL;
		throw(std::runtime_error("Webserv: Response : write failure in send header :" + std::string(strerror(errno))));
	}
	else if (ret == 0)
	{
		delete[] buff;
		buff = NULL;
		throw(std::runtime_error("Webserv: Response : client cannot receive data :" + std::string(strerror(errno))));
	}
	else
	{
		_fullHeader.erase(_fullHeader.begin(), _fullHeader.begin() + ret);
		if (DEBUG_RESPONSE)
			std::cerr << "Sent bytes : [" << ret << "]. Remaining Content : [" << _fullHeader.size() << "]" << std::endl;
	}
	delete[] buff;
	buff = NULL;
}

void Response::_sendBodyToClient(void)
{
	int ret;
	unsigned long buff_size = 0;
	char *bufBody = NULL;
	std::istream *bodyStreamPtr = _selectBodySourceBetweenFileAndStringStream();
	std::istream &bodyStream = *bodyStreamPtr;

	if (DEBUG_RESPONSE)
		std::cerr << "Header IS empty -> sending Body" << std::endl;
	if (_bodyLength < WRITE_BUFFER_SIZE)
		buff_size = _bodyLength;
	else
		buff_size = WRITE_BUFFER_SIZE;
	bufBody = new char[buff_size];
	bodyStream.read(bufBody, buff_size);
	if (!bodyStream.good())
	{
		std::ifstream* toClose = dynamic_cast<std::ifstream*>(&bodyStream);
		if (toClose)
			toClose->close();
		delete[] bufBody;
		bufBody = NULL;
		throw(std::runtime_error("Webserv: Response : read failure"));
	}
	if (DEBUG_RESPONSE)
	{
		std::cerr << "read [" << bodyStream.gcount() << "] from body file" << std::endl;
		std::cerr << "Sending chunk of body to client" << std::endl;
	}
	ret = send(_clientFd, bufBody, bodyStream.gcount(), MSG_NOSIGNAL);
	_bodyLength -= ret;
	if (ret == -1)
	{
		delete[] bufBody;
		bufBody = NULL;
		throw(std::runtime_error("Webserv: Response : write failure in send body"));
	}
	if (ret == 0)
	{
		delete[] bufBody;
		bufBody = NULL;
		throw(std::runtime_error("Webserv: Response : client cannot receive data"));
	}
	if (ret != bodyStream.gcount() && DEBUG_RESPONSE)
		std::cerr << "\e[32mLazy client : only [" << ret << "] out of [" << bodyStream.gcount() << "]\e[0m" << std::endl;
	if (_bodyLength == 0)
	{
		if (DEBUG_RESPONSE)
			std::cerr << "No more body data to read on body fd. Attempt to Close fd" << std::endl;
		try
		{
			std::ifstream &fsForClose = dynamic_cast<std::ifstream &>(bodyStream);
			if (DEBUG_RESPONSE)
				std::cerr << "Closing fs" << std::endl;
			fsForClose.close();
		}
		catch (std::exception &e)
		{
			(void)e;
			if (DEBUG_RESPONSE)
				std::cerr << "No need to close filestream : Body is default error body" << std::endl;
		}
		_state = R_OVER;
	}
	else
		bodyStream.seekg(-(bodyStream.gcount() - ret), bodyStream.cur);
	delete[] bufBody;
	bufBody = NULL;
}

int Response::_writeClientResponse(void)
{

	if (DEBUG_RESPONSE)
		std::cerr << "write	Response IN[\e[32m" << ft_get_time_sec() << "\e[0m]" << std::endl
				  << "Begin of Write Client response function" << std::endl;
	if (_fullHeader.size())
		_sendHeaderToClient();
	else if (_fullHeader.empty())
	{
		if (_bodyLength == 0)
			_state = R_OVER;
		else
			_sendBodyToClient();
	}
	if (DEBUG_RESPONSE)
		std::cerr << "writeResponse OUT[\e[31m" << ft_get_time_sec() << "\e[0m]" << std::endl;
	return 1;
}

std::map<std::string, unsigned int> Response::_populateDirectoryMap(const char *path)
{
	struct dirent *currentDir;
	DIR *dp;
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
			dirMap.insert(std::pair<string, unsigned int>(nextDirEntry, currentDir->d_type));
		}
		currentDir = readdir(dp);
	}
	closedir(dp);
	return (dirMap);
}

void Response::_generateErrorBodyFromTemplate(std::string &errorMessage)
{
	_defaultErrorBodyToSend = _errorBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = _defaultErrorBodyToSend.find("Error_placeholder");
		_defaultErrorBodyToSend.erase(pos, strlen("Error_placeholder"));
		_defaultErrorBodyToSend.insert(pos, errorMessage);
	}
}

std::string Response::_generateHTMLBodyWithPath(void)
{
	string HTMLbody = _autoIndexBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = HTMLbody.find("/title_placeholder");
		HTMLbody.erase(pos, strlen("/title_placeholder"));
		HTMLbody.insert(pos, _rawRequestedTargetWithOutQuery);
	}
	return (HTMLbody);
}

int Response::_createAutoIndex(const string &pathToDir)
{
	const char *path = pathToDir.c_str();
	std::map<string, unsigned int> dirMap = _populateDirectoryMap(path);
	string HTMLbody = _generateHTMLBodyWithPath();
	string cleanTargetDir = _rawRequestedTargetWithOutQuery;
	string cleanPathToDir = pathToDir + (pathToDir[pathToDir.size() - 1] != '/' ? "/" : "");

	cleanTargetDir += cleanTargetDir[cleanTargetDir.size() - 1] != '/' ? "/" : "";
	// Ajout des fichiers
	for (std::map<string, unsigned int>::reverse_iterator it = dirMap.rbegin(); it != dirMap.rend(); it++)
	{
		if (it->second != DT_DIR)
		{
			std::stringstream out;
			out << std::left << std::setw(80 + string("<a href=\"" + it->first + "\">" + "\">").size())
				<< "<a href=\"" + cleanTargetDir + it->first + "\">" + it->first + "</a>"
				<< std::setw(40) << getFileSizeStr(cleanPathToDir + it->first) + " bytes" << std::endl;
			size_t pos = HTMLbody.find("<pre>\n");
			pos += string("<pre>\n").size();
			HTMLbody.insert(pos, out.str());
		}
	}
	// Ajout des fichiers
	// Ajout des dossiers
	for (std::map<string, unsigned int>::reverse_iterator it = dirMap.rbegin(); it != dirMap.rend(); it++)
	{
		if (it->second == DT_DIR)
		{
			if (DEBUG_RESPONSE)
				std::cerr << "name:[" << it->first << "] type" << itoa(it->second) << "size:[" << std::endl;
			size_t pos = HTMLbody.find("<pre>\n");
			pos += string("<pre>\n").size();
			HTMLbody.insert(pos, "<a href=\"" + it->first + "\">" + it->first + "</a>\n");
		}
	}
	// Ajout des dossiers
	if (DEBUG_RESPONSE)
	{
		std::cerr << "\e[33m-----------Autoindex BODY-----------" << std::endl;
		std::cerr << HTMLbody << std::endl;
		std::cerr << "-----------Autoindex BODY-----------\e[0m" << std::endl;
	}
	_ss << HTMLbody;
	_bodyLength = HTMLbody.size();
	return (1);
}
