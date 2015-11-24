#include "MyClient.h"
#include <cstring>
#include <string>

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

MyClient::MyClient(const char *server_ip, u_short server_port)
    :m_started(false)
    ,m_players_count(MAX_PLAYER_COUNT)
    ,m_number("")
    ,m_croupier("")
    ,m_rouletteValue(NO_VALUE_str)
{
    m_players.resize(m_players_count);
    for(auto i=m_players.begin(); i!=m_players.end(); ++i)
    {
        i->first = 0; i->second = Player();
    }
    for(int i = 0; i < 3; ++i)
    {
        m_serverErrors.push_back("");
    }
    
    int iResult;
#ifdef WINDOWS_OS
    WSADATA wsaData;
    // The WSAStartup function initiates use of the Winsock DLL by a process.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed with error: %ld\n", iResult);
        throw 1;
    }
#endif
    
    //----------------------
    // Create a crossSocket for listening for
    // incoming connection requests.
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == 
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
    struct sockaddr_in clientService; 
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(server_ip);
    clientService.sin_port = htons(server_port);

    //----------------------
    // Connect to server.
    iResult = connect( m_socket, (
#ifdef WINDOWS_OS
                                    SOCKADDR
#else
                                    struct sockaddr
#endif
                                    *) &clientService, sizeof(clientService) );
    if ( iResult == 
#ifdef WINDOWS_OS
                    SOCKET_ERROR
#else
                    -1
#endif
            ) {
#ifdef WINDOWS_OS
        closesocket (m_socket);
#else
        close (m_socket);
#endif
        printf("Unable to connect to server: %ld\n", 
#ifdef WINDOWS_OS
                                                    WSAGetLastError()
#else
                                                    errno
#endif
                );
        //WSACleanup();
        throw 3;
    }
}

MyClient::~MyClient()
{
    // shutdown the connection since no more data will be sent
    int iResult = shutdown(m_socket, 
#ifdef WINDOWS_OS
                                    SD_SEND
#else
                                    SHUT_RDWR
#endif
                           );
    if (iResult == 
#ifdef WINDOWS_OS
            SOCKET_ERROR
#else
            -1
#endif
            ) {
        printf("shutdown failed: %d\n", 
#ifdef WINDOWS_OS
                                        WSAGetLastError()
#else
                                        errno
#endif
                );
        //closesocket(m_socket);
        //WSACleanup();
    }
    
    // cleanup
#ifdef WINDOWS_OS
        closesocket (m_socket);
        WSACleanup();
#else
        close (m_socket);
#endif
    
}

void MyClient::start()
{
    m_started = true;
    m_exchangeThread = shared_ptr<thread>( new thread(&MyClient::exchange, this) );
    m_getCommandThread = shared_ptr<thread>( new thread(&MyClient::getCommand, this) );
    m_getCommandThread->join();
    m_exchangeThread->join();
}

void MyClient::getCommand()
{
    while (m_started)
    {
        updateScreen();
        string s = "";
        getline(cin, s);
        m_command = s;
        if (m_command == "stop") break;
    }
}

void MyClient::exchange()
{
    Protocol p;
    while (m_started)
    {
        const int send_buf_len = p.sendClientBufLen;
        char send_buf[send_buf_len+1];
        
        if (m_command == "") {
            strcpy(send_buf, "ok");
            this_thread::sleep_for(chrono::milliseconds(1000/*25*/));
        }
        else {
            strcpy(send_buf, m_command.c_str());
            m_command = "";
        }

        int iResult = send( m_socket, send_buf, send_buf_len, 0);
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
            closesocket (m_socket);
#else
            close (m_socket);
#endif
            throw 6;
        }
        if (strcmp(send_buf,"stop") == 0)
        {
            m_started = false;
            break;
        }
        
        const int recv_buf_len = p.sendServerBufLen;
        char recv_buf[recv_buf_len+1];
        iResult = readn(m_socket, recv_buf, recv_buf_len);
        if ( iResult > 0 ) {
            //printf("%s\n",recvbuf);
            //printf("Bytes received: %d\n", iResult);
        }
        else if ( iResult == 0 ) {
            printf("Connection closed\n");
            m_started = false;
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
            m_started = false;
            break;
        }
        
        bool upd = true;
        char buf[p.headerLen + 1];
        sscanf(recv_buf, "%s", buf);
        string recv_command = buf;
        char *rb = recv_buf;
        rb += strlen(buf);
        
        if (recv_command == "error")
        {
            string er = recv_buf;
            addError(m_serverErrors,er);
        }
        else if (recv_command == "info")
        {
            int inc = 0;
            char tmp1[p.headerLen + 2];
            char tmp2[p.headerLen + 2];
            char tmp3[p.headerLen + 2];
            sscanf(rb, "%s %s %s", tmp1, tmp2, tmp3);
            m_rouletteValue = tmp1;
            m_number = tmp2;
            m_croupier = tmp3;
            inc = strlen(tmp1) + strlen(tmp2) + strlen(tmp3) + 3 + 6;
            rb += inc;
            
            for(int i = 0; i != MAX_PLAYER_COUNT; ++i)
            {
                char bet_val[BET_TYPE::max_string_len + 1];
                sscanf(rb, "%d %s %d %d %d %d %d ", 
                        &m_players.at(i).first, bet_val, &m_players.at(i).second.bet.number,
                        &m_players.at(i).second.bet.money, &m_players.at(i).second.last_bet,
                        &m_players.at(i).second.last_win, &m_players.at(i).second.money);
                m_players.at(i).second.bet.betValue = bet_val;
                
                char tmp[p.sendServerBufLen + 1];
                sprintf(tmp, "%d %s %d %d %d %d %d ",
                        m_players.at(i).first, bet_val, m_players.at(i).second.bet.number,
                        m_players.at(i).second.bet.money, m_players.at(i).second.last_bet,
                        m_players.at(i).second.last_win, m_players.at(i).second.money);
                inc = strlen(tmp);
                rb += inc;
            }
        }
        else if (recv_command == "ok")
        {
            upd = false;
        }
        else if (recv_command == "stop")
        {
            m_started = false;
            cout << "you stoped by server. press any key" << endl;
            break;
        }
        if (upd) updateScreen();
    }
}

void MyClient::updateScreen()
{
    cout << "\033[2J\033[1;1H"; //clear screen
    string croupier = (m_croupier=="0")?"waiting croupier":m_croupier;
    string roulette_value = (m_rouletteValue==NO_VALUE_str)?"was now games yet":m_rouletteValue;
    cout << "info:" << endl;
    cout << "             croupier: " << croupier << "     you number: " << m_number << endl << endl;
    cout << "                  last roulette value: " << roulette_value << endl << endl;
    cout << "        <player>   <bet>   <number>  <money> <last bet>  <last win> <money>   " << endl;
    for(auto i=m_players.begin(); i!=m_players.end(); ++i)
    cout << "        " << i->first << "\t" << i->second.bet.betValue 
                       << "\t" << i->second.bet.number << "\t" << i->second.bet.money 
                       << "\t" << i->second.last_bet << "\t" << i->second.last_win 
                       << "\t" << i->second.money << endl;
    cout << " last (s) errors:" << endl << endl;
    for(auto error = m_serverErrors.begin(); error != m_serverErrors.end(); ++error)
    cout << "                 " << *error << endl << endl;
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
