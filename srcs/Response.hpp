#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#define WRITE_BUFFER_SIZE 1024

#include "Request.hpp"

Class Response
{
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;

	public:

		Response(int clientFd, const &Request);
		Response(const Response &src);
		const Response &operator=(const Response &rhs) const;
		~Response(void);

		int	createResponse(void);
		int	writeClientResponse(void);

	private:

		int			_responseReady; // CGI donc temps de process possible ?
		int			_clientFd;
		m_ss		_header;
		v_c			_body;

};

#endif

