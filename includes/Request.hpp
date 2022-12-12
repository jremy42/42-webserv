#ifndef REQUEST_HPP
#define REQUEST_HPP

# define READ_BUFFER_SIZE 1024 * 16
# define MAX_REQUESTLINE_SIZE READ_BUFFER_SIZE * 4
# define MAX_HEADER_SIZE READ_BUFFER_SIZE * 2

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
# include <iomanip>
# include <fstream>
# include <stdlib.h>
# include <string.h>
# include "Config.hpp"
# include "Multipart.hpp"

# define VALID_REQUEST_N 3
# define HEADER_FIELD 3
# define REQUEST_LINE_FIELD 3

# ifndef DEBUG_REQUEST
#  define DEBUG_REQUEST 0
# endif

enum {R_REQUESTLINE, R_HEADER, R_SET_CONFIG, R_INIT_BODY_FILE, R_BODY, R_BOUNDARY_HEADER,R_END, R_ERROR, R_ZERO_READ};

class Config;
class Multipart;

class Request
{
	public:
		typedef std::string					string;
		typedef std::map<string, string>	m_ss;
		typedef std::vector<char>			v_c;
		typedef v_c::iterator				v_c_it;
		typedef std::vector<Config> 		v_config;
		typedef std::vector<string>			v_s;


	public:

		Request(void);
		Request(int clientFd, v_config *configList);
		Request(const Request &src);
		Request &operator=(const Request &rhs);
		~Request(void);

		int					readClientRequest(void);
		int					getState(void) const;
		string				&getStateStr(void) const;
		string				getMethod(void) const;
		string				getProtocol(void) const;
		string				getTarget(void) const;
		int					getStatusCode(void) const;
		void 				reset(void);
		string				getHost(void) const;
		string				getTmpBodyFile(void) const;
		void				setClientMaxBodySize(int clientMaxBodySize);
		void				setState(int state);
		int					handleRequest(void);
		const Config		*getMatchingConfig(void) const;
		const Config		*getRequestConfig(void) const;
		const Config		*getConfig(void) const;
		m_ss				getHeader(void) const;
		string 				getBoundaryDelim(void) const;
		string 				getUploadDir(void) const;
		string				getLog(void);
		v_s					getContentType(void) const;
		string				getTransfertEncoding(void) const;

	private:
		int				_state;
		int				_clientFd;
		int				_statusCode;
		int				_totalRead;
		m_ss			_requestLine;
		m_ss			_header;
		v_s				_contentType;
		v_c				_rawRequest;
		string			_rawRequestString;
		string			_boundary;
		int				_contentLength;
		int				_readRet;
		string			_rawRequestLine;
		int				_clientMaxBodySize;
		v_config		*_configList;
		const Config	*_config;
		static string	_requestLineField[3];
		static string	_headerField[3];
		static string	_validRequest[3];
		static string	_stateStr[10];

		void	_handleRequestLine(void);
		void	_handleHeader(void);
		void	_handleBody(void);
		void	_handleBodyChunked(void);
		int		parseRequestLine(string rawRequestLine);
		int		parseHeader(string rawRequestLine);
		int		checkRequestLine(void);
		int		_checkHeader(bool initializedHost);
		void	_setConfig(void);
		// check_header
		int	_checkAutorizationForMethod(void);

		//handle body
		void 			_initBodyFile(void);
		string			_nameBodyFile;
		int  			_bodyFileSize;
		std::fstream	_fs;
		void 			_parseContentType(string  rawContentType);
		int				_parseHeaderForBody(void);

		//std::string _rawChunkedSize;
		//int				_chunked;
		//int				_nextChunkSize;
};

std::string	&strtrim(std::string &str, const std::string &charset);

#endif
