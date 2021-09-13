#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>

#define MAX_EVENTS 32

bool set_nonblock(int handle) //перевод сокета в неблокирующий режим
{
    unsigned long int on = 1;
    const int result = ioctl(handle, FIONBIO, &on);
    if (result < 0) {
        perror("errrrrrr");
        return false;
    }
    return true;
}

std::vector<std::string> split (const std::string &s, char delim) //сплит строки
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

static const size_t buffer_size = 255;
class Server
{
private:
	std::string dir;
	size_t port;

	int MasterSocket;
	sockaddr_in SockAddr = {};
  int EPoll;

public:
	Server(const std::string& dir, size_t port) : dir(dir), port(port)
	{
		SockAddr.sin_family = AF_INET;
		SockAddr.sin_port = htons(port);
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		bind(MasterSocket, (sockaddr*)&SockAddr, sizeof(SockAddr));
		set_nonblock(MasterSocket);
	}
	~Server()
	{
		shutdown(MasterSocket, SHUT_RDWR);
		close(MasterSocket);
	}
	void start()
	{
		listen(MasterSocket, SOMAXCONN);
		///epoll///
		EPoll = epoll_create1(0);
		epoll_event Event;
		Event.data.fd = MasterSocket;
		Event.events = EPOLLIN; // доступность на чтение
		epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);//Регистрация epoll_event

    Handler();


	}
private:
	void Handler()
	{
    while(1)
		{
			epoll_event Events[MAX_EVENTS];
			int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);

			for(size_t i=0; i<N; i++)
			{
				if(Events[i].data.fd == MasterSocket)//Если мастер сокет, то принять новое соединение
				{
					auto client = accept(MasterSocket, 0, 0);
					set_nonblock(client);

					epoll_event Event;
					Event.data.fd = client;
					Event.events = EPOLLIN;
					epoll_ctl(EPoll, EPOLL_CTL_ADD, client, &Event);
				}
				else//если клиент, то обработать его запрос
				{
					char* buff = new char[buffer_size];
					int RecvResult = recv(Events[i].data.fd, buff, buffer_size, MSG_NOSIGNAL);
					if((RecvResult == 0) && (errno != EAGAIN))
					{
						shutdown(Events[i].data.fd, SHUT_RDWR);
						close(Events[i].data.fd);
					}
					else if (RecvResult > 0)
					{
            std::string buffer = buff;
            std::vector<std::string> vec= split(buffer, ' ');

						if(vec[0] == "remove")
            {
              std::string name = dir + vec[1];
              std::cout<<name<<std::endl;
              if(remove(name.c_str()))
              {
                std::string answ = "Error remove";
                send(Events[i].data.fd, answ.c_str(), answ.size(), MSG_NOSIGNAL);
              }
              else
              {
                std::string answ = "remove success!";
                send(Events[i].data.fd, answ.c_str(), answ.size(), MSG_NOSIGNAL);
              }

            }
            else if(vec[0] == "make")
            {
              std::ofstream newfile (dir + vec[1]);
              std::cout<<(dir + vec[1])<<std::endl;
              std::string answ = "make!";
              send(Events[i].data.fd, answ.c_str(), answ.size(), MSG_NOSIGNAL);
            }
            else
            {
              std::string answ = "wrong cmd";
              send(Events[i].data.fd, answ.c_str(), answ.size(), MSG_NOSIGNAL);
            }

					}
					delete[] buff;
				}
			}
		}
	}

};



int main (int argc, char const *argv[])
{
	Server myServ("/home/dortp68/test/", 1234);
	myServ.start();

	return 0;
}
