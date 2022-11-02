#include "Config.hpp"
#include "iostream"

Config::Config(void)
{

};

Config::Config(string listenPort, string rootDirectory) : _listenPort(listenPort), _rootDirectory(rootDirectory)
{
	std::cout << "create config with port :" << _listenPort << " and root dir :" << _rootDirectory << std::endl;
}

Config::Config(const Config &src)
{
	*this = src;
}

Config::~Config(void)
{

}

Config	&Config::operator=(const Config &rhs)
{
	this->_listenPort = rhs._listenPort;
	this->_rootDirectory = rhs._rootDirectory;

	return (*this);
}

const char*		Config::getListenPort(void) const
{
	return (_listenPort.c_str());
}

const char* Config::getRootDir(void) const
{
	return (_rootDirectory.c_str());
}