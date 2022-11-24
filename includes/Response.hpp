#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#define WRITE_BUFFER_SIZE	1024*1024*8

# include "Request.hpp"
# include <string>
# include <vector>
# include <map>
# include <sstream>
# include <sys/types.h>
# include <sys/socket.h>
# include <fstream>
# include <algorithm>
# include "Config.hpp"
# include <dirent.h>
# include <linux/limits.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>       /* time_t, struct tm, time, localtime, strftime */
# include "_utils.hpp"
# include <iostream>
# include <iomanip>
# include <sys/wait.h>

# define MAX_PATH 4092
# ifndef DEBUG_RESPONSE
#  define DEBUG_RESPONSE 1
# endif
enum {R_INIT, R_WAIT_CGI_EXEC, R_FILE_READY, R_WRITE, R_OVER};
class Config;

class Response
{
	public:
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;
	typedef std::map<int, string>		m_is;

	public:
		Response(void);
		Response(int clientFd, Request *request, const Config *config, int statusCode);
		Response(const Response &src);
		Response &operator=(const Response &rhs);
		~Response(void);
		void setRequest(const Request *request);
		int handleResponse(void);
		void reset(void);

	private:

		int								_responseReady; // CGI donc temps de process possible ?
		int								_clientFd;
		int								_statusCode;
		int								_state;
		string							_lineStatus;
		string							_header;
		v_c								_body;
		v_c								_fullHeader;
		string							_defaultErrorBodyToSend; // tmp
		const Request *					_request;
		const Config *					_config;
		std::ifstream					_fs;
		int								_bodyLength;
		std::stringstream				_ss; // is in fact defautlErrorbodytoSendStringStream;

		static std::map<int, string>	_statusCodeMessage;
		static string					_errorBodyTemplate;
		static string					_autoIndexBodyTemplate;

		static m_is 					_initStatusCodeMessage(void);
		void							_createErrorMessageBody(void);
		void							_createBody(void);
		void							_createHeaderBase(void);
		void							_createFullHeader(void);
		void							_checkAutorizationForMethod(void);
		void							_checkRedirect(void);
		string							_getExtensionFromTarget(string actualTarget);
		int								_createResponse(void);
		int								_writeClientResponse(void);
//GET
		void							_methodGET(void);
		int								_createAutoIndex(const string &pathToDir);
		std::map<string, unsigned int>	_populateDirectoryMap(const char *path);
		std::string						_generateHTMLBodyWithPath(void);
		string							_selectActualTarget(string &actualTarget, string requestTarget);
		void							_createBodyFromFile(const string &actualTarget);
		void							_createFileStreamFromFile(string actualTarget);
// POST
		void							_methodPOST(void);
// CGI
		pid_t	_pid;
		char _nameIn[32];
		char _nameOut[32];
		int	_inChild;
		int _outChild;
		//void	_handleCGI(string actualTarget, string cgiExecutable);
		//void 	_parentPartCgi(int pipefdParentToChild[2], int pipefdChildToParent[2], pid_t pid);
		void	_initCGIfile(string actualTarget, string cgiExecutable);
		//void	_extractHeaderFromCgiBody(void);
		void 	_waitCGIfile();
};

#endif
