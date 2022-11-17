#include "Webserv.hpp"
#include "Request.hpp"
#include <iterator>
#include <algorithm>

extern int	g_rv;

Webserv::Webserv()
{};

// Webserv::Webserv(char **av) : _configArray(av)
// {
// 	std::string		nextLine;
// 	std::ifstream	fs;
// 	int				viableConfig = 0;
// 	int				i = 0;

// 	++_configArray;
// 	while (_configArray[i])
// 	{
// 		_loadFile(_configArray[i]);
// 		i++;
// 	}
// 	std::cout << "Config list :" << std::endl;
// 	for (v_string::iterator it = _rawConfig.begin(); it != _rawConfig.end(); it++)
// 	{
// 		std::cout << "-----Start Config-----" << std::endl << *it << "------End Config------" << std::endl;
// 		viableConfig |= (*it != "");
// 	}
// 	if (viableConfig == 0)
// 		throw NotEnoughValidConfigFilesException();
// 	return ;
// }

Webserv::Webserv(string fileName)
{
	//_configArray = NULL;

	int				viableConfig = 0;
	_openFd = 0;
	_maxFd = _getMaxFd();
	if (_maxFd < 10)
		throw(std::runtime_error("Webserv: not open enough file descriptor for run webserv" ));
	std::cout << "fd max " << _maxFd << std::endl;
	_loadFile(fileName.c_str());
	std::cout << "Config list :" << std::endl;
	for (v_string::iterator it = _rawConfig.begin(); it != _rawConfig.end(); it++)
	{
		std::cout << "-----Start Config-----" << std::endl << *it << "------End Config------" << std::endl;
		viableConfig |= (*it != "");
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();

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
	this-> _configList = rhs._configList;
	this-> _portConfigList = rhs._portConfigList;
	this->_openFd = rhs._openFd;
	this->_maxFd = rhs._maxFd;

	return (*this);
}

void Webserv::_loadFile(const char * fileName)
{
	std::string		nextLine;
	std::ifstream	fs;

	fs.open(fileName, std::ifstream::in);
	if (fs.good())
		std::cout << "Successfully opened config file : '" << fileName << "'" << std::endl;
	else
	{
		std::cerr << "Failure opening config file : '" << fileName << "' : " << strerror(errno) << std::endl;
		fs.close();
	}
	_rawConfig.push_back("");
	while (std::getline(fs, nextLine))
	{
		if (DEBUG_WEBSERV)
		{
			if (nextLine.empty() || nextLine.find_first_not_of("\f\t\n\r\v ") == std::string::npos)
				std::cout << "[Empty Line]" << std::endl;
			else
				std::cout << nextLine << std::endl;
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
		//rawServerConf = getNextServerBlock(*it);
		while (!(rawServerConf = getNextServerBlock(*it)).empty())
		//while (!rawServerConf.empty())
		{
			// create Config constructor with rawServConf 
			// try catch error;
			// fonction wich verify if port or server name are the same.
			try {
				Config nextConfig(rawServerConf);
				for (v_config::iterator it = _configList.begin(); it != _configList.end() ; it++)
				{
					if (nextConfig.getListenPort() == it->getListenPort()
						&& (conflictServerName = _checkServerName(nextConfig.getServerName(), it->getServerName())) != "")
					{

						throw (std::runtime_error("\e[33mConfig Block ignore : Same server name on port :"
						+ nextConfig.getListenPortStr() + " with a server_name: " + conflictServerName + "\e[0m"));
					}
				}
				//Constructeur de config a faire avec la map au lieu des 2 premiers fields!!!!
				viableConfig |= 1;
				//std::cout << "\e[32mPushing back a new config in the ConfigList\e[0m" << std::endl;
				if(_portConfigList.find(nextConfig.getListenPort()) != _portConfigList.end())
					_portConfigList.find(nextConfig.getListenPort())->second.push_back(nextConfig);
				else
				{
					std::vector<Config> tmp;
					tmp.push_back(nextConfig);
					std::cout << "\e[32mPushing back a new config in the PortConfigList\e[0m" << std::endl;
					std::cout << "port :[" << nextConfig.getListenPort() << "]" << std::endl;
					_portConfigList.insert(std::pair<int, std::vector<Config> >(nextConfig.getListenPort(), tmp));
				}
				//_configList.push_back(nextConfig);// a suppr
			}
			catch (const std::exception &e) 
			{
				 std::cerr  << "\e[31m" << e.what() << "\e[0m" << std::endl;
				 throw(std::runtime_error("Webserv: Config creation failure RTFM!!!!!😡 😡 😡 😡 😡 😡"));
			}
		}
		if (DEBUG_WEBSERV)
			std::cout << "No more server block. Going to next conf file" << std::endl;
	}
	if (viableConfig == 0)
		throw NotEnoughValidConfigFilesException();
	return (1);
}

int		Webserv::createServerListByPortConfig(void)
{
	m_i_vc::iterator it;

	for (it = _portConfigList.begin(); it != _portConfigList.end() && _openFd < _maxFd ; it++)
	{
		Server *newServer = new Server((*it).second);
		_serverList.push_back(newServer);
		_evListener.trackNewFd(newServer->getServerFd(), EPOLLIN);
		_fdServerList.insert(std::pair<int, Server*>(newServer->getServerFd(), newServer));
		_openFd++;
	}
	return (1);
}

int		Webserv::execServerLoop(void)
{
	std::map<int, int> _fdAvailable;
	std::map<int, int>::iterator ite;

	while (g_rv)
	{
		_fdAvailable = _evListener.fdAvailable();
		ite = _fdAvailable.end();
		for (std::map<int, int>::iterator it = _fdAvailable.begin(); it != ite; it++)
		{
			if (_fdServerList.find(it->first) != _fdServerList.end() && _openFd < _maxFd - 10)
			{
				std::cout << "\e[32mACCEPT NEW CLIENT\e[0m\n";
				// verification du flag
				int newFd = _fdServerList.find(it->first)->second->acceptNewClient();
				_evListener.trackNewFd(newFd, EPOLLIN | EPOLLOUT);
				_openFd++;
				_fdClientList.insert(std::pair<int, Server*>(newFd, _fdServerList.find(it->first)->second));
			}
			else if (_fdClientList.find(it->first) != _fdClientList.end())
			{
				_fdClientList.find(it->first)->second->execClientAction(it->first, it->second);
			}
		}
	}

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
			std::cout << "Check for server name :" << *it << std::endl;
		if (find(currentServerName.begin(), currentServerName.end(), *it) != currentServerName.end())
			return (*it);
	}
	return "";
}

const char	*Webserv::NotEnoughValidConfigFilesException::what(void) const throw ()
{
	return ("Need at least one valid configuration file to launch Webserv");
}
