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

namespace BET_TYPE
{
    const string onesecond = "1/2";
    const string twosecond = "2/2";
    const string even = "even";
    const string odd = "odd";
    const string number = "number";
    const string no_bet = "no bet";
//    const string onethird = "1/3";
//    const string twothird = "2/3";
//    const string treethird = "3/3";
}

class MyClient
{
private:
    SOCKET m_socket;
    bool m_started;
    const int m_players_count;
    
    shared_ptr<thread> m_recv_thread;
    shared_ptr<thread> m_send_thread;
    
    struct Bet
    {
        Bet()
           :betValue(BET_TYPE::no_bet)
           ,number(" ")
           ,money(" ")
        {}
        string betValue;
        string number;
        string money;
    };
    struct GamePlayer
    {
        int socket;
        int money;
        int last_bet;
        int last_win;
        Bet bet;
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

