#include <thread>
#include <chrono>

#include "MyServer.h"

int readn(crossSocket fd, char *data, size_t data_len)
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
            printf("recv failed: %d\n", 
#ifdef WINDOWS_OS
                    WSAGetLastError()
#else
                    errno
#endif
                    );
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
    :m_started(false)
    ,m_delStarted(false)
{
    int iResult;
#ifdef WINDOWS_OS
    WSADATA wsaData;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed with error: %ld\n", iResult);
        throw 1;
    }
#endif
    
    m_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listen == 
#ifdef WINDOWS_OS
            INVALID_SOCKET
#else
            -1
#endif
                    ) {
        printf("socket failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                WSAGetLastError()
#else
                                                errno
#endif
                );
        //WSACleanup();
        throw 2;
    }
    
    //nonblocking
    //u_int nb = 1; //must be u_long 
    //int res = ioctlsocket(m_listen, FIONBIO, &nb);
    //if (res != NO_ERROR)
    //    printf("ioctlsocket failed with error: %ld\n", res);
    
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);//0 - is any
    
    if (bind(m_listen, (
#ifdef WINDOWS_OS
                        SOCKADDR
#else
                        struct sockaddr
#endif
            *) & service, sizeof (service))
            == 
#ifdef WINDOWS_OS
                        SOCKET_ERROR
#else
                        -1
#endif
            ) {
        printf("bind failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                WSAGetLastError()
#else
                                                errno
#endif
                );
#ifdef WINDOWS_OS
        closesocket (m_listen);
#else
        close (m_listen);
#endif
        //WSACleanup();
        throw 3;
    }
    
    if (listen(m_listen, 1) == 
#ifdef WINDOWS_OS
                                SOCKET_ERROR
#else
                                -1
#endif
            ) {
        printf("listen failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                WSAGetLastError()
#else
                                                errno
#endif
                );
#ifdef WINDOWS_OS
        closesocket (m_listen);
#else
        close (m_listen);
#endif
        //WSACleanup();
        throw 4;
    }
}

MyServer::~MyServer()
{
#ifdef WINDOWS_OS
    WSACleanup();
#endif
}

void MyServer::start()
{
    m_started = true;
    m_delStarted = true;
    m_acceptThread = shared_ptr<thread>( new thread(&MyServer::myAccept, this) );
    m_comandsThread = shared_ptr<thread>( new thread(&MyServer::getCommands, this) );
    m_cleanThread = shared_ptr<thread>( new thread(&MyServer::delClients, this) );
    m_comandsThread->join();
    m_acceptThread->join();
    m_delStarted = false;
    m_cleanThread->join();
}

void MyServer::myAccept()
{
    while (m_started)
    {
        crossSocket ac_sock;
        
        //nonblocking
        //u_int nb = 1;
        //int res = ioctlsocket(ac_sock, FIONBIO, &nb);
        //if (res != NO_ERROR)
        //    printf("ioctlsocket failed with error: %ld\n", res);
        //this_thread::sleep_for(chrono::microseconds(1000));
        
        ac_sock = accept(m_listen, NULL, NULL);

        if (ac_sock == 
#ifdef WINDOWS_OS
                        INVALID_SOCKET
#else
                        -1
#endif
                ) {
            printf("accept failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                    WSAGetLastError()
#else
                                                    errno
#endif
                    );
#ifdef WINDOWS_OS
            closesocket (m_listen);
#else
            close (m_listen);
#endif
            //m_started = false;
            //WSACleanup();
            break;
        } if (ac_sock ==
#ifdef WINDOWS_OS
                        WSAEWOULDBLOCK
#else
                        EWOULDBLOCK
#endif
                ) {
            continue;
        } else {
            printClientInfo(ac_sock);
            m_isClientsStartedMutex.lock();
            m_isClientsStarted.insert(make_pair(ac_sock, true));
            m_isClientsStartedMutex.unlock();
            m_isClientsUpdateMutex.lock();
            m_isClientsUpdate.insert(make_pair(ac_sock, true));
            m_isClientsUpdateMutex.unlock();
            m_clientsMutex.lock();
            m_clients.insert(make_pair(ac_sock, shared_ptr<thread> ( new thread(&MyServer::exchange, this, ac_sock) )));
            m_clientsMutex.unlock();
        }
    }
    
}

