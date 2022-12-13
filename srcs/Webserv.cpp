#include "Webserv.hpp"
#include "Request.hpp"

extern int	g_rv;

Webserv::Webserv()
{};


Webserv::Webserv(string fileName)
{
	_openFd = 0;
	_maxFd = _getMaxFd();
	if (DEBUG_WEBSERV)
		std::cerr << "Webserv: max fd available: " << _maxFd << std::endl;
	_loadFile(fileName.c_str());
	if (DEBUG_WEBSERV)
	{
		std::cerr << "Config list :" << std::endl;
		for (v_string::iterator it = _rawConfig.begin(); it != _rawConfig.end(); it++)
			std::cerr << "-----Start Config-----" << std::endl << *it << "------End Config------" << std::endl;
	}
	return ;
}

Webserv::Webserv(const Webserv &src)
{
	*this = src;
}

Webserv::~Webserv(void)
{
	m_i_serv::iterator ite = _fdServerList.end();

	for (m_i_serv::iterator it = _fdServerList.begin(); it != ite; it++)
		delete it->second;
}


Webserv &Webserv::operator=(const Webserv &rhs)
{
	this->_serverList = rhs._serverList;
	this->_rawConfig = rhs._rawConfig;
	this-> _portIpConfigList = rhs._portIpConfigList;
	this->_openFd = rhs._openFd;
	this->_maxFd = rhs._maxFd;

	return (*this);
}

void Webserv::_loadFile(const char * fileName)
{
	std::string		nextLine;
	std::ifstream	fs;

	fs.open(fileName, std::ifstream::in);
	if (!fs.good())
	{
		fs.close();
		throw std::runtime_error(std::string("webserv: Failure opening config file : '" ) + fileName + "' : " + strerror(errno));
	}
	_rawConfig.push_back("");
	while (std::getline(fs, nextLine))
	{
		if (DEBUG_WEBSERV)
		{
			if (nextLine.empty() || nextLine.find_first_not_of("\f\t\n\r\v ") == std::string::npos)
				std::cerr << "[Empty Line]" << std::endl;
			else
				std::cerr << nextLine << std::endl;
		}
		if (!nextLine.empty()
			&& !strtrim(nextLine, "\f\t\n\r\v ").empty() && nextLine[0] != '#')
		{
			nextLine = nextLine.substr(0, nextLine.find("#"));
			_rawConfig.back() += (nextLine + "\n");
		}
	}
	fs.close();
}

std::string::iterator	findMatchingBracket(std::string &str, std::size_t openBracketPos)
{
	std::string::iterator	it = str.begin() + openBracketPos + 1;
	std::string::iterator	ite = str.end();
	int						count = 1;

	for (; it != ite; it++)
	{
		if (*it == '{')
			count++;
		else if (*it == '}')
			count--;
		if (count == 0)
			break;
	}
	return (it);
}

std::string	Webserv::getNextServerBlock(std::string &rawConfig)
{
	std::string							rawServerConf;
	std::string							whiteSpaces("\f\t\n\r\v ");
	std::size_t							startServerWord;
	std::size_t							openBracketServer;
	std::string::iterator				closeBracketServer;

	startServerWord = rawConfig.find("server");
	if (startServerWord == std::string::npos)
		return (rawServerConf);
	openBracketServer = rawConfig.find("{", startServerWord);
	if (openBracketServer == std::string::npos)
		return (rawServerConf);
	std::string tmp = rawConfig.substr(startServerWord + 6, openBracketServer - (startServerWord + 6));
	if (strtrim(tmp, whiteSpaces).size() != 0)
		return (rawServerConf);
	closeBracketServer = findMatchingBracket(rawConfig, openBracketServer);
	if (closeBracketServer == rawConfig.end())
		return (rawServerConf);
	rawServerConf = std::string(rawConfig.begin() + openBracketServer, closeBracketServer + 1);
	rawConfig = std::string(closeBracketServer + 1, rawConfig.end());
		return(rawServerConf);
}


