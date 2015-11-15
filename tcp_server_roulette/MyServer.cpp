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
    ,m_delStarted(true)
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
    m_cleanThread = shared_ptr<thread>( new thread(&MyServer::delClients, this) );
    m_cleanThread->join();
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
            //m_started = false;
            //WSACleanup();
            break;
        } if (ac_sock == WSAEWOULDBLOCK) {
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
    //wprintf(L"Client connected:\n");
    //wprintf(L"  Client IP:PORT - %s:", inet_ntoa((in_addr) cl_service.sin_addr));
    //wprintf(L"%d\n", cl_service.sin_port);
    //wprintf(L"  Server create new socket for client, that IP:PORT - %s:", inet_ntoa((in_addr) ac_service.sin_addr));
    //wprintf(L"%d\n", ac_service.sin_port);
    cout << "player connected - id " << ac_sock << endl;
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
            preDelClient(a);
            m_needToDel.push(a);
            //delAndJoin(a);
        }
        else if (s == "stop")
        {
            closesocket(m_listen);
            m_clientsMutex.lock();
            for(auto it = m_clients.begin(); it != m_clients.end(); ++it)
            {
                preDelClient(it->first);
                m_needToDel.push(it->first);
            }
            m_clientsMutex.unlock();
            m_delStarted = false;
            m_cleanThread->join();
            m_started = false;
        }
    }
}

void MyServer::delAndJoin(SOCKET sock)
{
//    m_clientsMutex.lock();
//    cout << "lock" << endl;
//    auto it = m_clients.find(sock);
//    if (it != m_clients.end())
//    {
//        m_isClientsStartedMutex.lock();
//        cout << "lock" << endl;
//        //m_isClientsStarted.at(sock) = false;
//        m_isClientsStarted.erase(sock);
//        cout << "erase" << endl;
//        m_isClientsStartedMutex.unlock();
//        cout << "unlock" << endl;
//        m_isClientsUpdateMutex.lock();
//        cout << "lock" << endl;
//        m_isClientsUpdate.erase(sock);
//        cout << "erase" << endl;
//        m_isClientsUpdateMutex.unlock();
//        cout << "unlock" << endl;
//        it->second->join();
//        cout << "join" << endl;
//        m_clients.erase(it);
//        cout << "erase" << endl;
//    }
//    else
//    {
//        cout << "delAndJoin(): bad socket" << endl;
//    }
//    m_clientsMutex.unlock();
//    cout << "unlock" << endl;
}

void MyServer::exchange(SOCKET sock)
{
    int recv_buf_len = m_proto.sendClientBufLen;
    char recv_buf[recv_buf_len+1] = "";
    
    int send_buf_len = m_proto.sendServerBufLen;
    char send_buf[send_buf_len+1] = "";
    
    while (m_started)
    {
	int iResult = readn(sock, recv_buf, recv_buf_len);
        if ( iResult > 0 ) {
            //wprintf(L"%s!\n",recv_buf);
            //wprintf(L"Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 ) {
            wprintf(L"Connection closed\n");
            preDelClient(sock);
            m_needToDel.push(sock);
            break;
        }
        else {
            wprintf(L"recv failed: %d\n", WSAGetLastError());
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
            cout << "!exch_started" << endl;
            send_command = "stop";
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
        map<SOCKET, Player> pls = m_game.getPlayers();
        SOCKET croupier = m_game.getCroupier();
        int val = m_game.getValue();
        m_gameMutex.unlock();
        
        strcpy(send_buf, m_proto.convert(send_command, val, sock, croupier, pls, error).c_str());
        
        m_isClientsUpdateMutex.lock();
        iResult = send( sock, send_buf, send_buf_len, 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(sock);
            //TODO off client
            throw 6;
        }
	//send(isError?error:isUpdate?info,m_update=false:ok);
        cout << "send buf: " << send_buf << endl;
        if (send_command == "info")
            m_isClientsUpdate.at(sock) = false;
        //cout << "ok!" << endl;
        m_isClientsUpdateMutex.unlock();
        if (send_command == "stop")
        {
            preDelClient(sock);
            m_needToDel.push(sock);
            break;
        }
    }
    cout << sock << " end exchange" << endl;
}

string MyServer::analize(const string& command, const Player& player_param, const string& pass, SOCKET sock, string &error)
{
    if (command == "ok") {
        m_isClientsUpdateMutex.lock();
        bool upd = m_isClientsUpdate.at(sock);
        cout << upd << endl;
        m_isClientsUpdateMutex.unlock();
        string res = upd ? "info" : "ok";
        return res;
    }
    else if (command == "stop") {
        m_gameMutex.lock();
        m_game.delPlayer(sock);
        m_gameMutex.unlock();
        cout << "push for del" << endl;
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
        error += "wrong command type\n";
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

void MyServer::preDelClient(SOCKET sock)
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
        while (m_needToDel.size() != 0) {
            SOCKET sock = m_needToDel.front();
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
            cout << "size " << m_clients.size() << endl;
            m_clientsMutex.unlock();
            cout << " player " << sock << " deleted" << endl;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}
