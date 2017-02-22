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
	if (argc < 5)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int timeout = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(int));

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDRESS, (void *) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-') continue;

		switch (argv[i][1])
		{
		case 'a':
			if (inet_pton(AF_INET, argv[i + 1], (void*) &(server_addr.sin_addr.s_addr)) != 1)
			{
				cerr << "Can not convert little-endian to big-endian" << endl;

				return 1;
			}

			break;

		case 'p':
			server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));
			break;
		}
	}

	if (connect(client, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Error! Can not connect to server! Error: " << WSAGetLastError() << endl;

		return 1;
	}

	cout << "Connected!" << endl;

	string file_name;
	char name_to_save[2000];

	cout << "File send to server: " << endl;
	cin >> file_name;

	cout << "Name to save in server: " << endl;
	cin >> name_to_save;

	send(client, name_to_save, strlen(name_to_save), 0);

	ifstream file;
	file.open(file_name, ios::binary);

	if (file.good())
	{
		file.seekg(0, ios::end);
		unsigned int file_size = file.tellg();
		
		char* file_buffer = new char[file_size];
		file.read(file_buffer, file_size);

		unsigned int offset = 0;
		int bytes_to_send = 0;
		int ret;

		while (offset < file_size)
		{
			if (file_size - offset >= BUFFER_SIZE)
			{
				bytes_to_send = BUFFER_SIZE;
			}
			else
			{
				bytes_to_send = file_size - offset;
			}
			send(client, file_buffer + offset, bytes_to_send, 0);
			offset += bytes_to_send;
		}
	}
	else
	{
		cerr << "Can not open your file!" << endl;
	}

	shutdown(client, SD_SEND);
	closesocket(client);

	WSACleanup();

	return 0;
}