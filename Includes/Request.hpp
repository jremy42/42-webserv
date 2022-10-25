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

enum {R_REQUESTLINE, R_HEADER, R_BODY, R_END, R_ERROR};

class Request
{
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;

	public:

		Request(void);
		Request(int clientFd);
		Request(const Request &src);
		Request &operator=(const Request &rhs);
		~Request(void);

		int	readClientRequest(void);

	private:

		int			_state;
		int			_clientFd;
		m_ss		_requestLine;
		m_ss		_header;
		v_c			_body;
		v_c			_rawRequest;

		int	parseError(string rawRequestLine);
};

#endif
