

#pragma once
#include <iostream>       
#include <ws2tcpip.h>     // Windows sockets (TCP/IP functions)
#include <WinSock2.h>     // Windows sockets main header
#include<cctype>
#include <cstring>
#include "nim.hpp"
#include <string>

#define _CRT_SECURE_NO_WARNINGS

using std::cin;
using std::cout;
using std::endl;
using std::string;
int mainClient();




int server() {
    // ---------------------------
    // 1. Initialize Winsock (Windows Socket API)
    // ---------------------------
    WSADATA wsaData;  // Structure to store socket implementation details
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);      // Variable to store function return values
    // Check if initialization failed
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << endl;
        WSACleanup();  // Cleanup resources
        return 1;      // Exit with error
    }


    SOCKET GameSocket = INVALID_SOCKET;
    GameSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (GameSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    // Data in the sin_port and sin_addr members of the sockaddr_in struct need to be in network byte order.
    struct sockaddr_in myAddr;
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(DEFAULT_PORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(GameSocket, (SOCKADDR*)&myAddr, sizeof(myAddr));
    if (iResult == SOCKET_ERROR) {
        cout << "bind failed with error: " << WSAGetLastError() << "\n";
        closesocket(GameSocket);
        WSACleanup();
        return 1;
    }

    // ---------------------------
    // Receive & send messages
    // ---------------------------

    char recvbuf[DEFAULT_BUFLEN];  // Buffer for received data
    char sendbuf[DEFAULT_BUFLEN];  // Buffer for data to send

    // Sender Address
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    char server_name[100];

    cout << "Enter Name: \n";
    cin.getline(server_name, sizeof(server_name));

    while (true) {
        // Receive message from client
        iResult = recvfrom(GameSocket, recvbuf, DEFAULT_BUFLEN - 1, 0,
            (struct sockaddr*)&senderAddr, &senderAddrSize);

        if (iResult == SOCKET_ERROR) {  // Check receive failure
            cout << "Recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }

        if (iResult > 0) {  // Success
            recvbuf[iResult] = '\0'; // Ensure null termination
            cout << "Received: " << recvbuf << "\n";


           
            if (strcmp(recvbuf, Game_QUERY) == 0) {    // Who?
                // Format: "Name=servername"
                strcpy_s(sendbuf, DEFAULT_BUFLEN, Server_NAME);
                strcat_s(sendbuf, DEFAULT_BUFLEN, server_name);

            }
            else if (strcmp(recvbuf, Study_WHERE) == 0) {
                // Format: "Loc=location"
                strcpy_s(sendbuf, DEFAULT_BUFLEN, Study_LOC);
                strcat_s(sendbuf, DEFAULT_BUFLEN, group.loc);
            }
            else if (strcmp(recvbuf, Study_WHAT) == 0) {
                // Format: "Courses=course1\ncourse2\n..."
                strcpy_s(sendbuf, DEFAULT_BUFLEN, Study_COURSES);

                // Add all courses with newline delimiters
                for (int i = 0; i < group.coursesCounter; i++) {
                    strcat_s(sendbuf, DEFAULT_BUFLEN, group.courses[i]);
                    strcat_s(sendbuf, DEFAULT_BUFLEN, "\n");
                }
            }
            else if (strcmp(recvbuf, Study_MEMBERS) == 0) {

                if (group.membersCounter < 1) {
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, "No members.");

                }
                else {
                    // Format: "Members=member1\nmember2\n..."
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, Study_MEMLIST);
                    // Add all members with newline delimiters
                    for (int i = 0; i < group.membersCounter; i++) {
                        strcat_s(sendbuf, DEFAULT_BUFLEN, group.members[i]);
                        strcat_s(sendbuf, DEFAULT_BUFLEN, "\n");
                    }
                }

            }
            else if (strncmp(recvbuf, Study_JOIN, 5) == 0) {
                char* clientName = recvbuf + 5; // skips the first 5 characters

                // Add client to members list
                strcpy_s(group.members[group.membersCounter], sizeof(group.members[group.membersCounter]), clientName);
                group.membersCounter++;

                cout << "User '" << clientName << "' joined the group\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, Study_CONFIRM);
            }
            else if (strcmp(recvbuf, "Exit") == 0) {
                cout << "Client requested to exit\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Goodbye!");
            }
            else {
                // Handle general chat messages
                cout << "Chat message: " << recvbuf << "\n";
                strcpy_s(sendbuf, DEFAULT_BUFLEN, "Message received");
            }
            cout << "Sent: " << sendbuf << "\n";

            // Send response to client
            iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0,
                (struct sockaddr*)&senderAddr, senderAddrSize);

            if (iResult == SOCKET_ERROR) {  // Check send failure
                cout << "Send failed: " << WSAGetLastError() << endl;
                continue;
            }
        }
        else if (iResult == 0) {  // Connection closed
            cout << "Connection closed by client" << endl;
            continue;
        }
    }

    // ---------------------------
    // 5. Cleanup & Disconnect
    // ---------------------------
    closesocket(GameSocket);  // Close socket
    WSACleanup();              // Cleanup Winsock

    return 0;  // Exit successfully
}

int main() {
    int action = 0;

    do {
        cout << "Choose an option: (1) Join | (2) Host | (3) Exit\n";
        cin >> action;

        if (action == 1) {
            if (mainClient() == 3) {
                return 0;
            }
        }
        else if (action == 2) {
            mainHost();
        }
        else if (action == 3) {
            cout << "Program Exited";
        }
        else {
            cout << "Invalid option. Please try again.\n";
        }

    } while (action != 3);

    return 0;
}