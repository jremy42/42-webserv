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
std::string Response::_autoIndexBodyTemplate = "<html><head><title>Index of /title_placeholder</title></head>\n<body>\n<h1>Index of /title_placeholder</h1><hr><pre>\n</pre><hr>\n</body></html>";

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
	//_createAutoIndex();
	//exit(0);
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

std::string	Response::_selectActualTarget(string &actualTarget)
{
	std::string		requestedTarget	= _request->getTarget();
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
	if (fileExist(actualTarget) && isDir(actualTarget))
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
			//if (actualTarget[actualTarget.size() - 1] != '/')
				//actualTarget += "/";
			return ("Do_listing");
		}
		else
		{
			if (DEBUG_RESPONSE)
				std::cout << "No suitable index file and autoindex is off" << std::endl;
			return ("Index_file_nok");
		}
	}
	else if (fileExist(actualTarget))
	{
		if (DEBUG_RESPONSE)
			std::cout << "Requested file is standard" << std::endl;
		return ("File_ok");
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
	std::cout << "length: [" << length << "]\n";
	buff = new char[length];
	fs.read(buff, length);
	_body = v_c(buff, buff + length);
	delete buff;
	fs.close();
}

void Response::_methodGET(void)
{
	std::string	actualTarget;
	std::string	selectActualTargetResult;
	
	selectActualTargetResult = _selectActualTarget(actualTarget);
	if (selectActualTargetResult == "Do_listing")
		_createAutoIndex(actualTarget);
	else if (selectActualTargetResult != "Index_file_nok" && selectActualTargetResult != "File_nok")
		_createBodyFromFile(actualTarget);
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
	if(_statusCode > 200)
		_createErrorMessageBody();
	else if (_request->getMethod() == "GET")
	{
		_methodGET();
	}
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
// 	char buff[WRITE_BUFFER_SIZE];Successfully opened body file 

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

int Response::_createAutoIndex(const string &pathToDir)
{
	const char *path = pathToDir.c_str();
	struct dirent	*curr_dir;
	DIR				*dp;
	std::map<string, unsigned int> dir;
	std::vector<string> insertLigne;
	string				finalBody;

	finalBody = _autoIndexBodyTemplate;
	for (int i = 0; i < 2; i++)
	{
		size_t pos = finalBody.find("/title_placeholder");
		finalBody.erase(pos, strlen("/title_placeholder"));
		finalBody.insert(pos, path);
	}
	dp = opendir(path);
	if (!dp)
		return (1);
	curr_dir = readdir(dp);
	while(curr_dir)
	{
		string toInsert = curr_dir->d_name;
		if (toInsert[0] != '.' || toInsert == "..")
		{
			if (curr_dir->d_type == DT_DIR)
				toInsert += "/";
			dir.insert(std::pair<string, unsigned int> (toInsert, curr_dir->d_type));
		}
		curr_dir = readdir(dp);
	}
	for (std::map<string, unsigned int>::reverse_iterator it = dir.rbegin(); it != dir.rend(); it++)
	{
		if (it->second != DT_DIR)
		{
			std::stringstream out;
			struct stat buf;
     		stat(path, &buf);
			std::cout << "name:[" << it->first << "] type" << itoa(it->second) << "size:[" << getFileSize(it->first) << std::endl;
			out << std::left << std::setw(80 + string("<a href=\"" + it->first + "\">" + "\">").size()) << "<a href=\""  + it->first + "\">" + it->first + "</a>" << std::setw(40) <<  getFileSize(it->first) + " bytes" << std::endl;
			size_t pos = finalBody.find("<pre>\n");
			pos += string("<pre>\n").size();
			finalBody.insert(pos, out.str());
		}
	}
	for (std::map<string, unsigned int>::reverse_iterator it = dir.rbegin(); it != dir.rend(); it++)
	{
		if (it->second == DT_DIR)
		{
			
			std::cout << "name:[" << it->first << "] type" << itoa(it->second) << "size:[" << std::endl;
			size_t pos = finalBody.find("<pre>\n");
			pos += string("<pre>\n").size();
			finalBody.insert(pos, "<a href=\"" + it->first + "\">" + it->first + "</a>\n");
		}
	}

	std::cout << finalBody << std::endl;
	_body = v_c(finalBody.begin(), finalBody.end());
	if (closedir(dp) < 0)
		return (0);
	return (1);
}

//void Response::_createBody(void)
//{
//	//Selecting file
//	std::string		requestedTarget	= _request->getTarget();
//	std::string		requestedTargetLocation	= _config->getParamByLocation(requestedTarget, "root").at(0);
//	std::string		actualTarget = requestedTargetLocation + requestedTarget;	
//	//Selecting file
//	//creating body
//	std::ifstream	fs;
//	char			*buff;
//	int				length;
//	//creating body
//
//	if (DEBUG_RESPONSE)
//	{
//		std::cout << "requested Target : [" << requestedTarget << "]" << std::endl;
//		std::cout << "requested Target Location (according to root directives) : [" << requestedTargetLocation << "]" << std::endl;
//		std::cout << "actual Target : [" << actualTarget << "]" << std::endl;
//		if (!fileExist(actualTarget))
//			std::cout << "actual Target does not exists" << std::endl;
//		if (fileExist(actualTarget) && isDir(actualTarget))
//			std::cout << "actual Target exist and is a Directory" << std::endl;
//		else if (fileExist(actualTarget) && !isDir(actualTarget))
//			std::cout << "actual Target exist and is a regular file" << std::endl;
//	}
//	if (fileExist(actualTarget) && isDir(actualTarget))
//	{
//		std::vector<string> indexTryFiles = _config->getParamByLocation(requestedTarget, "index");
//		if (DEBUG_RESPONSE)
//			std::cout << "Trying files in indexTryFiles :" << indexTryFiles << std::endl;
//		std::vector<string>::iterator it = indexTryFiles.begin();
//		for (;it != indexTryFiles.end(); it++)
//		{
//			std::string	testedIndexFile = actualTarget + "/" + *it;
//			if (DEBUG_RESPONSE)
//				std::cout << "Testing index file :" << testedIndexFile << std::endl;
//			if (fileExist(testedIndexFile) && !isDir(testedIndexFile))
//			{
//				actualTarget = testedIndexFile;
//				break;
//			}
//		}
//		if (it != indexTryFiles.end())
//		{
//			if (DEBUG_RESPONSE)
//				std::cout << "Found a suitable index file : [" << *it << "]" << std::endl;
//		}
//		else if (_config->getParamByLocation(requestedTarget, "autoindex").at(0) == "on")
//		{
//			if (DEBUG_RESPONSE)
//				std::cout << "No suitable index file but autoindex is on. Returning Listing of directory" << std::endl;
//			_createAutoIndex(actualTarget);
//			return ;
//		}
//		else
//		{
//			if (DEBUG_RESPONSE)
//				std::cout << "No suitable index file and autoindex is off" << std::endl;
//			_statusCode = 404;
//			return ;
//		}
//	}
//	fs.open(actualTarget.c_str(), std::ifstream::in | std::ifstream::binary);
//	if (fs.good())
//		std::cout << "Successfully opened body file "<< std::endl;
//	else
//	{
//		std::cerr << "Failure opening body file '" << strerror(errno) << std::endl;
//		_statusCode = 404;
//		fs.close();
//		return;
//	}
//	fs.seekg(0, fs.end);
//	length = fs.tellg();
//	fs.seekg(0, fs.beg);
//	std::cout << "length: [" << length << "]\n";
//	buff = new char[length];
//	fs.read(buff, length);
//	_body = v_c(buff, buff + length);
//	delete buff;
//	fs.close();
//}
