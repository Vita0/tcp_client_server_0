#include "MyServer.h"
#include <string>

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
    //closesocket(m_listen);
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
        cout << "ac lol" << endl;
        ac_sock = accept(m_listen, NULL, NULL);
        cout << "ac lol 2" << endl;
        if (ac_sock == INVALID_SOCKET) {
            wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
            //closesocket(m_listen);
            //WSACleanup();
            break;
            //throw 5;
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
            
            ths_mutex.lock();
            //ths_for_del.push(ac_sock);
            ths.insert(make_pair(ac_sock, shared_ptr<thread>(new thread(&MyServer::exchange,this, ac_sock))));
            //ths.at(ac_sock)->detach();
            ths_mutex.unlock();
        }
    }
    auto end = ths.end();
    for(auto i=ths.begin(); i != end; ++i)
    {
        cout << "join: " << i->first << " ... " << endl;
        i->second->join();
        closesocket(i->first);
    }
}

void MyServer::exchange(SOCKET sock)
{
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN+1] = "";
    
    while (1)
    {
        std::fill(recvbuf, recvbuf + DEFAULT_BUFLEN, '\0');

        int iResult = readn(sock, recvbuf, recvbuflen);
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
        
//        for(int j = 0; j<10; ++j)
//        {
//            std::this_thread::__sleep_for(std::chrono::seconds(1), std::chrono::nanoseconds(0));
            cout << "thread " << sock << " sleeping " << /*j <<*/ "s" << endl;
//        }
        bool kill = false;
        ths_mutex.lock();
        if (ths_for_del.front() == sock || m_stop)
        {
            kill = true;
            sprintf(recvbuf,"secret");
        }
        ths_mutex.unlock();
        //----------------------
        // Send an initial buffer
        iResult = send( sock, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            //WSACleanup();
            throw 6;
        }
        if (kill) break;
    }
    cout << "shutdown " << sock << endl;
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(sock, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(sock);
        //WSACleanup();
        throw 7;
    }
    closesocket(sock);
    
    if (!m_stop){
        ths_mutex.lock();
        ths_for_del.pop();
        ths_mutex.unlock();
    }
    else
    {
        ths_mutex.lock();
        //ths.erase(sock);
        ths_mutex.unlock();
    }
    cout << "num of clients: " << ths.size() << endl;
        
    //ths.erase(sock);
    
}

void MyServer::getCommands()
{
    string s;
    while (!m_stop)
    {
        s = "";
        getline(cin, s);
        if (s.substr(0,4) == "kill")
        {
            cout << s.substr(4,6) << endl;
            int a = atoi(s.substr(4,6).c_str());
            ths_mutex.lock();
            auto it = ths.find(a);
            if (it != ths.end())
            {
                ths_for_del.push(SOCKET(a));
            }
            else
            {
                cout << "bad command" << endl;
            }
            ths_mutex.unlock();
        }
        else if (s == "stop")
        {
            m_stop = true;
            closesocket(m_listen);
        }
    }
}

void MyServer::start()
{
    m_stop = false;
    acc = shared_ptr<thread>( new thread(&MyServer::my_accept, this) );
    comands = shared_ptr<thread>( new thread(&MyServer::getCommands, this) );
    comands->join();
    cout << "lollollol" << endl;
    acc->join();
    cout << "sdfsdfsdgdfgysrtygdthd" << endl;
}