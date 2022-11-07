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
# include "_utils.hpp"

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

	private:
		m_s_vs	_locationInfoMap;

		void												_createLocationInfoMap(std::string &rawServerConf);
		std::pair<std::string, std::vector<std::string > >	parseLocationLine(std::string &nextLine);

		static std::map<string, int>	_configField;
		static std::map<string, int>	_initConfigField(void);
};

#endif
