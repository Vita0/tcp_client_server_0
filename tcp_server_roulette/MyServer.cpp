#include <thread>
#include <chrono>

#include "MyServer.h"

MyServer::MyServer(const char* ip, u_short port)
    :m_started(false)
{
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
        throw 1;
    }
    
    m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listen == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        //WSACleanup();
        throw 2;
    }
    
    //nonblocking
    //u_int nb = 1; //must be u_long 
    //int res = ioctlsocket(m_listen, FIONBIO, &nb);
    //if (res != NO_ERROR)
    //    wprintf(L"ioctlsocket failed with error: %ld\n", res);
    
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);//0 - is any
    
    if (bind(m_listen, (SOCKADDR *) & service, sizeof (service))
            == SOCKET_ERROR) {
        wprintf(L"bind failed with error: %ld\n", WSAGetLastError());
        closesocket(m_listen);
        //WSACleanup();
        throw 3;
    }
    
    if (listen(m_listen, 1) == SOCKET_ERROR) {
        wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
        closesocket(m_listen);
        //WSACleanup();
        throw 4;
    }
}

MyServer::~MyServer()
{
    WSACleanup();
}

void MyServer::start()
{
    m_started = true;
    m_accept_thread = shared_ptr<thread>( new thread(&MyServer::myAccept, this) );
    m_comands_thread = shared_ptr<thread>( new thread(&MyServer::getCommands, this) );
    m_comands_thread->join();
    m_accept_thread->join();
}

void MyServer::myAccept()
{
    while (m_started)
    {
        SOCKET ac_sock;
        wprintf(L"Waiting for client to connect...\n");
        
        //nonblocking
        //u_int nb = 1;
        //int res = ioctlsocket(ac_sock, FIONBIO, &nb);
        //if (res != NO_ERROR)
        //    wprintf(L"ioctlsocket failed with error: %ld\n", res);
        
        this_thread::sleep_for(chrono::microseconds(1000));
        
        cout << "accept now ...";
        ac_sock = accept(m_listen, NULL, NULL);
        cout << "accept ok!" << endl;
        if (ac_sock == INVALID_SOCKET) {
            wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
            closesocket(m_listen);
            //WSACleanup();
            break;
        } if (ac_sock == WSAEWOULDBLOCK) {
            continue;
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
            cout << "player id " << ac_sock << endl;
            m_clients_mutex.lock();
            m_clients.insert(make_pair(ac_sock, Player(ac_sock, &m_game, &m_game_mutex)));
            m_clients.at(ac_sock).start();
            m_clients_mutex.unlock();
        }
    }
    m_clients_mutex.lock();
    auto end = m_clients.end();
    for(auto i=m_clients.begin(); i != end; ++i)
    {
        m_cout_mutex.lock();
        cout << "myAccept: client" << i->first << " stoping ... ";
        m_cout_mutex.unlock();
        
        i->second.stop();
        
        m_cout_mutex.lock();
        cout << "ok!" << endl;
        m_cout_mutex.unlock();
        
        int iResult = shutdown(i->first, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"shutdown failed: %d\n", WSAGetLastError());
            closesocket(i->first);
        }
        closesocket(i->first);
    }
    m_clients_mutex.unlock();
}

void MyServer::getCommands()
{
    string s;
    while (m_started)
    {
        s = "";
        cout << "wait command..." << endl;
        getline(cin, s);
        cout << "wait command...ok!" << endl;
        if (s.substr(0,4) == "kill")
        {
            int a = atoi(s.substr(4,6).c_str());
            m_clients_mutex.lock();
            auto it = m_clients.find(a);
            if (it != m_clients.end())
            {
                m_game_mutex.lock();
                m_game.delPlayer(a);
                m_game_mutex.unlock();
                m_clients.at(a).stop();
            }
            else
            {
                cout << "bad command" << endl;
            }
            m_clients_mutex.unlock();
        }
        else if (s == "stop")
        {
            m_started = false;
            closesocket(m_listen);
        }
    }
}