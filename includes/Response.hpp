#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#define WRITE_BUFFER_SIZE 1024

# include "Request.hpp"
# include <string>
# include <vector>
# include <map>
# include <sstream>


class Response
{
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;
	typedef std::map<int, string>		m_is;

	public:
		Response(void);
		Response(int clientFd);
		Response(const Response &src);
		Response &operator=(const Response &rhs);
		~Response(void);
		void setRequest(const Request *request);
		int	createResponse(void);
		int	writeClientResponse(void);

	private:

		int								_responseReady; // CGI donc temps de process possible ?
		int								_clientFd;
		int								_statusCode;
		string							_lineStatus;
		m_ss							_header;
		v_c								_body;
		const Request *					_request;
		static std::map<int, string>	_errorMessage;
		static m_is 					_initErrorMessage(void);

};
 
std::string _itoa(int statusCode);

#endif

