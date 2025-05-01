#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <iphlpapi.h>
#include "nim.hpp"
#include <ctime>

using std::cin;
using std::cout;
using std::endl;
#pragma comment (lib, "iphlpapi.lib")

int getServers(SOCKET s, ServerStruct server[])
{
	// Parameter List:
	// [IN] s				 : SOCKET handle 
	// [OUT] server			 : Array in which all the available servers' information will be saved
	// Return value			 : The number of currently available servers
	int numServers = 0;
	char IPAddress[20] = "10.35.82.114", subnetMask[20] = "255.255.240.0";

	// Necessary to be able to send a broadcast message
	BOOL bOptVal = TRUE;
	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, sizeof(BOOL));

	sockaddr_in broadcastAddress = GetBroadcastAddress(IPAddress, subnetMask);

	int iResult = sendto(s, Game_QUERY, strlen(Game_QUERY) + 1, 0, (sockaddr*)&broadcastAddress, sizeof(broadcastAddress));
	char recvBuf[DEFAULT_BUFLEN];

	int status = wait(s, 2, 0);
	if (status > 0) {
		int len = 0;
		struct sockaddr_in addr;
		int addrSize = sizeof(addr);

		len = recvfrom(s, recvBuf, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &addrSize);
		while (status > 0 && len > 0) {
			if (strncmp(recvBuf, "Name=", 5) == 0)
				for (int i = 5; i < strlen(recvBuf) + 1; i++)
					server[numServers].name[i - 5] = recvBuf[i];

			server[numServers].addr = addr;
			numServers++;

			status = wait(s, 2, 0);
			if (status > 0)
				len = recvfrom(s, recvBuf, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &addrSize);
		}
	}
	return numServers;
}

int wait(SOCKET s, int seconds, int msec) {
	// Parameter List:
	// [IN] s		: SOCKET handle
	// [IN] seconds : Number of seconds to wait for incoming traffic
	// [IN] msec	: Number of milliseconds to wait
	// Return value : 1 = Data is waiting in TCP/UDP buffer; 0 = Timeout or error detected
	int    stat;					// Status of waiting socket
	struct timeval stTimeOut;		// Used to set timeout value
	fd_set stReadFDS;				// "File Descriptor Set" for select() call
	fd_set stXcptFDS;				// "File Descriptor Set" for exception handling

	// Set TimeOut value
	stTimeOut.tv_sec = seconds;	// Number of seconds
	stTimeOut.tv_usec = msec;		//   + msec milliseconds to wait

	FD_ZERO(&stReadFDS);			// Zero the set of "Read access" sockets
	FD_ZERO(&stXcptFDS);			// Zero the set of "Exception" sockets
	FD_SET(s, &stReadFDS);			// Add "s" to the set of "Read access" sockets
	FD_SET(s, &stXcptFDS);			// Add "s" to the set of "Exception" sockets

	// Check to see if Read access is enabled for socket "s"
	stat = select(-1, &stReadFDS, NULL, &stXcptFDS, &stTimeOut);
	if (stat == SOCKET_ERROR) {
		std::cout << std::endl << "wait() function failed" << std::endl;
		stat = 0;
	}
	else if (stat > 0) {
		if (FD_ISSET(s, &stXcptFDS))	// Some error was detected or OOB data
			stat = 0;
		if (!FD_ISSET(s, &stReadFDS)) 	// No incoming data!? (check just in case)
			stat = 0;
	}

	return stat;
}

sockaddr_in GetBroadcastAddress(char* ip, char* mask)
{
	// NOTE: This code only returns the broadcast address for the first adapter found.
	// Alternate function gets IP address and subnet mask from user to ensure valid
	// broadcast in the case that the first adapter retrieved has address 0.0.0.0
	const int MAXSIZE = 16;
	sockaddr_in addr{};
	unsigned long IPAddress = 0;
	unsigned long subnetMask = 0;
	unsigned long broadcastAddress = 0;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
	pAdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		std::cout << "Error allocating memory needed to call GetAdaptersinfo" << std::endl;
		return addr;
	}
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		// Release initial allocation of heap memory and request more appropriately sized memory
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
		if (pAdapterInfo == NULL) {
			std::cout << "Error allocating memory needed to call GetAdaptersinfo" << std::endl;
			return addr;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		if (pAdapter) {
			int iResult = inet_pton(AF_INET, pAdapter->IpAddressList.IpAddress.String, &IPAddress);
			iResult = inet_pton(AF_INET, pAdapter->IpAddressList.IpMask.String, &subnetMask);

			// Addresses returned from inet_pton are in network byte order, so they should 
			// be left alone. The port number needs to be converted to network byte order. 
			unsigned long BcastMask = subnetMask ^ 0xffffffff;
			broadcastAddress = IPAddress | BcastMask;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(DEFAULT_PORT);
			addr.sin_addr.S_un.S_addr = broadcastAddress;
		}
	}
	else {
		std::cout << "GetAdaptersInfo failed with error: " << dwRetVal << std::endl;
	}
	if (pAdapterInfo)
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);

	if (addr.sin_addr.S_un.S_addr == 4294967295) // If we computed 255.255.255.255 for the broadcast address
		return GetBroadcastAddressAlternate(ip, mask);
	else
		return addr;
}

sockaddr_in GetBroadcastAddressAlternate(char* IPAddress, char* subnetMask)
{
	unsigned long ip;
	unsigned long subnet;

	if (strlen(IPAddress) <= 1) {
		std::cout << "What is your IP address? ";
		std::cin.getline(IPAddress, 20);
		std::cout << "What is your subnet mask? ";
		std::cin.getline(subnetMask, 20);
	}

	int iResult = inet_pton(AF_INET, IPAddress, &ip);
	iResult = inet_pton(AF_INET, subnetMask, &subnet);

	// Addresses returned from inet_pton are in network byte order, so they should 
	// be left alone. The port number needs to be converted to network byte order. 
	unsigned long BcastMask = subnet ^ 0xffffffff;
	unsigned long broadcastAddress = ip | BcastMask;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(DEFAULT_PORT);
	addr.sin_addr.S_un.S_addr = broadcastAddress;

	return addr;
}