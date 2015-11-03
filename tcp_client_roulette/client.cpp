/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 11:00
 */

#pragma comment(lib,"wsock32.lib")

#include "ClientSidePlayer.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
WSADATA wsaData;
    // The WSAStartup function initiates use of the Winsock DLL by a process.
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
        return 1;
    }
    //----------------------
    // Create a SOCKET for listening for
    // incoming connection requests.
    SOCKET Socket;
    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    struct sockaddr_in clientService; 
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    clientService.sin_port = htons( 27015 );

    //----------------------
    // Connect to server.
    iResult = connect( Socket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (Socket);
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    
    ClientSidePlayer player(Socket);
    player.start();
    
    
    
    // shutdown the connection since no more data will be sent
    iResult = shutdown(Socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return 1;
    }
    
    // cleanup
    closesocket(Socket);
    WSACleanup();
    
    return 0;
}

