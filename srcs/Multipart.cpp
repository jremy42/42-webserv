# include "Multipart.hpp"


Multipart::Multipart(void)
{}

Multipart::Multipart(string bodyFile, string boundaryDelim, string uploadDir)
{
	_bodyFile = bodyFile;
	_boundaryDelim = boundaryDelim;
	_uploadDir = uploadDir;
	printTimeDebug(1, "create Multipart with tmpfile", _bodyfile);
	_fs.open(_bodyFile.c_str(), std::ofstream::binary | std::ifstream::in);
	if (_fs.good())
		std::cout << "Successfully opened body file "<< std::endl;
	else
		throw(std::runtime_error(std::string("Failed to open tmpfile body in multipart") + strerror(errno)));
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

void Multipart::_extractFileFromBody(void)
{

	string				newBodyFile;
	std::fstream		fsNewFile;


	while (getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == string("--" + _boundary + "--" + "\r"))
			break;		
		bufExtract += "\n";
		std::cout << "inserty[" << bufExtract << std::endl;
		fsNewBodyFile << bufExtract;
	}
	fsNewBodyFile.unget();
	fsNewBodyFile.ignore(1,'\n');
	fsNewBodyFile.flush();
	fsNewBodyFile.close();
	//unlink(_nameBodyFile.c_str());	if (_contentLength <= 0 && _state == R_BODY && _)

	printTimeDebug(1, "boundary header:", "");
	std::cout << _boundaryHeader << std::endl;
}

void Request::_extractHeader(void)
{
	string				bufExtract;
	string				header_key;
	string				header_value;
	std::size_t			colonPos;
	
	while(getline(_fs, bufExtract, '\n'))
	{
		if (bufExtract == (string("--" + _boundaryDelim + "\r")))
		{
			std::cout << "coutinue" << std::endl;
			continue;
		}
		if (bufExtract == "\r")
			break;
		colonPos = bufExtract.find(':');
		if (colonPos == std::string::npos
				|| colonPos == bufExtract.length() - 1
				|| colonPos == 1)
		{
			std::cerr<<"ERRRROOOOOORRRR" << std::endl;
			std::cout << "[" << string("--" + _boundary) << "]" << std::endl;
			std::cout << "[" << bufExtract << "]" << std::endl;
		}
		header_key = string(bufExtract.begin(), bufExtract.begin() + colonPos);
		header_value = string(bufExtract.begin() + colonPos + 1, bufExtract.end());
		header_key = strtrim(header_key, "\f\t\n\r\v ");
		header_value = strtrim(header_value, "\f\t\n\r\v ");
		_boundaryHeader.insert(std::pair<string, string>(header_key, header_value));
		std::cout << "Inserted :" << " new header key-value in Boundary header : [" << header_key << "][" << header_value << "]" << std::endl;
	}
	exit (1);
}


int Multipart::createFilesFromBody(void)
{
	while(_fs.tellg() > 0)
	{
		_extractHeader();
		_createFileFromHeader();
		_extractFileFromBody();
	}

}