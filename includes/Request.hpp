#ifndef REQUEST_HPP
#define REQUEST_HPP

#define READ_BUFFER_SIZE 1024

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


enum {R_REQUESTLINE, R_HEADER, R_BODY, R_END, R_ERROR};

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

		int			readClientRequest(void);
		int			getState(void) const;
		string		&getStateStr(void) const;
		string		getMethod(void) const;
		string		getProtocol(void) const;
		string		getTarget(void) const;
		int			getStatusCode(void) const;

	private:

		int				_state;
		int				_clientFd;
		int				_statusCode;
		m_ss			_requestLine;
		m_ss			_header;
		v_c				_body;
		v_c				_rawRequest;
		string			_rawRequestLine;
		static string	_requestLineField[3];
		static string	_headerField[3];
		static string	_validRequest[3];
		static string	_stateStr[5];

		void	_handleRequestLine(void);
		void	_handleHeader(void);
		int		parseRequestLine(string rawRequestLine);
		int		parseHeader(string rawRequestLine);
		int		checkRequestLine(void);
		int		checkHeader(void);

};

std::string	&strtrim(std::string &str, const std::string &charset);

#endif
