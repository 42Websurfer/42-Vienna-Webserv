#include "Connection.hpp"

Connection::Connection(Server& a_server, int a_clientSocket) : m_server(a_server), m_clientSocket(a_clientSocket), 
m_chunked(false)
{
	// std::cout << "New connection on fd: " << m_clientSocket << '\n';
	m_idleStart = std::time(NULL);
}

Connection::Connection(const Connection &a_other)
	: m_server(a_other.m_server), m_clientSocket(a_other.m_clientSocket), m_head(a_other.m_head), m_body(a_other.m_body), m_idleStart(std::time(NULL)), m_chunked(a_other.m_chunked)  {}

Connection &Connection::operator=(const Connection &a_other)
{
	if (this != &a_other)
	{
		m_clientSocket = a_other.m_clientSocket;
		m_head = a_other.m_head;
		m_body = a_other.m_body;
		m_server = a_other.m_server;
		m_idleStart = a_other.m_idleStart;
		m_chunked = a_other.m_chunked;
	}
	return (*this);
}

Connection::~Connection(void) {}

std::string	Connection::readUntilSep(std::string& a_ouput, const std::string& a_seperator)
{
	char		buffer[BUFFER_SIZE];
	std::size_t	sepPos = std::string::npos;
	std::string	remainder;
	int			recvRet = 0;

	do
	{
		recvRet = recv(m_clientSocket, buffer, BUFFER_SIZE, 0);
		if (recvRet == -1)
			throw (std::runtime_error("Error: recv bad socket?"));
		a_ouput.append(buffer, recvRet);
		if (!a_seperator.empty())
			sepPos = a_ouput.find(a_seperator);
		// std::cout << "Condition1: " << (recvRet == BUFFER_SIZE) << " Conditon2: " << (sepPos == std::string::npos) << '\n';
	} while (recvRet == BUFFER_SIZE && sepPos == std::string::npos);

	if (sepPos != std::string::npos)
	{
		remainder.append(a_ouput.begin() + sepPos + 4, a_ouput.end());
		a_ouput.erase(a_ouput.begin() + sepPos, a_ouput.end());
	}
	return (remainder);
}

int Connection::getSocketFd(void) const
{
	return (m_clientSocket);
}

int Connection::receiveRequestRaw(void)
{
	try
	{
		std::string remainder;
		if (!m_chunked)
		{
			m_head.clear();
			remainder = readUntilSep(m_head, "\r\n\r\n");
			std::cout << remainder << " | " << m_head << '\n';
			m_request = Request(m_head);
		}
		if (!remainder.empty() || m_chunked)
		{
			m_body.clear();
			m_body.append(remainder);
			readUntilSep(m_body, "");
			std::cout << "Bod: " << m_body << '\n';
			m_request.addBody(m_body);
		}
		if (m_request.requestComplete())
		{
			m_response = Response(m_request, m_server.getSubServer(m_request.getRequestHost()).getValidConfig(m_request.getValue("uri")));
			m_response.createResponseMsg();
			m_chunked = false;
		}
		else 
			m_chunked = true;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (-1);
	}
	m_idleStart = std::time(NULL);
	return (0);
}

int Connection::sendResponse(void)
{
	const std::string	response = m_response.getResponse();
	m_response.clearBody();
	//std::cout << "Response:\n" << response << '\n';
	m_idleStart = std::time(NULL);
	return (send(m_clientSocket, response.data(), response.size(), 0));
}

time_t Connection::getIdleTime(void) const
{
	return (std::time(NULL) - m_idleStart);
}

bool Connection::operator==(const int a_fd) const
{
	return (m_clientSocket == a_fd);
}

void Connection::printHeadNBody(void) const
{
	std::cout << "Head:\n" << m_head << '\n';
	std::cout << "Body:\n" << m_body << '\n';
}
