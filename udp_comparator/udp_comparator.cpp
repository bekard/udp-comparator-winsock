// udp_comparator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <winsock2.h>
#include <array>
#include <fstream>
#include <memory>
#include <iostream>
#include <string>


int   iPort = 65432;
char  szRecipient[128] = {};
size_t uSize = 0;
std::ofstream ofsDump("dump.dat", std::ios::out | std::ios::binary);

constexpr size_t BUFF_SIZE = 65000;

void usage()
{
	std::cout << "usage: receiver [-p:int] [-r:IP] [-s:size] \n" << std::endl;
	std::cout << "       -p:int   port\n" << std::endl;
	std::cout << "       -r:IP    IP address\n" << std::endl;
	std::cout << "       -s:size  packet size to compare\n" << std::endl;
	exit(0);
}

void ValidateArgs(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if ((argv[i][0] == '-') || (argv[i][0] == '/'))
		{
			switch (tolower(argv[i][1]))
			{
			case 'p':        // Remote port
				if (strlen(argv[i]) > 2)
					iPort = atoi(&argv[i][3]);
				std::cout << "port: " << iPort << std::endl;
				break;
			case 'r':        // Recipient's IP addr
				if (strlen(argv[i]) > 2)
					strcpy_s(szRecipient, sizeof szRecipient, &argv[i][3]);
				std::cout << "ip: " << szRecipient << std::endl;
				break;
			case 's':        // Size of received packet to compare
				if (strlen(argv[i]) > 2)
					uSize = atoi(&argv[i][3]);
				std::cout << "size: " << uSize << std::endl;
				break;
			default:
				usage();
				break;
			}
		}
	}
}


int main(int argc, char** argv)
try
{
	WSADATA wsd;
	SOCKADDR_IN recipient;

	ValidateArgs(argc, argv);

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
		throw std::exception("WSAStartup failed!");
	}
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		throw std::exception("socket() failed");
	}

	recipient.sin_family = AF_INET;
	recipient.sin_port = htons((USHORT)iPort);

	struct hostent* host = nullptr;
	host = gethostbyname(szRecipient);
	if (host) {
		memcpy(&recipient.sin_addr, host->h_addr_list[0], host->h_length);
	}

	int res = bind(s, (SOCKADDR*)&recipient, sizeof(recipient));
	if (res == SOCKET_ERROR) {
		throw std::exception("bind() error");
	}

	sockaddr_in from = {};
	int from_len = sizeof(from);

	auto buff = std::make_unique<char[]>(BUFF_SIZE);
	int sz = 0;

	std::cout << "Start...\n";
	do {
		sz = recvfrom(s, buff.get(), BUFF_SIZE, 0, (SOCKADDR*)&from, &from_len);
		ofsDump.write(buff.get(), sz);
		if (sz == SOCKET_ERROR) {
			std::cerr << '\t' << "recvfrom() error: " + WSAGetLastError() << '\n';
		}
		else {
			if (sz == uSize) {
				std::cout << "success!\n";
			}
			else {
				std::cout << "failure: recvfrom = " << sz << " \tsize = " << uSize << '\n';
			}
			
		}
	} while (true);

	closesocket(s);
	WSACleanup();
	return 0;

}
catch (std::exception& ex)
{
	std::cerr << "Message: " <<  ex.what() << "\t\tWsaLastError: " << WSAGetLastError() << std::endl;
	std::system("pause");
	return 1;
}
