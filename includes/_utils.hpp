#ifndef UTILS_HPP
#define UTILS_HPP

# include <string>
# include <vector>
# include <map>
# include <iostream>
# include <sstream>

std::string		&strtrim(std::string &str, const std::string &charset);
std::string		normalizeKeyValStr(std::string &keyValStr, const std::string &separatorCharset, const char defaultSeparator);

std::ostream	&operator<<(std::ostream &o, const std::vector<std::string> &vec);
std::ostream	&operator<<(std::ostream &o, const std::pair<std::string, std::vector<std::string> > &pair);
std::ostream	&operator<<(std::ostream &o, const std::map<std::string, std::vector<std::string> > &map);

std::string		itoa(int statusCode);
#endif