int		Webserv::parseRawConfig(void)
{
	int									viableConfig = 0;
	std::vector<std::string>::iterator	it;
	std::string							rawServerConf;
	std::string 						conflictServerName;

	for (it = _rawConfig.begin(); it != _rawConfig.end(); it++)
	{
		while (!(rawServerConf = getNextServerBlock(*it)).empty())
		{
			try {
				Config nextConfig(rawServerConf);
				for (m_piu_vc::iterator it = _portIpConfigList.begin(); it != _portIpConfigList.end() ; it++)
				{
					for (std::vector<Config>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
					{
						if (nextConfig.getListenPort() == it2->getListenPort()
							&& nextConfig.getHost() == it2->getHost()
							&& (conflictServerName = _checkServerName(nextConfig.getServerName(), it2->getServerName())) != "")
						{

							throw (std::runtime_error("\e[33mConfig Block ignore : Same server name on port :"
								+ nextConfig.getHostStr() + ":" + nextConfig.getListenPortStr()
								+ " with a server_name: " + conflictServerName + "\e[0m"));
						}
					}
				}
				viableConfig |= 1;
				if(_portIpConfigList.find(std::pair<int, int >(nextConfig.getListenPort(), nextConfig.getHost())) != _portIpConfigList.end())
					_portIpConfigList.find(std::pair<int, int >(nextConfig.getListenPort(), nextConfig.getHost()))->second.push_back(nextConfig);
				else
				{
					std::vector<Config> tmp;
					tmp.push_back(nextConfig);
					if (DEBUG_WEBSERV)
					{
						std::cerr << "\e[32mPushing back a new config in the PortConfigList\e[0m" << std::endl;
						std::cerr << "port :[" << nextConfig.getListenPort() << "]" << std::endl;
					}
					_portIpConfigList.insert(std::pair< std::pair<int, int> ,std::vector<Config> >(std::pair<int, int>(nextConfig.getListenPort(), nextConfig.getHost()), tmp));
				}
			}
			catch (const std::exception &e) 
			{
				 std::cerr  << "\e[31m" << e.what() << "\e[0m" << std::endl;
				 throw(std::runtime_error("Webserv: Config creation failure"));
			}
		}
		if (DEBUG_WEBSERV)
			std::cerr << "No more server block. Going to next conf file" << std::endl;
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();
	return (1);
}

void	Webserv::_moveHostConfigToWildcard(int depth)
{
	m_piu_vc::iterator it;
	int 				searchPort;
	for (it = _portIpConfigList.begin(); it != _portIpConfigList.end(); it++)
	{
		if (it->first.second == 0)
		{
			if (DEBUG_WEBSERV)
			{
				std::cerr << "\e[35mFound Wildcard for port : [" << it->first.first << "]\e[0m" << std::endl;
				std::cerr << "Before WildCard Clean" << std::endl << it->second << std::endl;
			}
			searchPort = it->first.first;
			m_piu_vc::iterator it2;
			for (it2 = _portIpConfigList.begin(); it2 != _portIpConfigList.end(); it2++)
			{
				if (it2->first.first == searchPort && it != it2)
				{
					if (DEBUG_WEBSERV)
						std::cerr << "\e[36mFound a Host/Port pair matching Wildcard for port : Port[" << it2->first.first << "] Host : [" << it2->first.second << "]\e[0m" << std::endl;
					v_config tmpVconfig = it2->second;
					for (v_config::iterator confIt = tmpVconfig.begin(); confIt != tmpVconfig.end(); confIt++)
						it->second.push_back(*confIt);
					_portIpConfigList.erase(it2);
					return _moveHostConfigToWildcard(depth + 1);
				}
			}
		}
		if (DEBUG_WEBSERV)
			std::cerr << "After WildCard Clean" << std::endl << it->second << std::endl;
	}
}

int		Webserv::createServerListByPortConfig(void)
{
	m_piu_vc::iterator it;

	_moveHostConfigToWildcard(0);
	if(_portIpConfigList.size() > _maxFd - 3)
		throw std::runtime_error("Webserv: No server created. not enough file descriptor available");
	for (it = _portIpConfigList.begin(); it != _portIpConfigList.end() && _openFd < _maxFd ; it++)
	{
		try {
			Server *newServer = new Server((*it).second);
			_serverList.push_back(newServer);
			_evListener.trackNewFd(newServer->getServerFd(), EPOLLIN);
			_fdServerList.insert(std::pair<int, Server*>(newServer->getServerFd(), newServer));
			_openFd++;
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
	if(_serverList.size() == 0)
		throw std::runtime_error("Webserv: No server created. check your config file or quit webserv");
	return (1);
}

int		Webserv::execServerLoop(void)
{
	std::map<int, int> _fdAvailable;
	std::map<int, int>::iterator ite;
	printLog(1,0,1,"Webserv is running");
	while (g_rv)
	{
		_fdAvailable = _evListener.fdAvailable();
		ite = _fdAvailable.end();
		for (std::map<int, int>::iterator it = _fdAvailable.begin(); it != ite; it++)
		{

			if (_fdServerList.find(it->first) != _fdServerList.end() && _openFd < _maxFd / 2)
			{
				try {
					if (DEBUG_SERVER)
						std::cerr << "\e[32mACCEPT NEW CLIENT\e[0m\n";
					if (DEBUG_SERVER)
						std::cerr << "Number of openfd : [" <<_openFd << "]" << std::endl;
					int newFd = _fdServerList.find(it->first)->second->acceptNewClient();
					_evListener.trackNewFd(newFd, EPOLLIN | EPOLLOUT);
					_openFd++;
					_fdClientList.insert(std::pair<int, Server*>(newFd, _fdServerList.find(it->first)->second));
				}
				catch (const std::exception &e)
				{
					std::cerr << e.what() << std::endl;
				}
			}
			else if (_fdClientList.find(it->first) != _fdClientList.end())
			{
				if (!(_fdClientList.find(it->first)->second->execClientAction(it->first, it->second)))
				{
					_fdClientList.erase(it->first);
					_openFd--;
				}
			}
		}
	}
	printLog(1,0,1,"Closing Webserv");
	return (1);
}


unsigned long	Webserv::_getMaxFd()
{
	struct rlimit rlim;
	getrlimit(RLIMIT_NOFILE, &rlim);
	return(rlim.rlim_cur);
}

std::string Webserv::_checkServerName(std::vector<string> nextServerName, std::vector<string> currentServerName)
{
	std::vector<string>::iterator ite = nextServerName.end();
	for (std::vector<string>::iterator it = nextServerName.begin(); it != ite; it++)
	{
		if (DEBUG_WEBSERV)
			std::cerr << "Check for server name :" << *it << std::endl;
		if (find(currentServerName.begin(), currentServerName.end(), *it) != currentServerName.end())
			return (*it);
	}
	return "";
}

const char	*Webserv::NotEnoughValidConfigFilesException::what(void) const throw ()
{
	return ("Webserv : Need at least one valid configuration file to launch Webserv");
}
