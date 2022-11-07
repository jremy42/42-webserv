# include "Location.hpp"

std::map<std::string, int>	Location::_configField = _initConfigField();

std::map<std::string, int>	Location::_initConfigField()
{
	std::map<string, int> configField;

	configField.insert(std::pair<std::string, int>("root", 0));
	return (configField);
}

Location::Location(string rawLocation)
{
	if (DEBUG_LOCATION)
		std::cout << "Location string constructor called with : ~>" << rawLocation << "<~" << std::endl;
	_createLocationInfoMap(rawLocation);
	if (DEBUG_LOCATION)
		std::cout << "Location Info Map at end of constructor : ~>" << _locationInfoMap << "<~" << std::endl;
}

Location::~Location(void)
{

}

Location::Location(const Location &src)
{
	*this = src;
}
Location	&Location::operator=(const Location &rhs)
{
	this->_locationInfoMap = rhs._locationInfoMap;
	return (*this);
}

std::pair<std::string, std::vector<std::string > >	Location::parseLocationLine(std::string &nextLine)
{
	std::pair<string, std::vector<string> >		ret;
	std::istringstream							iss(nextLine);
	string										key;
	string										value;

	getline(iss, key, ' ');
	if (_configField.find(key) != _configField.end())
	{
		ret.first = key;
		while (getline(iss, value, ' '))
			ret.second.push_back(value);
	}
	return (ret);
}

void	Location::_createLocationInfoMap(std::string &rawServerConf)
{
	std::istringstream								istr(rawServerConf);
	std::string										nextLine;
	std::pair<string, std::vector<string> >			locationLine;


	for (std::map<std::string, int>::iterator it = _configField.begin(); it != _configField.end(); it++)
		_locationInfoMap.insert(std::pair<std::string, std::vector<std::string> >((*it).first, std::vector<string>()));
	while (getline(istr, nextLine, ';'))
	{
		normalizeKeyValStr(nextLine, "\f\t\n\r\v ;", ' ');
		locationLine = parseLocationLine(nextLine);
		if (!locationLine.first.empty())
			_locationInfoMap[locationLine.first] = locationLine.second;
	}
}
const Location::m_s_vs	&Location::getLocationInfoMap(void) const
{
	const m_s_vs	&LocRef = this->_locationInfoMap;
	return (LocRef);

}

std::ostream	&operator<<(std::ostream &o, const Location &Location)
{
	std::cout << "-------------------Location Printer Start-------------------" << std::endl;
	std::cout << Location.getLocationInfoMap();
	std::cout << "--------------------Location Printer End--------------------" << std::endl;
	return (o);
}
