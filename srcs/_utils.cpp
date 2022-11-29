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
		std::cout << (i != 0 ? ", " : "");
		std::cout << "[" << i << "][" << vec[i] << "]";
	}
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair)
{
	std::cout << "[" << pair.first << "] : ";
	std::cout << pair.second;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map)
{
	std::map<std::string, std::vector<std::string> >::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		std::cout << *it << std::endl;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<int, std::string> &map)
{
	std::map<int, std::string>::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		std::cout << "key [" << it->first << "] value:[" << it->second << "]" << std::endl;
	return (o);
}

std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::string> &map)
{
	std::map<std::string, std::string>::const_iterator	it = map.begin();
	for (; it != map.end(); it++)
		std::cout << "key [" << it->first << "] value:[" << it->second << "]" << std::endl;
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
	std::cout << "getFileSize FILENAME:" << filename << std::endl; 
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
	std::cout << "getFileSize FILENAME:" << filename << std::endl; 
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

float	ft_get_time_sec(void)
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
	return ((now.tv_sec - begin.tv_sec) * 1000 + (now.tv_usec - begin.tv_usec) / 1000);
}

unsigned int			getipbyhost(const char *host, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *result;
	int	g_error;
	in_addr_t ret;

	std::cout << "get ip by host [" << host << "][" << service << "]" << std::endl;
	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((g_error = getaddrinfo(host, service, &hints, &result )) != 0)
		throw(std::runtime_error(gai_strerror(g_error)));
	ret = ((struct sockaddr_in*)(result->ai_addr))->sin_addr.s_addr;
	freeaddrinfo(result);
	std::cout << "ret : [" << ret << "]" << std::endl;
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


std::string _tmpFileName(const std::string path)
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
			fileName.clear();
	}
	return (NULL);
}