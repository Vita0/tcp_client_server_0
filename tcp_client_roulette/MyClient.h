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

using namespace std;

class MyClient
{
private:
    SOCKET m_socket;
    bool m_started;
    const int m_players_count;
    
    shared_ptr<thread> m_recv_thread;
    shared_ptr<thread> m_send_thread;
    
    struct GamePlayer
    {
        int socket;
        int money;
        int last_bet;
        int last_win;
    };
    //screen
    string m_number;
    string m_croupier;
    string m_roulette_value;
    string m_commandInfo;
    vector<GamePlayer> m_pls;
    deque<string> m_server_errors;
    deque<string> m_client_errors;
public:
    MyClient(const char *server_ip, u_short server_port);
    ~MyClient();
    void start();
    void mySend();
    void myRecv();
    void updateScreen();
    void addError(deque<string> &deq, const string &s);
};

#endif	/* MYCLIENT_H */

