#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>

class Config
{
	typedef std::string			string;

	public:
		Config(void);
		Config(string listenPort, string rootDirectory);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);

		const char*	getListenPort(void) const;

	private:
		string	_listenPort;
		string	_rootDirectory;
};

#endif
