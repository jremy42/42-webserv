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


std::string		&strtrim(std::string &str, const std::string &charset);
std::string		normalizeKeyValStr(std::string &keyValStr, const std::string &separatorCharset, const char defaultSeparator);

std::ostream	&operator<<(std::ostream &o, const std::vector<std::string> &vec);
std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair);
std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map);

std::string		itoa(int statusCode);
std::string		ltoa(long statusCode);
std::string		getFileSize(std::string filename);
int				isDir(std::string fileName);

#endif