void MyServer::printClientInfo(crossSocket ac_sock) {
    sockaddr_in ac_service;
    int len = sizeof(ac_service);
    if (getsockname(ac_sock, (
#ifdef WINDOWS_OS
                                SOCKADDR
#else
                                struct sockaddr
#endif
                                * ) &ac_service, &len)
            == 
#ifdef WINDOWS_OS
                        SOCKET_ERROR
#else
                        -1
#endif
            ) {
        printf("getsocketname failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                        WSAGetLastError()
#else
                                                        errno
#endif
                );
    }
    sockaddr_in cl_service;
    int cl_len = sizeof(cl_service);
    if (getpeername(ac_sock, (
#ifdef WINDOWS_OS
                                SOCKADDR
#else
                                struct sockaddr
#endif
                             * ) &cl_service, &cl_len)
            == 
#ifdef WINDOWS_OS
                        SOCKET_ERROR
#else
                        -1
#endif
            ) {
        printf("getsocketname failed with error: %ld\n", 
#ifdef WINDOWS_OS
                                                        WSAGetLastError()
#else
                                                        errno
#endif
                );
    }
    //printf("Client connected:\n");
    //printf("  Client IP:PORT - %s:", inet_ntoa((in_addr) cl_service.sin_addr));
    //printf("%d\n", cl_service.sin_port);
    //printf("  Server create new socket for client, that IP:PORT - %s:", inet_ntoa((in_addr) ac_service.sin_addr));
    //printf("%d\n", ac_service.sin_port);
    cout << "player connected - id " << ac_sock << endl;
}

void MyServer::getCommands()
{
    string s;
    while (m_started)
    {
        s = "";
        getline(cin, s);
        if (s.substr(0,4) == "kill")
        {
            int a = atoi(s.substr(4,6).c_str());
            preDelClient(a);
            m_needToDel.push(a);
        }
        else if (s == "stop")
        {
#ifdef WINDOWS_OS
            closesocket (m_listen);
#else
            close (m_listen);
#endif
            m_clientsMutex.lock();
            for(auto it = m_clients.begin(); it != m_clients.end(); ++it)
            {
                preDelClient(it->first);
                m_needToDel.push(it->first);
            }
            m_clientsMutex.unlock();
            m_started = false;
            break;
        }
    }
}

void MyServer::exchange(crossSocket sock)
{
    int recv_buf_len = m_proto.sendClientBufLen;
    char recv_buf[recv_buf_len+1] = "";
    
    int send_buf_len = m_proto.sendServerBufLen;
    char send_buf[send_buf_len+1] = "";
    
    while (m_started)
    {
	int iResult = readn(sock, recv_buf, recv_buf_len);
        if ( iResult > 0 ) {
            //printf("%s!\n",recv_buf);
            //printf("Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 ) {
            printf("Connection closed\n");
            preDelClient(sock);
            m_needToDel.push(sock);
            break;
        }
        else {
            printf("recv failed: %d\n", 
#ifdef WINDOWS_OS
                                        WSAGetLastError()
#else
                                        errno
#endif
                    );
            preDelClient(sock);
            m_needToDel.push(sock);
            break;
        }
        
        m_isClientsStartedMutex.lock();
        bool exch_started = m_isClientsStarted.at(sock);
        m_isClientsStartedMutex.unlock();
        
        string send_command;
        string error;
        if (!exch_started) {    // если сервер остановил
            send_command = "stop";
            m_gameMutex.lock();
            m_game.delPlayer(sock);
            m_gameMutex.unlock();
        }
	else {
            string command;
            Player player_param;
            string pass;
            
            m_proto.convert(recv_buf, recv_buf_len, command, player_param, pass);
            send_command = analize(command, player_param, pass, sock, error);
            if (send_command == "stop from client")
                break;
        }
        
        m_gameMutex.lock();
        map<crossSocket, Player> pls = m_game.getPlayers();
        crossSocket croupier = m_game.getCroupier();
        int val = m_game.getValue();
        m_gameMutex.unlock();
        
        strcpy(send_buf, m_proto.convert(send_command, val, sock, croupier, pls, error).c_str());
        
        m_isClientsUpdateMutex.lock();
        iResult = send( sock, send_buf, send_buf_len, 0);
        if (iResult == 
#ifdef WINDOWS_OS
                        SOCKET_ERROR
#else
                        -1
#endif
                ) {
            printf("send failed with error: %d\n", 
#ifdef WINDOWS_OS
                                                    WSAGetLastError()
#else
                                                    errno
#endif
                    );
#ifdef WINDOWS_OS
            closesocket (sock);
#else
            close (sock);
#endif
            //TODO off client
            throw 6;
        }
        if (send_command == "info")
            m_isClientsUpdate.at(sock) = false;
        m_isClientsUpdateMutex.unlock();
        
        if (send_command == "stop")
        {
            break;
        }
    }
    m_gameMutex.lock();
    m_game.delPlayer(sock);
    m_gameMutex.unlock();
}

