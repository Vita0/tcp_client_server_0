#include "MyServer.h"

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

MyServer::MyServer(const char* ip, u_short port)
    :m_stop(true)
{
    WSADATA wsaData;
    m_acs.resize(0);
    // The WSAStartup function initiates use of the Winsock DLL by a process.
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
        throw 1;
    }
    //----------------------
    // Create a SOCKET for listening for
    // incoming connection requests.
    m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listen == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        throw 2;
    }
    u_int nb = 1;
    int res = ioctlsocket(m_listen, FIONBIO, &nb);
    
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);//0 - is any
    
    if (bind(m_listen, (SOCKADDR *) & service, sizeof (service))
            == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(m_listen);
        WSACleanup();
        throw 3;
    }
    //----------------------
    // Listen for incoming connection requests.
    // on the created socket
    if (listen(m_listen, 1) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        closesocket(m_listen);
        WSACleanup();
        throw 4;
    }
}

MyServer::~MyServer()
{
    // No longer need server socket
    closesocket(m_listen);
    int end = m_acs.size();
    for(int i=0; i < end; ++i)
    {
        closesocket(m_acs[i]);
    }
    WSACleanup();
}

void MyServer::my_accept()
{
    while (!m_stop)
    {
        //----------------------
        // Create a SOCKET for accepting incoming requests.
        SOCKET ac_sock;
        wprintf(L"Waiting for client to connect...\n");
        u_int nb = 1;
        int res = ioctlsocket(ac_sock, FIONBIO, &nb);
        //----------------------
        // Accept the connection.
        ac_sock = accept(m_listen, NULL, NULL);
        if (ac_sock == INVALID_SOCKET) {
            wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
            closesocket(m_listen);
            WSACleanup();
            throw 5;
        } else {
            sockaddr_in ac_service;
            int len = sizeof(ac_service);
            if (getsockname(ac_sock, (SOCKADDR* ) &ac_service, &len)
                    == SOCKET_ERROR) {
                wprintf(L"getsocketname failed with error: %ld\n", WSAGetLastError());
            }
            sockaddr_in cl_service;
            int cl_len = sizeof(cl_service);
            if (getpeername(ac_sock, (SOCKADDR* ) &cl_service, &cl_len)
                    == SOCKET_ERROR) {
                wprintf(L"getsocketname failed with error: %ld\n", WSAGetLastError());
            }
            wprintf(L"Client connected:\n");
            wprintf(L"  Client IP:PORT - %s:", inet_ntoa((in_addr) cl_service.sin_addr));
            wprintf(L"%d\n", cl_service.sin_port);
            wprintf(L"  Server create new socket for client, that IP:PORT - %s:", inet_ntoa((in_addr) ac_service.sin_addr));
            wprintf(L"%d\n", ac_service.sin_port);
            m_acs.push_back(ac_sock);
        }
    }
}

void MyServer::exchange()
{
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN+1] = "";
    
    while (!m_stop)
    {
        int end = m_acs.size();
        bool secret = false;
        int i;
        for(i=0; i < end; ++i)
        {
            std::fill(recvbuf, recvbuf + DEFAULT_BUFLEN, '\0');

            int iResult = readn(m_acs[i], recvbuf, recvbuflen);
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
                secret = true;
                break;
            }

            //----------------------
            // Send an initial buffer
            iResult = send( m_acs[i], recvbuf, DEFAULT_BUFLEN, 0);
            if (iResult == SOCKET_ERROR) {
                wprintf(L"send failed with error: %d\n", WSAGetLastError());
                closesocket(m_acs[i]);
                WSACleanup();
                throw 6;
            }
        }
        if (secret)
        {
            // shutdown the connection since no more data will be sent
            int iResult = shutdown(m_acs[i], SD_SEND);
            if (iResult == SOCKET_ERROR) {
                printf("shutdown failed: %d\n", WSAGetLastError());
                closesocket(m_acs[i]);
                WSACleanup();
                throw 7;
            }
            closesocket(m_acs[i]);
            vector<SOCKET>::const_iterator for_del = m_acs.begin()+i;
            m_acs.erase(for_del);
        }
    }
}

void MyServer::start()
{
    m_stop = false;
    acc = shared_ptr<thread>( new thread(&MyServer::my_accept, this) );
    run = shared_ptr<thread>( new thread(&MyServer::exchange, this) );
    acc->join();
    run->join();
}