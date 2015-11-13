#include <thread>
#include <chrono>

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
    m_acceptThread = shared_ptr<thread>( new thread(&MyServer::myAccept, this) );
    m_comandsThread = shared_ptr<thread>( new thread(&MyServer::getCommands, this) );
    m_comandsThread->join();
    m_acceptThread->join();
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
        //this_thread::sleep_for(chrono::microseconds(1000));
        
        cout << "accept now ...";
        ac_sock = accept(m_listen, NULL, NULL);
        cout << "accept ok!" << endl;
        if (ac_sock == INVALID_SOCKET) {
            wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
            closesocket(m_listen);
            m_started = false;
            //WSACleanup();
            break;
        } if (ac_sock == WSAEWOULDBLOCK) {
            continue;
        } else {
            printClientInfo(ac_sock);
            m_clientsMutex.lock();
            m_clients.insert(make_pair(ac_sock, shared_ptr<thread> ( new thread(&MyServer::exchange, this, ac_sock) )));
            m_clientsMutex.unlock();
            m_isClientsUpdateMutex.lock();
            m_isClientsUpdate.insert(make_pair(ac_sock, true));
            m_isClientsUpdateMutex.unlock();
        }
    }
    delAndJoinAll();
}

void MyServer::delAndJoinAll() {
    m_clientsMutex.lock();
    auto end = m_clients.end();
    for(auto i=m_clients.begin(); i != end; ++i)
    {
        cout << "myAccept: client" << i->first << " stoping ... ";
        
        m_isClientsStartedMutex.lock();
        m_isClientsStarted.at(i->first) = false;
        m_isClientsStartedMutex.unlock();
        
        i->second->join();
        cout << "ok!" << endl;
        
        int iResult = shutdown(i->first, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"shutdown failed: %d\n", WSAGetLastError());
            closesocket(i->first);
        }
        closesocket(i->first);
    }
    m_clients.clear();
    m_clientsMutex.unlock();
}

void MyServer::printClientInfo(SOCKET ac_sock) {
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
            delAndJoin(a);
        }
        else if (s == "stop")
        {
            m_started = false;
            closesocket(m_listen);
        }
    }
}

void MyServer::delAndJoin(SOCKET sock)
{
    m_clientsMutex.lock();
    auto it = m_clients.find(sock);
    if (it != m_clients.end())
    {
        m_isClientsStartedMutex.lock();
        m_isClientsStarted.at(sock) = false;
        m_isClientsStartedMutex.unlock();
        it->second->join();
        m_clients.erase(it);
    }
    else
    {
        cout << "delAndJoin(): bad socket" << endl;
    }
    m_clientsMutex.unlock();
}

void MyServer::exchange(SOCKET sock)
{
    m_isClientsStartedMutex.lock();
    bool exch_started = m_isClientsStarted.at(sock);
    m_isClientsStartedMutex.unlock();
    
    int recv_buf_len = m_proto.sendClientBufLen;
    char recv_buf[recv_buf_len+1] = "";
    
    int send_buf_len = m_proto.sendServerBufLen;
    char send_buf[send_buf_len+1] = "";
    
    //bool is_exit = false;
    
    while (!m_started)
    {
	int iResult = readn(sock, recv_buf, recv_buf_len);
        if ( iResult > 0 ) {
            wprintf(L"%s\n",recv_buf);
            wprintf(L"Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 ) {
            wprintf(L"Connection closed\n");
            m_started = false;
        }
        else {
            wprintf(L"recv failed: %d\n", WSAGetLastError());
            m_started = false;
        }
        
        m_isClientsStartedMutex.lock();
        exch_started = m_isClientsStarted.at(sock);
        m_isClientsStartedMutex.unlock();
        
        string send_command;
        string error;
        if (!exch_started) {    // если сервер остановил
            send_command = "stop";
            m_started = false;
        }
	else {
            string command;
            Player player_param;
            string pass;
            
            m_proto.convert(recv_buf, recv_buf_len, command, player_param, pass);
            send_command = analize(command, player_param, pass, sock, error);
        }
        
        m_gameMutex.lock();
        map<SOCKET, Player> pls = m_game.getPlayers();
        SOCKET croupier = m_game.getCroupier();
        int val = m_game.getValue();
        m_gameMutex.unlock();
        
        strcpy(send_buf, m_proto.convert(send_command, val, sock, croupier, pls, error).c_str());
        
        m_isClientsUpdateMutex.lock();
	//send(isError?error:isUpdate?info,m_update=false:ok);
        if (send_command == "info" || send_command == "stop")
            m_isClientsUpdate.at(sock) = false;
        m_isClientsUpdateMutex.unlock();
    }
}

string MyServer::analize(const string& command, const Player& player_param, const string& pass, SOCKET sock, string &error)
{
    if (command == "ok") {
        m_isClientsUpdateMutex.lock();
        bool upd = m_isClientsUpdate.at(sock);
        m_isClientsUpdateMutex.unlock();
        string res = upd ? "info" : "ok";
        if (upd) updateAll();
        return res;
    }
    else if (command == "stop") {
        m_gameMutex.lock();
        m_game.delPlayer(sock);
        m_gameMutex.unlock();
        delAndJoin(sock);
        updateAll();
    }
    else if (command == "enter_p") {
        m_gameMutex.lock();
        m_game.addPlayer(sock, player_param.money, error);
        m_gameMutex.unlock();
        updateAll();
    }
    else if (command == "enter_c") {
        m_gameMutex.lock();
        m_game.addCroupier(sock, pass, error);
        m_gameMutex.unlock();
        updateAll();
    }
    else if (command == "bet") {
        m_gameMutex.lock();
        m_game.setBet(player_param.bet, sock, error);
        m_gameMutex.unlock();
        updateAll();
    }
    else if (command == "rotate") {
        m_gameMutex.lock();
        m_game.doBets();
        m_gameMutex.unlock();
        updateAll();
    }
    else {
        error += "wrong command type\n";
    }
    
    if (error != "") {
        return "error";
    }
    else {
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
