#pragma once

/*
	TCP client for Kincony network relay

	Copyright 2020 "Captain" Dave Tobias

This software is covered by the MIT software license.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "WinsockTCP.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_PORT "4196" 


WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo* result = NULL, * ptr = NULL, hints;

char	recvbuf[DEFAULT_BUFLEN];
int		iResult;
int		recvbuflen = DEFAULT_BUFLEN;
char	sendbuf[DEFAULT_BUFLEN];




int		OpenTCPSocket()
{
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	return(0);
}



int		CloseTCPSocket()
{
	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	return(0);
}


int		OpenTCPConnection(char *ServerIPAddress, PCSTR TCPPort)
{
	// Resolve the server address and port
	//iResult = getaddrinfo(ServerIPAddress, DEFAULT_PORT, &hints, &result);
	iResult = getaddrinfo(ServerIPAddress, TCPPort, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	return(0);
}

int		CloseTCPConnection()
{
	Sleep(100);		// There is a race condition in the Kincony FW whereby you cannot send and then shutdown immediately lest the data come back corrupted.
// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	return(0);
}


// Sends a message and if no error then return the number of bytes sent otherwise return 0 to indicate nothing sent.
int		SendTCPMessage()
{
	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return(0);
	}
	else
	{
		return(iResult);
	}
}

// Attempts to read a response and if no error then return either 
//		--> the number of bytes received if data was received
//		--> 0 if no error but no data available withint the 1000ms timeout
//		--> -1 if there was an error (meaning buffer will be invalid) 
int		GetTCPMessage()
{
	struct pollfd fd;
	int ret;

	memset(recvbuf, 0, sizeof(recvbuf));	// Clear the buffer each time before trying to read data into it.

	fd.fd = ConnectSocket; // your socket handler 
	fd.events = POLLIN;
	ret = WSAPoll(&fd, 1, 1000); // 1 second for timeout

	switch (ret)
	{
	case -1:
		// Error
		return(-1);  // tell caller there was an error

	case 0:
		// Timeout 
		return(0);  // tell caller there was an error

	default:
		recv(ConnectSocket, recvbuf, sizeof(recvbuf), 0); // get your data

		return(strlen(recvbuf));
	}


}
