

#pragma once
#include <iostream>       
#include <ws2tcpip.h>     // Windows sockets (TCP/IP functions)
#include <WinSock2.h>     // Windows sockets main header
#include<cctype>
#include <cstring>
#include "nim.hpp"
#include "nim_game.cpp"
#include <string>

#define _CRT_SECURE_NO_WARNINGS

using std::cin;
using std::cout;
using std::endl;
using std::string;


int client() {
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (iResult != 0) {
        cout << "WSAStartup() failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }

    // Create Socket
    SOCKET GameSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (GameSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Enter name
    char client_name[100];
    cout << "Enter name:" << endl;
    cin.getline(client_name, sizeof(client_name));

    // Enter loop to allow for multiple challenges if unsuccessful
    bool challenging = true;
    while (challenging) {
        challenging = false;

        // Discover servers (send 'Who?')
        ServerStruct servers[MAX_SERVERS];
        int availableServers = getServers(GameSocket, servers);

        cout << endl << "Servers available:" << endl;
        if (availableServers > 0) {
            for (int i = 0; i < availableServers; i++) {
                cout << (i + 1) << " - " << servers[i].name << endl;
            }
        }
        else {
            cout << "Could not find servers." << endl;
            closesocket(GameSocket);
            WSACleanup();
            return 1;
        }

        // Choose server to challenge
        bool validChoice = false;
        int serverChoice;
        while (!validChoice) {
            cout << "Choose server number to challenge:" << endl;
            cin >> serverChoice;

            if (serverChoice < 1 || serverChoice > availableServers) {
                cout << "Invalid choice." << endl;
                cout << "Please select a number between 1 and " << availableServers << "." << endl;
            }
            else {
                validChoice = true;
            }
        }

        struct sockaddr_in serverAddr = servers[serverChoice - 1].addr;
        int serverAddrSize = sizeof(serverAddr);
        serverAddr.sin_port = htons(DEFAULT_PORT);
        
        char sendbuf[DEFAULT_BUFLEN];
        char recvbuf[DEFAULT_BUFLEN];

        // Construct 'Player=' message
        strcpy_s(sendbuf, DEFAULT_BUFLEN, Player_NAME);
        strcat_s(sendbuf, DEFAULT_BUFLEN, client_name);

        // Send challenge to server
        iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0, (sockaddr*)&serverAddr, serverAddrSize);
        if (iResult == SOCKET_ERROR) {
            cout << "sendto() failed: " << WSAGetLastError() << endl;
            closesocket(GameSocket);
            WSACleanup();
            return 1;
        }
        cout << "Server challenged." << endl;

        // wait for server response
        int waitResult = wait(GameSocket, 10, 0);
        if (waitResult == 0) {
            cout << "Server was not ready." << endl;
            closesocket(GameSocket);
            WSACleanup();
            return 1;
        }

        // Recieve response from server
        iResult = recvfrom(GameSocket, recvbuf, DEFAULT_BUFLEN - 1, 0, (sockaddr*)&serverAddr, &serverAddrSize);
        if (iResult == SOCKET_ERROR) {
            cout << "recvfrom() failed: " << WSAGetLastError() << endl;
            closesocket(GameSocket);
            WSACleanup();
            return 1;
        }

        recvbuf[iResult] = '\0'; // add null terminator

        cout << "Server responded: " << recvbuf << endl;

        //parse response
        if (_stricmp(recvbuf, "NO") == 0) {
            cout << "Server declined challenge." << endl;
            cout << "Would you like to (1) Challenge another server, or (2) quit." << endl;
            int quitSelect = 0;
            cin >> quitSelect;

            if (quitSelect == 1) {
                challenging = true;
            }
            else {
                if (quitSelect != 2) { cout << "Invalid input. "; }
                cout << "Quitting..." << endl;
                closesocket(GameSocket);
                WSACleanup();
                return 1;
            }
        }
        else if (_stricmp(recvbuf, "YES") == 0) {
            cout << "Server accepted challenge." << endl;

            // send 'GREAT!'
            strcpy_s(sendbuf, DEFAULT_BUFLEN, "GREAT!");
            iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0, (sockaddr*)&serverAddr, serverAddrSize);
            if (iResult == SOCKET_ERROR) {
                cout << "sendto() failed: " << WSAGetLastError() << endl;
                closesocket(GameSocket);
                WSACleanup();
                return 1;
            }
            cout << "Sent 'GREAT!' to server." << endl;

            // Negotiation complete, start game
            cout << "Negotiation complete. Game starting now..." << endl;
            startGame();
        }
        else {
            cout << "Invalid response from server." << endl;
            closesocket(GameSocket);
            WSACleanup();
            return 1;
        }
    }

    // Close socket and cleanup
    closesocket(GameSocket);
    WSACleanup();

    return 0;

}


void startGame();

