# include "Multipart.hpp"


Multipart::Multipart(void)
{}

Multipart::Multipart(string bodyFile, string boundaryDelim, string uploadDir)
{
	_bodyFile = bodyFile;
	_globalError = false;
	_currentFileError = false;
	_boundaryDelim = boundaryDelim;
	_uploadDir = uploadDir;
	printTimeDebug(DEBUG_MULTIPART, "create Multipart with tmpfile", _bodyFile);
	_fs.open(_bodyFile.c_str(), std::ofstream::binary | std::ifstream::in);
	if (!_fs.good())
		throw(std::runtime_error(string("Multipart: cannot open body file") + strerror(errno)));
}

Multipart::Multipart(const Multipart &src)
{
	*this = src;
}

Multipart::~Multipart(void)
{
		_fs.close();
}

Multipart	&Multipart::operator=(const Multipart &rhs)
{
	_bodyFile = rhs._bodyFile;
	return (*this);
}

int Multipart::_extractFileFromBody(void)
{

	string				newFile;
	string				bufExtract;


	while (getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == string("--" + _boundaryDelim + "\r"))
			break;
		if (bufExtract == string("--" + _boundaryDelim + "--" + "\r"))
		{
			if(!_currentFileError)
				_returnMessage += _fileName + ": File successfully uploaded\n";
			else
				_returnMessage += _fileName + ": Failed \n";
			_fsNewFile.flush();
			_fsNewFile.close();
			return (0);
		}
		if (bufExtract == string("\r"))
			continue;
		bufExtract += "\n";
		_fsNewFile << bufExtract;
	}
	if(!_currentFileError)
		_returnMessage += _fileName + ": File successfully uploaded\n";
	else
		_returnMessage += _fileName + ": Failed \n";
	_currentFileError = false;
	_fsNewFile.flush();
	_fsNewFile.close();
	return (1);	
} 

int Multipart::_extractHeader(void)
{
	string				bufExtract;
	string				header_key;
	string				header_value;
	std::size_t			colonPos;
	
	while(getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == (string("--" + _boundaryDelim + "\r")))
		{
			if (DEBUG_MULTIPART)
				std::cerr << "continue" << std::endl;
			continue;
		}
		if (bufExtract == "\r")
			break;
		colonPos = bufExtract.find(':');
		if (DEBUG_MULTIPART &&(colonPos == std::string::npos
				|| colonPos == bufExtract.length() - 1
				|| colonPos == 1))
		{
			std::cerr << "[" << string("--" + _boundaryDelim) << "]" << std::endl;
			std::cerr << "[" << bufExtract << "]" << std::endl;
		}
		header_key = string(bufExtract.begin(), bufExtract.begin() + colonPos);
		header_value = string(bufExtract.begin() + colonPos + 1, bufExtract.end());
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		_boundaryHeader.insert(std::pair<string, string>(header_key, header_value));
		if (DEBUG_MULTIPART)
			std::cerr << "Inserted :" << " new header key-value in Boundary header : [" << header_key << "][" << header_value << "]" << std::endl;
	}
	if (_boundaryHeader.empty())
		return 0;
	return 1;
}

int 	Multipart::_createFileFromHeader(void)
{
	string rawContentType = _boundaryHeader["Content-Disposition"];
	_fileName = rawContentType.find("filename=") != std::string::npos ? rawContentType.substr(rawContentType.find("filename=") + 9) : "";
	_fileName = strtrim(_fileName, "\"");
	if (_fileName.empty())
		return 0;
	_fileName = _uploadDir + "/" + _fileName;
	if (DEBUG_MULTIPART)
		std::cerr << "Creating new file : " << _fileName << std::endl;
	_fsNewFile.open(_fileName.c_str(), std::ofstream::binary | std::ifstream::out | std::ifstream::trunc);
	if (!_fsNewFile.good())
	{
		_currentFileError = true;
		_globalError = true;
		_fsNewFile.open("/dev/null", std::ofstream::binary | std::ifstream::out);
	}

	return 1;
}

int Multipart::createFilesFromBody(void)
{
	int continueLoop = 1;
	
		while (continueLoop)
		{
			if (!_extractHeader())
				break;
			if (_createFileFromHeader())
				continueLoop = _extractFileFromBody();
			_boundaryHeader.clear();
		}
	return 1;
}


std::string Multipart::getReturnMessage(void)
{
	return _returnMessage;
}

bool Multipart::getError(void)
{
	return _globalError;
}