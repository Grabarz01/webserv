#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <poll.h>
class Server
{
private:
	
public:
	Server();
	Server(const Server& other);
	~Server();

	const Server& operator= (const Server& other);

	std::vector<int> ports;
	std::vector<std::string> server_names;
	std::vector<pollfd> fds;
	static void setNonblocking(int socket);
	void addPort(int port);
	void setServer(void);
	void addFD(pollfd fd);
	void serverListen(void);
};

#endif