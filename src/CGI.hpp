#ifndef CGI_HPP
# define CGI_HPP

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "Data.hpp"
#include "Request.hpp"
#include "Timer.hpp"

class CGI
{
private:

	char*						m_path;
	std::vector<char*>			m_argv;
	std::vector<char*>			m_envp;

	t_config					m_config;
	Request&					m_request;
	int							m_pid;
	int							m_inputPipe[2];
	int							m_outputPipe[2];
	int							m_status;
	std::string					m_responseBody;
	std::string					m_filePath;

	CGI(void);

	int		scriptIsExecutable(const std::string& a_filePath) const;
	int		setPath(const std::string& a_filePath);
	int		run();
	void	setArgv(const std::string& a_filePath);
	void	setEnvp();
	void	deleteData();
	CGI(const CGI& other);
	CGI& operator=(const CGI& other);

public:

	CGI(t_config config, Request& a_request);
	~CGI();

	Timer				startTime;

	int					readFromPipe();
	int					execute(std::string a_filePath);
	const std::string& 	getResponseBody() const;
	int					getStatusCode() const;
	void				setUrlQuery(const std::string& a_urlQuery);
	int					io(void);
	int					getFd(void)
	{
		return (m_inputPipe[1] != -1 ? m_inputPipe[1] : m_outputPipe[0]);
	}
};

#endif // !CGI_HPP