string MyServer::analize(const string& command, const Player& player_param, const string& pass, crossSocket sock, string &error)
{
    if (command == "ok") {
        m_isClientsUpdateMutex.lock();
        bool upd = m_isClientsUpdate.at(sock);
        m_isClientsUpdateMutex.unlock();
        string res = upd ? "info" : "ok";
        return res;
    }
    else if (command == "stop") {
        m_gameMutex.lock();
        m_game.delPlayer(sock);
        m_gameMutex.unlock();
        preDelClient(sock);
        m_needToDel.push(sock);
        return "stop from client";
    }
    else if (command == "enter_p") {
        m_gameMutex.lock();
        m_game.addPlayer(sock, player_param.money, error);
        m_gameMutex.unlock();
    }
    else if (command == "enter_c") {
        m_gameMutex.lock();
        m_game.addCroupier(sock, pass, error);
        m_gameMutex.unlock();
    }
    else if (command == "bet") {
        m_gameMutex.lock();
        if (m_game.isPlayer(sock))
            m_game.setBet(player_param.bet, sock, error);
        else
            error += "you are not player";
        m_gameMutex.unlock();
    }
    else if (command == "rotate") {
        m_gameMutex.lock();
        if (m_game.getCroupier() == sock)
            m_game.doBets();
        else
            error += "you are not croupier";
        m_gameMutex.unlock();
    }
    else {
        error += "wrong command type";
    }
    
    if (error != "") {
        return "error";
    }
    else {
        updateAll();
        return "info";
    }
}

void MyServer::updateAll()
{
    m_isClientsUpdateMutex.lock();
    for(auto i=m_isClientsUpdate.begin(); i != m_isClientsUpdate.end(); ++i)
    {
        i->second = true;
    }
    m_isClientsUpdateMutex.unlock();
}

void MyServer::preDelClient(crossSocket sock)
{
    m_isClientsStartedMutex.lock();
    m_isClientsStarted.at(sock) = false;
    m_isClientsStartedMutex.unlock();
    
    m_isClientsUpdateMutex.lock();
    m_isClientsUpdate.at(sock) = false;
    m_isClientsUpdateMutex.unlock();
}

void MyServer::delClients()
{
    while (m_delStarted)
    {
        this_thread::sleep_for(chrono::seconds(1));
        while (m_needToDel.size() != 0) {
            crossSocket sock = m_needToDel.front();
            m_needToDel.pop();
            
            m_clientsMutex.lock();
            m_clients.at(sock)->join();
            m_clientsMutex.unlock();
            
            m_isClientsStartedMutex.lock();
            m_isClientsStarted.erase(sock);
            m_isClientsStartedMutex.unlock();
            
            m_isClientsUpdateMutex.lock();
            m_isClientsUpdate.erase(sock);
            m_isClientsUpdateMutex.unlock();
            
            m_clientsMutex.lock();
            m_clients.erase(sock);
            m_clientsMutex.unlock();
            cout << " player " << sock << " deleted" << endl;
            if (m_needToDel.size() == 0)
                updateAll();
        }
    }
}
