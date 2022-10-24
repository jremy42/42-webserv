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
		Config(int listenPort);
		Config(const Config &src);
		~Config(void);
		Config &operator=(const Config &rhs);

		int	getListenPort(void) const;
		char*	getCharListenPort(void) const;

	private:
		int	_listenPort;
		// et ? ....
};

#endif
