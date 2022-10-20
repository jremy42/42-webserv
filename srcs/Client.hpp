#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"

enum {S_INIT, S_REQREAD, S_RESWRITE, S_OVER};

Class Client
{
	public:

		Client(int ClientFd);
		Client(const Client &src);
		const Client &operator=(const Client &rhs) const;
		~Client(void);

		void	setAvailableActions(int epoll_flags); //Set les actions (read request ou write response)
		void	executeAction(void); //Fait une action de read request ou de write response selon l'etat

	private:

		int			_clientFd;
		Request		_request;
		Response	_response;
		int			_state;
		int			_availableActions;
};

#endif
