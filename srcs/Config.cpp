#include "Config.hpp"
#include "iostream"

Config::Config(){};
Config::Config(string listenPort) : _listenPort(listenPort)
{
	std::cout << "create config with port :" << _listenPort << std::endl;
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
	return (*this);
}

const char*		Config::getListenPort(void) const
{
	return (_listenPort.c_str());
}
