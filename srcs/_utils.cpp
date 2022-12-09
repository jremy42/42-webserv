#include "_utils.hpp"

extern int	g_rv;

std::string	&strtrim(std::string &str, const std::string &charset)
{
	std::size_t first 					= str.find_first_not_of(charset);
	std::size_t last 					= str.find_last_not_of(charset);
	std::string::iterator begin			= str.begin();
	std::string::iterator end			= str.end();

	if (first == std::string::npos)
	{
		str = "";
		return (str);
	}
	begin += first;
	end -= str.length() - last - 1;
	str = std::string(begin, end);
	return (str);
}

std::string	normalizeKeyValStr(std::string &keyValStr, const std::string &separatorCharset, const char defaultSeparator)
{
	std::size_t	nextSeparatorToReplace;
	std::size_t	replaceOffset;

	replaceOffset = 0;
	strtrim(keyValStr, separatorCharset);
	while (replaceOffset < keyValStr.length()
			&& (nextSeparatorToReplace = keyValStr.find_first_of(separatorCharset, replaceOffset)) != std::string::npos)
	{
		keyValStr.at(nextSeparatorToReplace) = defaultSeparator;
		replaceOffset = nextSeparatorToReplace + 1;
		while (replaceOffset < keyValStr.length()
				&& separatorCharset.find_first_of(keyValStr.at(replaceOffset)) != std::string::npos)
			keyValStr.erase(replaceOffset, 1);
	}
	return (keyValStr);
}



std::ostream	&operator<<(std::ostream &o, const std::vector<std::string> &vec)
{
	for (unsigned long i = 0; i < vec.size(); i++)
	{
		o << (i != 0 ? ", " : "");
		o << "[" << i << "][" << vec[i] << "]";
	}
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair)
{
	o << "[" << pair.first << "] : ";
	o << pair.second;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map)
{
	std::map<std::string, std::vector<std::string> >::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		o << *it << std::endl;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<int, std::string> &map)
{
	std::map<int, std::string>::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		o << "key [" << it->first << "] value:[" << it->second << "]" << std::endl;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::string> &map)
{
	std::map<std::string, std::string>::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		o << "key [" << it->first << "] value:[" << it->second << "]" << std::endl;
	return (o);
}

std::string itoa(int statusCode)
{
	std::stringstream out;
	out << statusCode;
	return out.str();
}


std::string ltoa(long statusCode)
{
	std::stringstream out;
	out << statusCode;
	return out.str();
}


long getFileSize(std::string filename)
{
	//std::cout << "getFileSize FILENAME:" << filename << std::endl; 
    FILE *fp = fopen(filename.c_str(), "r");

    if (fp == NULL)
        return (0);
    if (fseek(fp, 0, SEEK_END) < 0)
	{
        fclose(fp);
        return (0);
    }
    long size = ftell(fp);
    fclose(fp);
    return size;
}

std::string getFileSizeStr(std::string filename)
{
	//std::cout << "getFileSize FILENAME:" << filename << std::endl; 
    FILE *fp = fopen(filename.c_str(), "r");

    if (fp == NULL)
        return ("ERROR FOPEN FILE DURING getFileSize function");
    if (fseek(fp, 0, SEEK_END) < 0)
	{
        fclose(fp);
        return ("ERROR FSEEK DURING getFileSize function");
    }
    long size = ftell(fp);
    fclose(fp);
    return ltoa(size);
}

int isDir(std::string fileName)
{
  	struct stat buf;
    stat(fileName.c_str(), &buf);
	return (!S_ISREG(buf.st_mode));
}

int fileExist(std::string fileName)
{
  	struct stat buf;
    return (!stat(fileName.c_str(), &buf));
}

void	__signal(int signal)
{
	if (signal == 2)
	{	
		g_rv = 0;
	}
}

unsigned long	ft_get_time(void)
{
	struct timeval	now;

	gettimeofday(&now, NULL);
	//std::cout << "TIME [" << now.tv_sec * 1000 + now.tv_usec / 1000 <<  "]" << std::endl;
	return (now.tv_sec * 1000 + now.tv_usec / 1000);
}

unsigned long	ft_get_time_sec(void)
{
	static struct timeval	begin;
	static int				init = 0;
	struct timeval	now;

	if (init == 0)
	{
		gettimeofday(&begin, NULL);
		init = 1;
	}
	gettimeofday(&now, NULL);
	return ((now.tv_sec - begin.tv_sec) * 1);
}

unsigned int			getipbyhost(const char *host, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *result;
	int	g_error;
	in_addr_t ret;

	//std::cout << "get ip by host [" << host << "][" << service << "]" << std::endl;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((g_error = getaddrinfo(host, service, &hints, &result )) != 0)
		throw(std::runtime_error(gai_strerror(g_error)));
	ret = ((struct sockaddr_in*)(result->ai_addr))->sin_addr.s_addr;
	freeaddrinfo(result);
	//std::cout << "ret : [" << ret << "]" << std::endl;
	return ret;
}


void printTimeDebug(int debug, std::string quote, std::string arg)
{
	if (debug)
	{
		if (arg.size() > 0)
			std::cout << "[\e[32m" << ft_get_time_sec() << "\e[0m]] :" << quote << ": [" << arg << "]" << std::endl;
		else
			std::cout << "[\e[32m" << ft_get_time_sec() << "\e[0m]] :" << quote << std::endl;
	}
}

