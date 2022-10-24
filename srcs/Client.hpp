#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
# include <iostream>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>

enum {S_INIT, S_REQREAD, S_RESWRITE, S_OVER};

class Client
{
	public:
		Client();
		Client(int clientFd);
		Client(const Client &src);
		Client &operator=(const Client &rhs);
		~Client(void);

		void	setAvailableActions(int epoll_flags); //Set les actions (read request ou write response)
		int		executeAction(void); //Fait une action de read request ou de write response selon l'etat
		int		getClientFd(void) const;
	private:

		int			_clientFd;
		//Request		_request;
		//Response	_response;
		int			_state;
		int			_availableActions;
};

#endif
