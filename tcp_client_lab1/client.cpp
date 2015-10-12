/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 29 сентября 2015 г., 11:35
 */
#include <cstdlib>
#include <iostream>
#include <Winsock2.h>
#include <stdio.h>
#include <windows.h>

//#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"wsock32.lib")

#define DEFAULT_BUFLEN 7

using namespace std;

int readn(SOCKET fd, char *data, size_t data_len)
{
    int cnt;
    int res;
    
    cnt = data_len;
    while( cnt > 0 ) {
        res = recv(fd, data, cnt, 0);
        if ( res < 0 )
        {
            if ( errno == EINTR )
                continue;
            wprintf(L"recv failed: %d\n", WSAGetLastError());
            return -1;
        }
        if ( res == 0 )
            return data_len - cnt;
        data += res;
        cnt -= res;
    }
    return data_len;
}

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
    
    
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN] = "";
    // Send an initial buffer
    do {
        char sendbuf[DEFAULT_BUFLEN+1];
        std::fill(sendbuf, sendbuf + DEFAULT_BUFLEN, '\0');
        cin.getline(sendbuf,DEFAULT_BUFLEN);
        iResult = send( Socket, sendbuf, DEFAULT_BUFLEN, 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(Socket);
            WSACleanup();
            return 1;
        }
        printf("Bytes Sent: %ld\n", iResult);
        
        if (strcmp(sendbuf,"secret") == 0) {
            break;
        }
        
        std::fill(recvbuf, recvbuf + DEFAULT_BUFLEN, '\0');
        iResult = readn(Socket, recvbuf, recvbuflen);
        if ( iResult > 0 ) {
            printf("%s\n",recvbuf);
            wprintf(L"Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 )
            wprintf(L"Connection closed\n");
        else
            wprintf(L"recv failed: %d\n", WSAGetLastError());
        
        if (strcmp(recvbuf,"secret") == 0) {
            cout << "server say 'secret'" << endl;
            break;
        }
    } while ( iResult > 0 );
    
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
    cout << "lol" << endl;
    return 0;
}

