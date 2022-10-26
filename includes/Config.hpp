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
		Config();
		Config(string listenPort);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);

		const char*	getListenPort(void) const;

	private:
		string	_listenPort;
		// et ? ....
};

#endif
