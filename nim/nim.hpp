#pragma once
#define MAX_NAME		 80
#define MAX_MESSAGE		 80
#define MAX_SERVERS      100
#define DEFAULT_BUFLEN   512
#define DEFAULT_PORT     29333
#define Game_QUERY      "Who?"
#define Server_NAME	    "Name="
#define Player_NAME		"Player="



struct ServerStruct {
	char name[MAX_NAME];
	sockaddr_in addr;
};

int getServers(SOCKET s, ServerStruct server[]);
int wait(SOCKET s, int seconds, int msec);
sockaddr_in GetBroadcastAddress(char* IPAddress, char* subnetMask);
sockaddr_in GetBroadcastAddressAlternate(char* IPAddress, char* subnetMask);