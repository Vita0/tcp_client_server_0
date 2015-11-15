/* 
 * File:   MyClient.h
 * Author: Vita
 *
 * Created on 7 ноября 2015 г., 23:27
 */

#ifndef MYCLIENT_H
#define	MYCLIENT_H

#include <Winsock2.h>
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
    SOCKET m_socket;
    bool m_started;
    const int m_players_count;
    
    shared_ptr<thread> m_exchangeThread;
    shared_ptr<thread> m_getCommandThread;
    
    string m_command;
    
    //screen
    string m_number;
    string m_croupier;
    string m_rouletteValue;
    vector< pair<SOCKET, Player> > m_players;
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

