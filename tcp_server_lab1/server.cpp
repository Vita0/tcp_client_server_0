/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 20 сентября 2015 г., 0:06
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
    SOCKET ListenSocket;
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(27015);//0 - is any

    if (bind(ListenSocket, (SOCKADDR *) & service, sizeof (service))
            == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //----------------------
    // Listen for incoming connection requests.
    // on the created socket
    if (listen(ListenSocket, 1) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    //----------------------
    // Create a SOCKET for accepting incoming requests.
    SOCKET AcceptSocket;
    wprintf(L"Waiting for client to connect...\n");

    //----------------------
    // Accept the connection.
    AcceptSocket = accept(ListenSocket, NULL, NULL);
    if (AcceptSocket == INVALID_SOCKET) {
        wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    } else {
        sockaddr_in ac_service;
        int len = sizeof(ac_service);
        if (getsockname(AcceptSocket, (SOCKADDR* ) &ac_service, &len)
                == SOCKET_ERROR) {
            wprintf(L"getsocketname failed with error: %ld\n", WSAGetLastError());
        }
        sockaddr_in cl_service;
        int cl_len = sizeof(cl_service);
        if (getpeername(AcceptSocket, (SOCKADDR* ) &cl_service, &cl_len)
                == SOCKET_ERROR) {
            wprintf(L"getsocketname failed with error: %ld\n", WSAGetLastError());
        }
        wprintf(L"Client connected:\n");
        wprintf(L"  Client IP:PORT - %s:", inet_ntoa((in_addr) cl_service.sin_addr));
        wprintf(L"%d\n", cl_service.sin_port);
        wprintf(L"  Server create new socket for client, that IP:PORT - %s:", inet_ntoa((in_addr) ac_service.sin_addr));
        wprintf(L"%d\n", ac_service.sin_port);
    }
    
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN+1] = "";
    do {
        std::fill(recvbuf, recvbuf + DEFAULT_BUFLEN, '\0');
        
        iResult = readn(AcceptSocket, recvbuf, recvbuflen);
        if ( iResult > 0 ) {
            printf("%s\n",recvbuf);
            wprintf(L"Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 )
            wprintf(L"Connection closed\n");
        else
            wprintf(L"recv failed: %d\n", WSAGetLastError());
        
        if (strcmp(recvbuf,"secret") == 0) {
            cout << "client say 'secret'" << endl;
            break;
        }
        
        //----------------------
        // Send an initial buffer
        iResult = send( AcceptSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(AcceptSocket);
            WSACleanup();
            return 1;
        }
    } while( iResult > 0 );
    
    // shutdown the connection since no more data will be sent
    iResult = shutdown(AcceptSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(AcceptSocket);
        WSACleanup();
        return 1;
    }
    closesocket(AcceptSocket);

    // No longer need server socket
    closesocket(ListenSocket);
    

    WSACleanup();
    cout << "lol" << endl;
    return 0;
}

