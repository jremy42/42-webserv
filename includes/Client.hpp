#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <cstdio>
# include <unistd.h>
# include <cstdlib>
# include <cstring>
# include <algorithm>
# include "Config.hpp"
# include "Server.hpp"

# ifndef DEBUG_CLIENT
#  define DEBUG_CLIENT 0
# endif

# define TIMEOUT_REQUEST 5000
# define TIMEOUT_CLIENT 45000

enum {S_INIT, S_REQREAD, S_RESWRITE, S_OVER, S_CLOSE_FD};
class Server;

class Client
{
	public:
		typedef std::vector<Config> v_config;

	public:
		Client();
		Client(int clientFd, v_config *config, Server *myServ, unsigned int host);
		Client(const Client &src);
		Client &operator=(const Client &rhs);
		~Client(void);

		void			setAvailableActions(int epoll_flags); //Set les actions (read request ou write response)
		int				executeAction(void); //Fait une action de read request ou de write response selon l'etat
		int				getClientFd(void) const;
		int				getState(void) const;
		std::string&	getStateStr(void) const;
		const Config*			getMatchingConfig(void) const;
		Server*			getMyServer(void) const;

	private:

		int					_clientFd;
		Request*			_request;
		Response*			_response;
		int					_state;
		int					_availableActions;
		static std::string	_stateStr[4];
		v_config			_configList;
		Server*				_myServ;
		unsigned long		_timeoutRequest;
		unsigned long		_timeoutClient;
		v_config			_matchingConfigListByHost(v_config * configList, unsigned int host);

};

#endif
