#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <algorithm>
# include <fstream>
# include <map>
# include <vector>
# include <iostream>
# include <string>
# include <sstream>
# include "Request.hpp"
# include "Response.hpp"
# include <iterator>

# define DEBUG_LOCATION 1

class Location
{
	public:
		typedef std::string			string;
		typedef std::map<string, std::vector<string> > m_s_vs;

		Location(void);
		Location(const Location &src);
		Location(string rawLocation);
		~Location(void);
		Location &operator=(const Location &rhs);
		std::string	normalizeKeyValStr(std::string &keyValStr, const std::string &separatorCharset, const char defaultSeparator);

	private:
		void	_createLocationInfoMap(std::string &rawServerConf);
		std::pair<std::string, std::vector<std::string > >	parseLocationLine(std::string &nextLine);
		m_s_vs	_locationInfoMap;
		static std::map<string, int>	_configField;
		static std::map<string, int> _initConfigField(void);
};


#endif
