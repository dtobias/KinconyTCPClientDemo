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


#include	<windows.h>
#include	<stdio.h>
#include	"winsockTCP.h"
#include	"KinconyTCPRelay.h"
#include	"string.h"


#define		STARTUP_MSG1	"RELAY-SCAN_DEVICE-NOW"
#define		MSG1_RESPONSE	"RELAY-SCAN_DEVICE-CHANNEL_32,OK"

#define		STARTUP_MSG2	"RELAY-TEST-NOW"
#define		MSG2_RESPONSE	"HOST-TEST-START"

#define		RELAY_SETSTATE	"RELAY-SET-255"
#define		RELAY_GETSTATE	"RELAY-READ-255"

#define		RELAY_GETINPUTS	"RELAY-GET_INPUT-255"

#define		RC_OK			"OK"
#define		RC_ERROR		"ERROR"

#define		MIN_CMD_DLY		0

#define		BADSTARTUP1		1
#define		BADSTARTUP2		2
#define		BAD_RC			3

void InitKinconyAfterPowerUp(void)
{
	Sleep(MIN_CMD_DLY);

	sprintf_s(sendbuf, sizeof(sendbuf), STARTUP_MSG1);
	printf("%s%+*s", sendbuf, 30 - (int)strlen(sendbuf), " --> ");

	SendTCPMessage();
	GetTCPMessage();			// This autoclears buffer before receiving

	printf("%s\n", recvbuf);

	int result1 = strcmp(recvbuf, MSG1_RESPONSE);
	if (result1 == 0)		// First command is finished and response was OK so now send 2nd cold power up command
	{
		sprintf_s(sendbuf, sizeof(sendbuf), STARTUP_MSG2);
		printf("%s%+*s", sendbuf, 30 - (int)strlen(sendbuf), " --> ");

		SendTCPMessage();
		GetTCPMessage();		// This autoclears buffer before receiving

		printf("%s\n", recvbuf);
		Sleep(MIN_CMD_DLY);

		int result2 = strcmp(recvbuf, MSG2_RESPONSE);
		if (result2 != 0)		// See if 2nd command response was OK.  If not exit with error code.
		{
			exit(BADSTARTUP2);		// Bail out with result code 2
		}
	}
	else
	{
		exit(BADSTARTUP1);		// Bail out with result code 1
	}
}


int	SetKinconyRelayState(int	RelayNumber, int State)
{
	sprintf_s(sendbuf, sizeof(sendbuf), "%s,%d,%d      ", RELAY_SETSTATE, RelayNumber, State);
	printf("%s%+*s", sendbuf, 30 - (int)strlen(sendbuf), " --> ");

	SendTCPMessage();
	GetTCPMessage();

	printf("%s\n", recvbuf);

	Sleep(MIN_CMD_DLY);

	return(0);
}


int	GetKinconyRelayState(int RelayNumber)
{
	int		RelayState;
	   
	const char* delim		= ",";
	char* remainder_str		= NULL;
	char* echoed_command	= NULL;
	char* relay_num_str		= NULL;
	char* relay_state_str	= NULL;
	char* cmd_result_str	= NULL;

	sprintf_s(sendbuf, sizeof(sendbuf), "%s,%d      ", RELAY_GETSTATE, RelayNumber);
	printf("%s%+*s", sendbuf, 30 - (int)strlen(sendbuf), " --> ");

	SendTCPMessage();
	GetTCPMessage();

	printf("%s\n", recvbuf);

	// You can think of the string getting shorter each time by cutting off everything up to and including the delimiter(s)
	echoed_command = strtok_s(recvbuf, delim, &remainder_str);

	// The second token is the relay whose state is being read
	relay_num_str = strtok_s(remainder_str, delim, &remainder_str);

	// The third token is the state of the relay, should be zero or 1.
	relay_state_str = strtok_s(remainder_str, delim, &remainder_str);

	// The 4th token is the reult of the command whether OK or not.
	cmd_result_str = strtok_s(remainder_str, delim, &remainder_str);
   	
	if (strcmp(cmd_result_str, RC_OK) == 0)
	{
		RelayState = atoi(relay_state_str);
	}
	else // result string was not OK
	{
		exit(BAD_RC);
	}

	return(RelayState);
}


BYTE	GetKinconyInputPort(void)
{
	BYTE			InputPortState;

	const char*		delim = ",";
	char*			remainder_str = NULL;
	char*			echoed_command = NULL;
	char*			input_byte_str = NULL;
	char*			cmd_result_str = NULL;

	sprintf_s(sendbuf, sizeof(sendbuf), "%s", RELAY_GETINPUTS);
	printf("%s%+*s", sendbuf, 30 - (int)strlen(sendbuf), " --> ");

	SendTCPMessage();
	GetTCPMessage();

	printf("%s\n", recvbuf);

	// You can think of the string getting shorter each time by cutting off everything up to and including the delimiter(s)
	echoed_command = strtok_s(recvbuf, delim, &remainder_str);

	// The 2nd token is the state of the inputs, should be a number 0..255.
	input_byte_str = strtok_s(remainder_str, delim, &remainder_str);

	// The 3rd token is the reult of the command whether OK or not.
	cmd_result_str = strtok_s(remainder_str, delim, &remainder_str);

	if (strcmp(cmd_result_str, RC_OK) == 0)
	{
		InputPortState = (BYTE)atoi(input_byte_str);
	}
	else // result string was not OK
	{
		exit(BAD_RC);
	}

	return(InputPortState);
}