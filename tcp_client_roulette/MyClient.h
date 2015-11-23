/* 
 * File:   MyClient.h
 * Author: Vita
 *
 * Created on 7 ноября 2015 г., 23:27
 */

#ifndef MYCLIENT_H
#define	MYCLIENT_H

#include "../LinuxWindows.h"

#ifdef WINDOWS_OS
#pragma comment(lib,"wsock32.lib")
//linker -lWs2_32
#else
//linker -pthread
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/stat.h> 
//#include <sys/un.h>
//#include <netdb.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <deque>
#include <vector>
#include "../tcp_server_roulette/RouletteGame.h"
#include "../tcp_server_roulette/RouletteProtocol.h"

using namespace std;

class MyClient
{
private:
    crossSocket m_socket;
    bool m_started;
    const int m_players_count;
    
    shared_ptr<thread> m_exchangeThread;
    shared_ptr<thread> m_getCommandThread;
    
    string m_command;
    
    //screen
    string m_number;
    string m_croupier;
    string m_rouletteValue;
    vector< pair<crossSocket, Player> > m_players;
    deque<string> m_serverErrors;
public:
    MyClient(const char *server_ip, u_short server_port);
    ~MyClient();
    void start();
    void getCommand();
    void exchange();
    void updateScreen();
    void addError(deque<string> &deq, const string &s);
};

#endif	/* MYCLIENT_H */

