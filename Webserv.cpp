#include "Webserv.hpp"
#include "iostream"

Webserv::Webserv(string config_filename)
{
	std::string		nextLine;
	std::ifstream	fs;

	std::cout	<< "Opening and reading config: "
				<< config_filename
				<< std::endl;

	fs.open(filename.c_str(), std::ifstream::in);
	if (fs.good())
		std::cout << "Successfully opened config file : '" << filename << "'" << std::endl;
	else
	{
		std::cerr << "Failure opening config file : '" << filename << "' : " << strerror(errno) << std::endl;
		return ;
	}
	while (std::getline(fs, nextLine))
	{
		if (DEBUG)
		{
			if (nextLine.length() == 0)
				std::cout << "[Empty Line]" << std::endl;
			else
				std::cout << nextLine << std::endl;
		}
		this->_rawConfig += nextLine;
	}
	if (DEBUG)
		std::cout << "rawConfig : [" << this->_rawConfig "]" << std::endl;
	fs.close();
	return ;
}

Webserv::Webserv(const Webserv &src)
{
	*this = src;
}

Webserv::~Webserv(void)
{

}

Webserv &Webserv::operator=(const Webserv &rhs)
{
	this->_serverList = rhs._serverList;
	this->_rawConfig = rhs._rawConfig;
	return (*this);
}

int		Webserv::createServerListFromRawConfig(void)
{
	return (1);

}


int		Webserv::execServerLoop(void)
{
	return (1);
}
