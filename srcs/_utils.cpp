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


std::string getFileSize(std::string filename)
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
	std::cout << "TIME [" << now.tv_sec * 1000 + now.tv_usec / 1000 <<  "]" << std::endl;
	return (now.tv_sec * 1000 + now.tv_usec / 1000);
}
