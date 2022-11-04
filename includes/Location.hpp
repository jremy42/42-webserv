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

# define DEBUG 1


class Location
{
	public:
		typedef std::string			string;
		typedef std::map<string, std::vector<string> > m_s_vs;
		Location(string rawLocation);
		Location(const Location &src);
		~Location(void);
		Location &operator=(const Location &rhs);

	private:
		std::map<std::string, std::vector<std::string> > _createLocationInfoMap(std::string &rawServerConf);
		m_s_vs	_locationInfoMap;
		static std::map<string, int>	_configField;
		static std::map<string, int> _initConfigField(void);
};


#endif