int server() {
    char server_name[100];

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

    // ---------------------------
    // 2. Create connection socket
    // ---------------------------

    SOCKET GameSocket = INVALID_SOCKET;
    GameSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (GameSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }


    // -------------------------
    // 3. Get input from user and discover
    // --------------------------
    ServerStruct servers[MAX_SERVERS];


    cout << "Enter Name: \n";
    cin.getline(server_name, sizeof(server_name));

    // Broadcast "Who?"
    int availableServers = getServers(GameSocket, servers);
    cout << "\nServers available: \n";

    if (availableServers > 0) {
        for (int i = 0; i < availableServers; i++) {
            cout << (i + 1) << " - " << servers[i].name << "\n";
        }
    }
    else {
        cout << "No servers available\n";
        closesocket(GameSocket);
        WSACleanup();
        return 1;
    }

    int serverInt;
    cout << "Choose Server #\n> ";
    cin >> serverInt;


    // Invalid server number
    if (serverInt > availableServers || serverInt < 1) {
        cout << "Invalid Server\n";
        closesocket(GameSocket);
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
    // 4. Receive & send messages
    // ---------------------------

    char recvbuf[DEFAULT_BUFLEN];  // Buffer for received data
    char sendbuf[DEFAULT_BUFLEN];  // Buffer for data to send

    // Sender Address
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    bool checkWait = false;


    // Wait for response

    while (true) {

        // Receive message from client
        iResult = recvfrom(GameSocket, recvbuf, DEFAULT_BUFLEN - 1, 0,
            (struct sockaddr*)&senderAddr, &senderAddrSize);

        if (iResult == SOCKET_ERROR) {  // Check receive failure
            cout << "Recvfrom failed: " << WSAGetLastError() << endl;
            continue;
        }


        if (iResult > 0 ) {  // Success
            recvbuf[iResult] = '\0'; // Ensure null termination
            cout << "Received: " << recvbuf << "\n";


           
            if (strcmp(recvbuf, Game_QUERY) == 0) {    // Who?
                // Format: "Name=servername"
                strcpy_s(sendbuf, DEFAULT_BUFLEN, Server_NAME);
                strcat_s(sendbuf, DEFAULT_BUFLEN, server_name);

                // Send response to client
                iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0,
                    (struct sockaddr*)&senderAddr, senderAddrSize);
                cout << "Sent: " << sendbuf << "\n";

                continue;

            }
            else if (strncmp(recvbuf, Player_NAME, 7) == 0) { // Player=client_name
                char* clientName = recvbuf + 7; // skips the first 5 characters
                int userResponse;

                // * user has been challenged * //
                
                cout << "---------------------------------------------------------------------------------------";
                cout << "You have been challenged to a game by: " << clientName <<"\n";
                cout << "---------------------------------------------------------------------------------------";

                cout << "Accept game:\n";
                cout << "1. YES\n";
                cout << "2. NO\n";

                cin >> userResponse;

                if (userResponse == 1) {
                    // * user accepted challenge * //
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, "YES");
                    

                    // Send response to client
                    iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0,
                        (struct sockaddr*)&senderAddr, senderAddrSize);

                    cout << "Sent: " << sendbuf << "\n";

                    // after "YES" wait 2 seconds
                    int ready = wait(GameSocket, 2, 0);
                    
                    if (ready == 0) {
                        // timeout
                        // User not ready to play...
                        cout << "No 'READY!' confirmation. \n";
                        continue;
                    }
                    else if (ready == 1) {
                        // Receive message from client
                        iResult = recvfrom(GameSocket, recvbuf, DEFAULT_BUFLEN - 1, 0,
                            (struct sockaddr*)&senderAddr, &senderAddrSize);

                        if (iResult == SOCKET_ERROR) {  // Check receive failure
                            cout << "Recvfrom failed: " << WSAGetLastError() << endl;
                            continue;
                        }
                        if (iResult > 1) {

                            recvbuf[iResult] = '\0'; // Ensure null termination
                            cout << "Received: " << recvbuf << "\n";

                            if (strcmp(recvbuf, "GREAT!") == 0) {
                               
                                // * End negotiations && start game* 
                                break;

                            }

                        }
                        else {
                            std::cerr << "recvfrom error after YES: " << WSAGetLastError() << "\n";
                            continue;
                        }
                    }




                }
                else if(userResponse == 2){
                    strcpy_s(sendbuf, DEFAULT_BUFLEN, "NO");
                    // Send response to client
                    iResult = sendto(GameSocket, sendbuf, strlen(sendbuf) + 1, 0,
                        (struct sockaddr*)&senderAddr, senderAddrSize);
                    cout << "Sent: " << sendbuf << "\n";

                    continue;
                }
                else {
                    cout << "Incorrect\n";
                    continue;
                }

            }
            
        }else if (iResult == 0) {  // Connection closed
            cout << "Connection closed by client" << endl;
            continue;
        }
    }


    // --------------------------------
    // Negotiation is over, Start Game
    // --------------------------------
    startGame();



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
            if (client() == 3) {
                return 0;
            }
        }
        else if (action == 2) {
            server();
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
