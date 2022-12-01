#ifndef MULTIPART_HPP
#define MULTIPART_HPP

# define MAX_MULTIPARTLINE_SIZE 4096
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
# include <iomanip>
# include <fstream>
# include <stdlib.h>
# include <string.h>
# include "_utils.hpp"

# ifndef DEBUG_MULTIPART
#  define DEBUG_MULTIPART 0
# endif


class Multipart
{
	public:
		typedef std::string					string;
		typedef std::map<string, string>	m_ss;
		typedef std::vector<string>			v_s;


	public:

		Multipart(void);
		Multipart(string bodyFile, string boundaryDelim, string uploadDir);
		Multipart(const Multipart &src);
		Multipart &operator=(const Multipart &rhs);
		~Multipart(void);
		int createFilesFromBody(void);
		string getReturnMessage(void);
		bool	getError(void);

	private:
		bool 			_currentFileError;
		bool			_globalError;
		string			_bodyFile;
		string 			_boundaryDelim;
		string			_uploadDir;
		string			_fileName;
		std::fstream	_fs;
		std::fstream	_fsNewFile;
		m_ss			_boundaryHeader;
		string			_returnMessage;
		int		_extractFileFromBody(void);
		int		_createFileFromHeader(void);
		int 	_extractHeader(void);
};

std::string	&strtrim(std::string &str, const std::string &charset);
#endif
