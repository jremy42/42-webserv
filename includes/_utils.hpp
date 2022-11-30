#ifndef UTILS_HPP
#define UTILS_HPP

# include <string>
# include <vector>
# include <map>
# include <iostream>
# include <sstream>
# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>

#include <sys/time.h>
#include <stddef.h>

# include <arpa/inet.h>
# include <netdb.h>
# include <string.h>
# include <sys/epoll.h>
# include <time.h>
# include <cstdlib>
# include <unistd.h>


# define MAX_FILENAME 6
# define MAX_TRY 200

# ifndef DEBUG_UTILS
#  define DEBUG_UTILS 0
# endif

std::string		&strtrim(std::string &str, const std::string &charset);
std::string		normalizeKeyValStr(std::string &keyValStr, const std::string &separatorCharset, const char defaultSeparator);

std::ostream	&operator<<(std::ostream &o, const std::vector<std::string> &vec);
std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair);
std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map);
std::ostream	&operator<<(std::ostream &o, const std::map<int, std::string> &map);
std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::string> &map);

std::string		itoa(int statusCode);
std::string		ltoa(long statusCode);
std::string		getFileSizeStr(std::string filename);
long			getFileSize(std::string filename);

int				isDir(std::string fileName);
int 			fileExist(std::string fileName);
std::string		_tmpFileName(const std::string path);
void		__signal(int signal);
unsigned long	ft_get_time(void);
float	ft_get_time_sec(void);
unsigned int			getipbyhost(const char *host, const char *service);
void                    printTimeDebug(int debug,std::string quote, std::string arg);
void                    printAvailableAction(int debug, int _clientFd, int _availableActions);

std::string								getClientAddrFromSocket(int acceptSysCallReturnedFd);
std::string								getRequestedAddrFromSocket(int acceptSysCallReturnedFd);
std::pair<std::string, std::string>		getClientHostnameAndService(int acceptSysCallReturnedFd);

#endif
