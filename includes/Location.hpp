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

# ifndef DEBUG_LOCATION
#  define DEBUG_LOCATION 0
# endif

class Location
{
	public:
		typedef std::string			string;
		typedef std::map<string, std::vector<string> > m_s_vs;
		typedef std::map<int, string> m_is;
		typedef std::map<string, string> m_ss;

		Location(void);
		Location(const Location &src);
		Location(string rawLocation);
		~Location(void);
		Location &operator=(const Location &rhs);
		const m_s_vs	&getLocationInfoMap(void) const;
		const m_is		&getErrorPageLocation(void) const;
		const m_ss		&getCgi(void) const;


	private:
		m_s_vs	_locationInfoMap;
		m_is	_errorPage;
		m_ss	_cgi;

		void												_createLocationInfoMap(std::string &rawServerConf);
		std::pair<std::string, std::vector<std::string > >	parseLocationLine(std::string &nextLine);
		void												_parseAllowedMethods(void);
		void												_parseAutoindex(void);
		void												_parseErrorPage(string errorNum);
		void												_parseCgi(string extension, string executable);
		void												_initLocationInfoMap(void);

		static std::map<std::string, std::pair<int, int> >	_configField;
		static std::map<std::string, std::pair<int, int> >	_initConfigField(void);;
};

std::ostream	&operator<<(std::ostream &o, const Location &Location);

#endif
