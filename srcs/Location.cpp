# include "Location.hpp"

std::map<std::string, int>	Location::_configField = _initConfigField();

std::map<std::string, int>	Location::_initConfigField()
{
	std::map<string, int> configField;

	configField.insert(std::pair<std::string, int>("listen", -1));
	configField.insert(std::pair<std::string, int>("root", 0));
	configField.insert(std::pair<std::string, int>( "server_name", -1));

	return (configField);
}

Location::Location(string rawLocation)
{
	std::cout << "\e[35m rawLocation:" << rawLocation << "\e[0m" << std::endl;
}

Location::~Location(){};

Location::Location(const Location &src)
{
	*this = src;
}

Location	&Location::operator=(const Location &rhs)
{
	this->_locationInfoMap = rhs._locationInfoMap;
	return (*this);
}

// std::map<std::string, std::vector<std::string> > _createLocationInfoMap(std::string &rawServerConf)
// {
	
// };