void printAvailableAction(int debug, int _clientFd, int _availableActions)
{
	if (debug)
	{
		printf(" Client_fd:[%d], events [%s][%s][%s][%s][%s]\n", _clientFd,
				(_availableActions & EPOLLIN) ? "EPOLLIN " : "",
				(_availableActions & EPOLLOUT) ? "EPOLLOUT " : "",
				(_availableActions & EPOLLERR) ? "EPOLLERR " : "",
				(_availableActions & EPOLLRDHUP) ? "EPOLLRDHUP " : "",
				(_availableActions & EPOLLHUP) ? "EPOLLHUP " : "");
	}
}


std::string tmpFileName(const std::string path)
{
	std::string allowedChar = "abcdefghijklmnoprstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string fileName(path);

	for (int y = 0; y < MAX_TRY; y++)
	{

		for (int i = 0; i < MAX_FILENAME; i++)
			fileName += allowedChar[rand() % allowedChar.size()];
		if (access(fileName.c_str(), F_OK) == -1)
			return fileName;
		else
		{
			fileName.clear();
			fileName += path;
		}
	}
	return ("");
}

std::string		getClientAddrFromSocket(int acceptSysCallReturnedFd)
{
	struct sockaddr_in	clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	char buf[INET_ADDRSTRLEN + 1];
	std::string result;

	if (getpeername(acceptSysCallReturnedFd, (struct sockaddr *)&clientAddr, &addrlen))
		return ("getClientAddrFromSocket ERROR");
	memset(buf, 0, sizeof(buf));
	inet_ntop(AF_INET, &clientAddr.sin_addr, buf, sizeof(buf));

	result = std::string(buf);
	return (result);
}

std::string		getClientPortFromSocket(int acceptSysCallReturnedFd)
{
	struct sockaddr_in	clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	std::string result;

	memset(&clientAddr, 0, addrlen);
	if (getpeername(acceptSysCallReturnedFd, (struct sockaddr *)&clientAddr, &addrlen))
		return ("getClientPortFromSocket ERROR");

	result = std::string(itoa(ntohs(clientAddr.sin_port)));
	return (result);
}

std::string		getRequestedAddrFromSocket(int acceptSysCallReturnedFd)
{
	struct sockaddr_in	requestedAddr;
	socklen_t addrlen = sizeof(requestedAddr);
	char buf[INET_ADDRSTRLEN + 1];
	std::string result;

	if (getsockname(acceptSysCallReturnedFd, (struct sockaddr *)&requestedAddr, &addrlen))
		return ("getRequestedAddrFromSocket ERROR");
	memset(buf, 0, sizeof(buf));
	inet_ntop(AF_INET, &requestedAddr.sin_addr, buf, sizeof(buf));

	result = std::string(buf);
	return (result);
}

std::string		getRequestedPortFromSocket(int acceptSysCallReturnedFd)
{
	struct sockaddr_in	requestedAddr;
	socklen_t addrlen = sizeof(requestedAddr);
	std::string result;

	memset(&requestedAddr, 0, addrlen);
	if (getsockname(acceptSysCallReturnedFd, (struct sockaddr *)&requestedAddr, &addrlen))
		return ("getRequestedPortFromSocket ERROR");

	result = std::string(itoa(ntohs(requestedAddr.sin_port)));
	return (result);
}

std::pair<std::string, std::string>		getClientHostnameAndService(int acceptSysCallReturnedFd)
{
	struct sockaddr_in	clientAddr;
	socklen_t addrlen = sizeof(clientAddr);
	char host[NI_MAXHOST];
	std::size_t hostlen = sizeof(host);
	char service[NI_MAXSERV];
	std::size_t servicelen = sizeof(host);
	std::pair<std::string, std::string>  result;

	if (getpeername(acceptSysCallReturnedFd, (struct sockaddr *)&clientAddr, &addrlen))
		return (std::make_pair("getClientHostNameFromSocket : getpeername ERROR", ""));
	memset(host, 0, sizeof(host));
	memset(host, 0, sizeof(service));
	if (getnameinfo((struct sockaddr *)&clientAddr, addrlen, host, hostlen, service, servicelen, 0))
		return (std::make_pair("getClientHostNameFromSocket : getnameinfo ERROR", ""));
	result = std::pair<std::string, std::string>(host, service);
	return (result);
}

std::string								subStringBeforeFirstDelim(std::string &str, char delim)
{
	std::size_t	pos;

	pos = str.find_first_of(delim);
	return (str.substr(0, pos));
}

std::string								subStringAfterFirstDelim(std::string &str, char delim)
{
	std::size_t	pos;

	pos = str.find_first_of(delim);
	return (str.substr(pos + 1));
}

std::string								subStringBeforeLastDelim(std::string &str, char delim)
{
	std::size_t	pos;

	pos = str.find_last_of(delim);
	return (str.substr(0, pos));
}

std::string								subStringAfterLastDelim(std::string &str, char delim)
{
	std::size_t	pos;

	pos = str.find_last_of(delim);
	return (str.substr(pos + 1));
}

void printLog(int log, int fd, int nb, ...)
{
    va_list args;
    va_start(args, nb);

	if (log)
	{
		std::cout << "[\e[32m" << ft_get_time_sec() << "\e[0m]";
		if (fd)
			std::cout << "client : " << getClientHostnameAndService(fd).first << "| host: " << getRequestedAddrFromSocket(fd) << " | " ;
		while (nb)
		{
			char * toPrintc = va_arg(args, char *);
			std::cout << toPrintc << " ";
			nb--;
		}
	}
	std::cout << std::endl;
    va_end(args);
}

void printLogServer(int log, int fd)
{
	if (log)
	{
		std::cout << "[\e[32m" << ft_get_time_sec() << "\e[0m]";
		std::cout << "server is created with addr [" <<  getRequestedAddrFromSocket(fd) << ":" << getRequestedPortFromSocket(fd) <<"]";
	}
	std::cout << std::endl;
}
