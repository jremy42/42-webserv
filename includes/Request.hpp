#ifndef REQUEST_HPP
#define REQUEST_HPP

#define READ_BUFFER_SIZE 512
# define MAX_REQUESTLINE_SIZE 4096
# define MAX_HEADER_SIZE 4096

# include <string>
# include <vector>
# include <map>
# include <unistd.h>
# include <stdexcept>
# include <cstring>
# include <errno.h>
# include <iostream>
# include <sstream>
# include <iterator>

# define VALID_REQUEST_N 3
# define HEADER_FIELD 3
# define REQUEST_LINE_FIELD 3

# ifndef DEBUG_REQUEST
#  define DEBUG_REQUEST 0
# endif

enum {R_REQUESTLINE, R_HEADER, R_GET_MAX_BODY_SIZE, R_BODY, R_END, R_ERROR, R_ZERO_READ};

class Request
{
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;
	typedef v_c::iterator				v_c_it;

	public:

		Request(void);
		Request(int clientFd);
		Request(const Request &src);
		Request &operator=(const Request &rhs);
		~Request(void);

		int			readClientRequest(int do_read);
		int			getState(void) const;
		string		&getStateStr(void) const;
		string		getMethod(void) const;
		string		getProtocol(void) const;
		string		getTarget(void) const;
		int			getStatusCode(void) const;
		void 		reset(void);
		string		getHost(void) const;
		void		setClientMaxBodySize(int clientMaxBodySize);
		void		setState(int state);

	private:

		int				_state;
		int				_clientFd;
		int				_statusCode;
		m_ss			_requestLine;
		m_ss			_header;
		v_c				_body;
		v_c				_rawRequest;
		string			_rawRequestLine;
		int				_clientMaxBodySize;
		static string	_requestLineField[3];
		static string	_headerField[3];
		static string	_validRequest[3];
		static string	_stateStr[7];

		void	_handleRequestLine(void);
		void	_handleHeader(void);
		void	_handleBody(void);
		int		parseRequestLine(string rawRequestLine);
		int		parseHeader(string rawRequestLine);
		int		checkRequestLine(void);
		int		checkHeader(void);

};

std::string	&strtrim(std::string &str, const std::string &charset);

#endif
