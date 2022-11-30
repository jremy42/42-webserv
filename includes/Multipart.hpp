#ifndef MULTIPART_HPP
#define MULTIPART_HPP

# define READ_BUFFER_SIZE 512
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

# define VALID_Multipart_N 3
# define HEADER_FIELD 3
# define MULTIPART_LINE_FIELD 3

# ifndef DEBUG_MULTIPART
#  define DEBUG_MULTIPART 1
# endif


class Multipart
{
	public:
		typedef std::string					string;
		typedef std::map<string, string>	m_ss;
		typedef std::vector<string>			v_s;


	public:

		Multipart(void);
		Multipart(string bodyFile);
		Multipart(const Multipart &src);
		Multipart &operator=(const Multipart &rhs);
		~Multipart(void);
		int createFilesFromBody(void);

	private:
		string			_bodyFile;
		string 			_boundaryDelim;
		string			_uploadDir;
		std::fstream	_fs;
		m_ss			_boundaryHeader;
		void	_extractFileFromBody(void);
		void	_createFileFormHeader(void);
		void 	_extractHeader(void);
};

std::string	&strtrim(std::string &str, const std::string &charset);
#endif
