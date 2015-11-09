#include "MyClient.h"
#include <cstring>
//#include <cstdio>
#include <stdio.h>
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

MyClient::MyClient(const char *server_ip, u_short server_port)
    :m_started(false)
    ,m_players_count(4)
    ,m_number("")
    ,m_croupier("")
    ,m_roulette_value("")
    ,m_commandInfo("")
{
    m_pls.resize(m_players_count);
    for(auto i=m_pls.begin(); i!=m_pls.end(); ++i)
    {
        i->socket = 0; i->money = 0; i->last_bet = 0; i->last_win = 0;
    }
    for(int i = 0; i < 3; ++i)
    {
        m_server_errors.push_back("");
        m_client_errors.push_back("");
    }
    
    WSADATA wsaData;
    // The WSAStartup function initiates use of the Winsock DLL by a process.
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %ld\n", iResult);
        throw 1;
    }
    //----------------------
    // Create a SOCKET for listening for
    // incoming connection requests.
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        //WSACleanup();
        throw 2;
    }
    struct sockaddr_in clientService; 
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(server_ip);
	//clientService.sin_addr.s_addr = inet_pton(AF_INET,);
    clientService.sin_port = htons(server_port);

    //----------------------
    // Connect to server.
    iResult = connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) );
    if ( iResult == SOCKET_ERROR) {
        closesocket (m_socket);
        wprintf(L"Unable to connect to server: %ld\n", WSAGetLastError());
        //WSACleanup();
        throw 3;
    }
}

MyClient::~MyClient()
{
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(m_socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"shutdown failed: %d\n", WSAGetLastError());
        //closesocket(m_socket);
        //WSACleanup();
    }
    
    // cleanup
    closesocket(m_socket);
    WSACleanup();
}

void MyClient::start()
{
    m_started = true;
    m_recv_thread = shared_ptr<thread>( new thread(&MyClient::myRecv, this) );
    m_send_thread = shared_ptr<thread>( new thread(&MyClient::mySend, this) );
    m_send_thread->join();
    m_recv_thread->join();
}

void MyClient::mySend()
{
    while (m_started)
    {
        m_commandInfo = "enter command:";
        updateScreen();
        string s;
        cin >> s;
        if (!m_started) break;
        const int sendbuflen = 70;
        char sendbuf[sendbuflen+1];
        const int inc = 10;
        int idx = 0;
        
        if (s == "enter_p")
        {
            idx = 0;
            strcpy(sendbuf+idx, s.c_str());
            int money = 0;
            m_commandInfo = "enter MONEY:";
            updateScreen();
            cin >> money;
            if (money <= 0)
            {
                addError(m_client_errors, "wrong money!");
                continue;
            }
            idx += inc;
            sprintf(sendbuf+idx, "%d", money);
        }
        else if (s == "enter_c")
        {
            idx = 0;
            strcpy(sendbuf+idx, s.c_str());
            string pass;
            m_commandInfo = "enter PASSWORD:";
            updateScreen();
            cin >> pass;
            idx += inc;
            sprintf(sendbuf+idx, "%s", pass.c_str());
        }
        else 
        {
            addError(m_client_errors, "wrong command!");
            continue;
        }
        int iResult = send( m_socket, sendbuf, sendbuflen, 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(m_socket);
            throw 6;
        }
    }
}

void MyClient::myRecv()
{
    while (m_started)
    {
        updateScreen();
        const int recvbuflen = 260;
        char recvbuf[recvbuflen+1];
        int iResult = readn(m_socket, recvbuf, recvbuflen);
        if ( iResult > 0 ) {
            wprintf(L"%s\n",recvbuf);
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

        const int buflen = 10;
        char buf[buflen+1];
        const int inc = 10;
        int idx = 0;
        
        if (strcmp(recvbuf+idx, "info") == 0)
        {
            idx += inc;
            m_number = recvbuf+idx;
            idx += inc;
            m_roulette_value = recvbuf+idx;
            idx += inc;
            if (strcmp(recvbuf+idx, "croupier") != 0)
                addError(m_client_errors, "protocol error: info - field croupier not found");
            idx += inc;
            m_croupier = recvbuf+idx;
            for(GamePlayer &i: m_pls)
            {
                idx += inc;
                if (strcmp(recvbuf+idx, "player") != 0)
                    addError(m_client_errors, "protocol error: info - field player not found");
                idx += inc;
                i.socket = atoi(recvbuf+idx);
                idx += inc;
                i.money = atoi(recvbuf+idx);
                idx += inc;
                i.last_bet = atoi(recvbuf+idx);
                idx += inc;
                i.last_win = atoi(recvbuf+idx);
            }
            cout << "recv, idx = " << idx << endl;
        }
        else if (strcmp(recvbuf+idx,"error") == 0)
        {
            idx += inc;
            string er = recvbuf+idx;
            addError(m_server_errors, er);
        }
        else if (strcmp(recvbuf+idx,"stop") == 0)
        {
            cout << "server say 'stop'" << endl;
            m_started = false;
            const int sendbuflen = 70;
            char sendbuf[sendbuflen+1];
            strcpy(sendbuf,"bye");
            int iResult = send( m_socket, sendbuf, sendbuflen, 0);
            if (iResult == SOCKET_ERROR) {
                wprintf(L"send failed with error: %d\n", WSAGetLastError());
                closesocket(m_socket);
                throw 6;
            }
            break;
        }
    }
}
const string NO_VALUE = "37";
void MyClient::updateScreen()
{
    cout << "\033[2J\033[1;1H"; //clear screen
    string croupier = (m_croupier=="0")?"waiting croupier":m_croupier;
    string roulette_value = (m_roulette_value==NO_VALUE)?"was now games yet":m_roulette_value;
    cout << "            info:" << endl;
    cout << "                    croupier: " << croupier << "     you number: " << m_number << endl << endl;
    cout << "                             last roulette value: " << roulette_value << endl;
    cout << "                     <player>      <money>         <bet>       <win>" << endl;
    for(auto i=m_pls.begin(); i!=m_pls.end(); ++i)
    cout << "                    " << i->socket << "\t\t" << i->money << "\t\t" << i->last_bet << "\t\t" << i->last_win << endl;
    cout << " last (s) errors:" << endl;
    for(auto error = m_server_errors.begin(); error != m_server_errors.end(); ++error)
    cout << "                 " << *error << endl;
    cout << " last (c) errors:" << endl;
    for(auto error = m_client_errors.begin(); error != m_client_errors.end(); ++error)
    cout << "                 " << *error << endl;
    cout << "    command info:" << m_commandInfo << endl;
}


void MyClient::addError(deque<string> &deq, const string &s)
{
    if (deq.size() == 3)
    {
        deq.pop_back();
    }
    if ( !deq.empty() )
        if (*deq.begin() != s)
            deq.push_front(s);
}
