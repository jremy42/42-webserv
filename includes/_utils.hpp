#ifndef UTILS_HPP
#define UTILS_HPP

# include <string>
# include <vector>
# include <map>
# include <iostream>
# include <sstream>
# include <cstdio>
# include <sys/types.h>
# include <sys/stat.h>

#include <sys/time.h>
#include <stddef.h>

# include <arpa/inet.h>
# include <netdb.h>
# include <cstring>
# include <sys/epoll.h>
# include <time.h>
# include <cstdlib>
# include <unistd.h>
#include <iostream>
#include <cstdarg>


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
int 			accessFileParentDir(std::string fileName);
std::string		tmpFileName(const std::string path);
void		__signal(int signal);
unsigned long	ft_get_time(void);
unsigned long	ft_get_time_sec(void);
unsigned int			getipbyhost(const char *host, const char *service);
void                    printTimeDebug(int debug,std::string quote, std::string arg);
void                    printAvailableAction(int debug, int _clientFd, int _availableActions);
void                 	printLog(int log, int fd, int nb, ...);
void 					printLogServer(int log, int fd);

std::string								getClientAddrFromSocket(int acceptSysCallReturnedFd);
std::string								getClientPortFromSocket(int acceptSysCallReturnedFd);
std::string								getRequestedAddrFromSocket(int acceptSysCallReturnedFd);
std::string								getRequestedPortFromSocket(int acceptSysCallReturnedFd);
std::pair<std::string, std::string>		getClientHostnameAndService(int acceptSysCallReturnedFd);
std::string								subStringBeforeFirstDelim(std::string &str, char delim);
std::string								subStringAfterFirstDelim(std::string &str, char delim);
std::string								subStringBeforeLastDelim(std::string &str, char delim);
std::string								subStringAfterLastDelim(std::string &str, char delim);

#endif
