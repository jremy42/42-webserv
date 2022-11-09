# include "Location.hpp"

std::map<std::string, std::pair<int, int> >	Location::_configField = _initConfigField();

std::map<std::string, std::pair<int, int> > Location::_initConfigField()
{
	std::map<string, std::pair<int, int> > configField;

	configField.insert(std::pair<std::string, std::pair<int, int> >("root", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("allowed_method", std::pair<int, int>(1,3)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("client_max_body_size", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("autoindex", std::pair<int, int>(1,1)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("index", std::pair<int, int>(1,__INT_MAX__)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("upload", std::pair<int, int>(1,1)));

	configField.insert(std::pair<std::string, std::pair<int, int> >("cgi", std::pair<int, int>(2,2)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("rewrite", std::pair<int, int>(2,2)));
	configField.insert(std::pair<std::string, std::pair<int, int> >("error_page", std::pair<int, int>(2,2)));

	return (configField);
}

Location::Location(string rawLocation)
{
	if (DEBUG_LOCATION)
		std::cout << "Location string constructor called with : ~>" << rawLocation << "<~" << std::endl;
	_createLocationInfoMap(rawLocation);
	_parseErrorPage();
	_parseMaxBodySize();
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
	if (_configField.find(key) != _configField.end() )
	{
		if (DEBUG_LOCATION)
			std::cout << "Location : Found a valid Field : [" << key << "]" << std::endl;
		ret.first = key;
		while (getline(iss, value, ' '))
		{
			if (DEBUG_LOCATION)
				std::cout << "Location Added value: [" << value << "]" << std::endl;
			ret.second.push_back(value);
		}
		if (ret.second.size() < (size_t)_configField.find(key)->second.first
		|| ret.second.size() > (size_t)_configField.find(key)->second.second)
		{
			if (DEBUG_LOCATION)
				std::cout << " Location : Wrong number of config field values : [" << key << "]" << std::endl;
			throw(std::runtime_error("Webserv: Config: Location: Wrong number of config field values : [" + key + "]"));

		}
	}
	else if(!key.empty())
		throw(std::runtime_error("Webserv: Config: Location: not handled in webserver [" + key + "]"));
	return (ret);
}

void	Location::_createLocationInfoMap(std::string &rawServerConf)
{
	std::istringstream								istr(rawServerConf);
	std::string										nextLine;
	std::pair<string, std::vector<string> >			locationLine;


	for (std::map<std::string, std::pair<int, int> >::iterator it = _configField.begin(); it != _configField.end(); it++)
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

void Location::_parseErrorPage(void)
{
	string errorNum = _locationInfoMap.find("error_page")->second[0];
	int errorCode  = atoi(errorNum.c_str());
	if (errorNum.find_first_not_of("1234567890") != std::string::npos || (errorCode < 400 || errorCode > 511))
		throw(std::runtime_error("webserv: config : not valid field in error_page, must be valid error code : [" + errorNum + "]"));
}

void Location::_parseMaxBodySize(void)
{
	string field = _locationInfoMap.find("client_max_body_size")->second[0];

	if (field.find_first_of('m') == field.length() - 1
		&& field.substr(0, field.length() - 1).find_first_not_of("0123456789") == std::string::npos)
	{
		if (atoi(field.c_str()) > 32 || atoi(field.c_str()) < 1)
			throw(std::runtime_error("webserv: config : not valid field , Max Body size must be with in 0 & 32 [" + field+ "]"));
	}
	else
		throw(std::runtime_error("webserv: config : not valid field in max_body_size: [" + field+ "]"));

}

std::ostream	&operator<<(std::ostream &o, const Location &Location)
{
	std::cout << "-------------------Location Printer Start-------------------" << std::endl;
	std::cout << Location.getLocationInfoMap();
	std::cout << "--------------------Location Printer End--------------------" << std::endl;
	return (o);
}
