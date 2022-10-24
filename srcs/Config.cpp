#include "Config.cpp"
#include "iostream"

Config::Config(int listenPort) : _listenPort(listenPort)
{

}

Config::Config(const Config &src)
{
	*this = src;
}

Config::~Config(int listenPort) : _listenPort(listenPort)
{

}

Config	&Config::operator=(const Config &rhs) const
{
	this->_listenPort = rhs._listenPort;
	return (*this);
}

int		Config::getListenPort(void) const
{
	return (this->_listenPort);
}
