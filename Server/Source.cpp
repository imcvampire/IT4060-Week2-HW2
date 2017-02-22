#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>
#include<cstdbool>
#include<fstream>

using namespace std;

const char SERVER_ADDRESS[] = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 64 * 1024;
const char MY_ERROR[] = "Error!";

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA	wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET listen_socket;
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);

	if (inet_pton(AF_INET, SERVER_ADDRESS, (void*) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	// Start from first argument
	for (int i = 1; i <= argc && argv[i][0] == '-'; ++i)
	{
		if (argv[i][1] == 'p')
		{
			server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));

			break;
		}
	}

	if (bind(listen_socket, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Can not bind to this address!" << endl;

		return 1;
	}

	if (listen(listen_socket, 10))
	{
		cerr << "Can not not listen!" << endl;

		return 1;
	}

	cout << "Server started!" << endl;

	sockaddr_in client_addr;
	char buffer_filename[BUFFER_SIZE];
	int ret_filename;
	int client_addr_len = sizeof(client_addr);

	for (;;)
	{
		SOCKET connect_socket = accept(listen_socket, (sockaddr *) &client_addr, &client_addr_len);

		ret_filename = recv(connect_socket, buffer_filename, BUFFER_SIZE, 0);

		if (ret_filename == SOCKET_ERROR)
		{
			cerr << "Error: " << WSAGetLastError() << endl;
		}
		else if (ret_filename > 0)
		{
			buffer_filename[ret_filename] = 0;
			ofstream file;

			file.open(buffer_filename);

			if (file.good())
			{
				int offset = 0;
				int ret_content;
				char buffer_content[BUFFER_SIZE];

				for (;;)
				{
					ret_content = recv(connect_socket, buffer_content, BUFFER_SIZE, 0);

					if (ret_content <= 0)
					{
						break;
					}
					else
					{
						file << buffer_content;
					}
				}

				file.close();
			}
			else
			{
				cerr << "File already exists!" << endl;
			}
		}

		shutdown(connect_socket, SD_SEND);
		closesocket(connect_socket);
	}

	cout << "Bye!" << endl;

	closesocket(listen_socket);

	WSACleanup();

	return 0;
}
