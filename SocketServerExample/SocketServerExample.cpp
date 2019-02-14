
#include "pch.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

void printWsaData(const WSADATA &wsaData)
{
	std::cout << "wsaData\n";
	std::cout << "----------------------\n";
	std::cout << "wVersion: " << wsaData.wVersion << '\n';
	std::cout << "wHighVersion: " << wsaData.wHighVersion << '\n';
	std::cout << "iMaxSockets: " << wsaData.iMaxSockets << '\n';
	std::cout << "iMaxUdpDg: " << wsaData.iMaxUdpDg << '\n';
	//std::cout << "lpVendorInfo: " << (wsaData.lpVendorInfo ? wsaData.lpVendorInfo : "null")<< '\n';
	std::cout << "szDescription: " << wsaData.szDescription << '\n';
	std::cout << "szSystemStatus: " << wsaData.szSystemStatus << '\n';
	std::cout << '\n';
}

void printAddrInfo(const addrinfo &ai)
{
	std::cout << "addrinfo\n";
	std::cout << "----------------------\n";
	std::cout << "ai_flags: " << ai.ai_flags << '\n';
	std::cout << "ai_family: " << ai.ai_family << '\n';
	std::cout << "ai_socktype: " << ai.ai_socktype << '\n';
	std::cout << "ai_protocol: " << ai.ai_protocol << '\n';
	std::cout << "ai_addrlen: " << ai.ai_addrlen << '\n';
	std::cout << "ai_canonname: " << (ai.ai_canonname ? ai.ai_canonname : "null") << '\n';
	std::cout << "ai_addr: " << ai.ai_addr << '\n';
	if (ai.ai_addr)
	{
		std::cout << "ai_addr.sa_family: " << (ai.ai_addr)->sa_family << '\n';
		std::cout << "ai-addr.sa_data: " << (ai.ai_addr)->sa_family << '\n';
	}
	std::cout << "ai_next: " << ai.ai_next << '\n';
	std::cout << '\n';
}

int main()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != 0)
	{
		std::cout << "WSAStartup failed: " << iResult << '\n';
		return 1;
	}
	printWsaData(wsaData);

	struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	printAddrInfo(hints);

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if(iResult != 0)
	{
		std::cout << "getaddrinfo failed: " << iResult << '\n';
		WSACleanup();
		return 1;
	}
	std::cout << "hints: ";
	printAddrInfo(hints);
	std::cout << "result: ";
	printAddrInfo(*result);

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << WSAGetLastError() << '\n';
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	std::cout << "ListenSocket: " << ListenSocket << '\n';
	std::cout << '\n';

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		std::cout << "bind failed with error: " << WSAGetLastError() << '\n';
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	std::cout << "bind result: " << iResult << '\n';

	iResult = listen(ListenSocket, SOMAXCONN);
	if(iResult == SOCKET_ERROR)
	{
		std::cout << "Listen failed with error: " << WSAGetLastError() << '\n';
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	std::cout << "listen result: " << iResult << '\n';

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	std::cout << "accept socket: " << ClientSocket << '\n';
	if(ClientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed: " << WSAGetLastError() << '\n';
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	closesocket(ListenSocket);

	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	do 
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			std::cout << "Bytes received: " << iResult << '\n';
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				std::cout << "send failed: " << WSAGetLastError() << '\n';
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			std::cout << "Bytes sent: " << iSendResult << '\n';
		}
		else if (iResult == 0)
			std::cout << "Connection closing... \n";
		else
		{
			std::cout << "recv failed: " << WSAGetLastError << '\n';
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "shutdown failed: " << WSAGetLastError() << '\n';
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
}

