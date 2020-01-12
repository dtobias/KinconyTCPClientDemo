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

#include	<stdio.h>
#include	<Windows.h>
#include	<string.h>
#include	<conio.h>
#include	<stdio.h>
#include	"winsockTCP.h"
#include	"kinconyTCPrelay.h"

//#define		PORT	8888				//The port on which to listen for incoming data when testing with UDPServer.exe
#define			PORT	4196				// Kincony relay controller seems to like this

char	KinconyRelayServer[] = "192.168.1.50";
char	KinconyRelayPort[] = "4196";

int		RelayState;
int		ActualState;



bool isShift()
{  // Check if shift is pressed
	if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) // If the high-order bit is 1, the key is down; otherwise, it is up.
		return true;
	else
		return false;
}


int main(void)
{
	int		RelayState;

	OpenTCPSocket();

	OpenTCPConnection(KinconyRelayServer, KinconyRelayPort);


	InitKinconyAfterPowerUp();
	while (!isShift())
	{
		for (int Relay = 1; Relay <= 32; Relay++)
		{
			SetKinconyRelayState(Relay, RELAY_ON);
			ActualState = GetKinconyRelayState(Relay);
			if (ActualState != RELAY_ON)
			{
				printf("Expected relay state was %d, actual was %d", RelayState, ActualState);
				Sleep(10000);
			}
		}

		for (int Relay = 1; Relay <= 32; Relay++)
		{
			SetKinconyRelayState(Relay, RELAY_OFF);
			ActualState = GetKinconyRelayState(Relay);
			if (ActualState != RELAY_OFF)
			{
				printf("Expected relay state was %d, actual was %d", RelayState, ActualState);
				Sleep(10000);
			}
		}

		for (int Relay = 1; Relay <= 32; Relay++)
		{
			for (RelayState = RELAY_ON; RelayState >= 0; RelayState--)
			{
				SetKinconyRelayState(Relay, RelayState);
				ActualState = GetKinconyRelayState(Relay);
				if (ActualState != RelayState)
				{
					printf("Expected relay state was %d, actual was %d", RelayState, ActualState);
					Sleep(10000);
				}
			}
		}

		// These bits are pulled up and so without connection to them you will read back 255.
		// Any bits that are grounded will return 0.
		printf("Input port reads ==> %d", GetKinconyInputPort());

	}

	CloseTCPConnection();


	CloseTCPSocket();
}