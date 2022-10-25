#ifndef REQUEST_HPP
#define REQUEST_HPP

#define READ_BUFFER_SIZE 1024

# include <string>
# include <vector>
# include <map>

class Request
{
	typedef std::string					string;
	typedef std::map<string, string>	m_ss;
	typedef std::vector<char>			v_c;

	public:

		Request(int clientFd);
		Request(const Request &src);
		const Request &operator=(const Request &rhs) const;
		~Request(void);

		int	readClientRequest(void);

	private:

		int			_clientFd;
		m_ss		_header;
		v_c			_body;

};

#endif
