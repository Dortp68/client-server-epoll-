#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

class Client
{
private:
	size_t port;
	int ClientSocket;
	sockaddr_in SockAddr = {};
public:
	Client(size_t port):port(port)
	{
		ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		SockAddr.sin_family = AF_INET;
		SockAddr.sin_port = htons(port);
		SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	}
	void start()
	{
		if(connect(ClientSocket, (sockaddr*)&SockAddr, sizeof(SockAddr)))
		{
			std::cout<<"Неудалось подключится к серверу\n";
		}
		else
		{
			Handler();
		}

	}
private:
	void Handler()
	{
		std::string cmd;
		std::string name;
		std::string buff;
		while(1)
		{
			std::cout<<"Введите команду\n";
			std::cin >> cmd;
			if (cmd == "make")
			{
				std::cout<<"Введите имя файла\n";
				std::cin >> name;
				buff = cmd + " " + name;
				char answ[50];
				send(ClientSocket, buff.c_str(), buff.size(), MSG_NOSIGNAL);
				recv(ClientSocket, answ, 50, MSG_NOSIGNAL);
				std::cout<<answ<<std::endl;
			}
			else if (cmd == "remove")
			{
				std::cout<<"Введите имя файла\n";
				std::cin >> name;
				buff = cmd + " " + name;
				char answ[50];
				send(ClientSocket, buff.c_str(), buff.size(), MSG_NOSIGNAL);
				recv(ClientSocket, answ, 50, MSG_NOSIGNAL);
				std::cout<<answ<<std::endl;
			}
			else if (cmd == "out")
			{
				shutdown(ClientSocket, SHUT_RDWR);
				close(ClientSocket);
				break;
			}
			else
			{
				std::cout<<"Неверная команда\n";
			}
		}
	}
};
int main (int argc, char const *argv[])
{
	Client myClient(1234);
	myClient.start();


	return 0;
}
