/* 
 * File:   MyServer.h
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 14:16
 */

#ifndef MYSERVER_H
#define	MYSERVER_H

#include <Winsock2.h>
#include <memory>
#include <thread>
#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const short NO_VALUE = 37;

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

class Game
{
public:
    enum StateType
    {
        WAITING_CROUPIER,
        WAITING_START
    };
    struct Bet
    {
        Bet(string betValue, int number, int money)
           :betValue(betValue)
           ,number(number)
           ,money(money)
        {}
        Bet()
           :betValue(BET_TYPE::no_bet)
           ,number(NO_VALUE)
           ,money(0)
        {}
        string betValue;
        int number;
        int money;
    };
    struct GamePlayer
    {
        SOCKET socket;
        int money;
        int last_bet;
        int last_win;
        Bet bet;
    };

public:
    const short m_maxPlayersCountValue;
private:
    StateType m_state;
    SOCKET m_croupier;

    short m_rouletteValue;
    string m_croupierPassword;
    vector<Game::GamePlayer> m_players;
public:
    Game() :m_maxPlayersCountValue(4)
           ,m_state(WAITING_CROUPIER)
           ,m_croupier(0)
           ,m_rouletteValue(NO_VALUE)
           ,m_croupierPassword("password")
    {};
    virtual ~Game(){};
    void setCroupier(SOCKET sock) { m_croupier = sock; m_state = WAITING_START; m_players.resize(0); return; }
    void clrCroupier() { m_croupier = 0; m_state = WAITING_CROUPIER; return; }
    short getPlayersCount() { return m_players.size(); }
    bool checkCroupierPassword(string s) { return s == m_croupierPassword; }
    void addPlayer(SOCKET sock, int money)
    {
        Game::GamePlayer lol;
        lol.socket = sock;
        lol.money = money;
        lol.last_bet = 0;
        lol.last_win = 0;
        Bet bet;
        lol.bet = bet;
        m_players.push_back(lol);
    }
    void delPlayer(SOCKET sock)
    {
        if (sock == m_croupier)
        {
            clrCroupier();
        }
        for(auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            if (it->socket == sock)
            {
                m_players.erase(it);
                break;
            }
        }
    }
    SOCKET getCroupier() { return m_croupier; };
    vector<Game::GamePlayer> getPlayerList() { return m_players; }
    short getRouletteValue() { return m_rouletteValue; }
    bool isNoBets(SOCKET s) { 
        bool res = true;
        for (auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            if (it->socket == s)
                res = it->bet.money <= 0; 
        }
        return res;
    }
    void setBet(string betValue, int number, int money, SOCKET s) {
        for (auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            if (it->socket == s)
                it->bet = Bet(betValue, number, money);
        }
    }
    void doBet(short rouletteValue, SOCKET s) {     
        auto it = m_players.begin();
        for (; it != m_players.end(); ++it)
        {
            if (it->socket == s) break;
        }
        if (it == m_players.end())
            return;
        
        bool odd = rouletteValue%2;
        bool onesecond = rouletteValue<(NO_VALUE-1)/2;
        bool number = rouletteValue == it->bet.number;
        int koef=0;
        if (it->bet.betValue == BET_TYPE::odd) {
            if (odd) koef = 2; 
        } else if (it->bet.betValue == BET_TYPE::even) {
            if (!odd) koef = 2; 
        } else if (it->bet.betValue == BET_TYPE::onesecond) {
            if (onesecond) koef = 2; 
        } else if (it->bet.betValue == BET_TYPE::twosecond) {
            if (!onesecond) koef = 2; 
        } else if (it->bet.betValue == BET_TYPE::twosecond) {
            if (number) koef = 36;
        }
        
        m_rouletteValue = rouletteValue;
        it->bet = Bet();
    }
    int getPlMoney(SOCKET s) {
        int res = -1;
        for (auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            if (it->socket == s)
                res = it->money;
        }
        return res;
    }
};

class MyServer;

class Player 
{
private:
    bool m_started;
    SOCKET m_socket;
    shared_ptr<thread> m_recvThread;
    shared_ptr<thread> m_sendThread;

    Game *m_game;
    mutex *m_game_mutex;
    bool m_isCroupier;    
public:
    Player(SOCKET sock, Game *game, mutex *game_mutex);
    virtual ~Player();
    virtual void start() final;//start thread
    virtual void stop() final;//join thread
    virtual void myRecv() final;
    virtual void mySend() final;
    bool isCroupier() { return m_isCroupier; }
    
private:
    Player();
};

class MyServer
{
private:
    mutex m_cout_mutex;
    mutex m_clients_mutex;
    mutex m_game_mutex;
    
    map<SOCKET,Player> m_clients;
    SOCKET m_listen;
    
    bool m_started;
    
    Game m_game;
    
    shared_ptr<thread> m_accept_thread;
    shared_ptr<thread> m_comands_thread;
public:
    MyServer(const char *ip, u_short port);
    ~MyServer();
    void start();
    void myAccept();
    void getCommands();
};


#endif	/* MYSERVER_H */

//TODO rand in Player::doBet(short ))
//TODO remove player, when recive failed
//TODO server accept whithout errors - may be because netbeans can't understand windows function / or some mingw problem
