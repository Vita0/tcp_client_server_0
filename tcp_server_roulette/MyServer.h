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

class Game
{
public:
    enum StateType
    {
        WAITING_CROUPIER,
        WAITING_START
    };
    struct GamePlayer
    {
        SOCKET socket;
        int money;
        int last_bet;
        int last_win;
    };
public:
    const short m_maxPlayersCountValue;
private:
    StateType m_state;
    SOCKET m_croupier;

    short m_playersCount;
    short m_rouletteValue;
    string m_croupierPassword;
    vector<Game::GamePlayer> m_players;
public:
    Game() :m_maxPlayersCountValue(4)
           ,m_state(WAITING_CROUPIER)
           ,m_croupier(0)
           ,m_playersCount(0)
           ,m_rouletteValue(NO_VALUE)
           ,m_croupierPassword("password")
    {};
    virtual ~Game(){};
    void setCroupier(SOCKET sock) { m_croupier = sock; m_state = WAITING_START; return; }
    void clrCroupier() { m_croupier = 0; m_state = WAITING_CROUPIER; return; }
    short getPlayersCount() { return m_playersCount; }
    void incPlayersCount() { ++m_playersCount; return; }
    void decPlayersCount() { --m_playersCount; return; }
    bool checkCroupierPassword(string s) { return s == m_croupierPassword; }
    void addPlayer(SOCKET sock, int money)
    {
        Game::GamePlayer lol;
        lol.socket = sock;
        lol.money = money;
        lol.last_bet = 0;
        lol.last_win = 0;
        m_players.push_back(lol);
    }
    void delPlayer(SOCKET sock)
    {
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

//TODO use class Player in client app
//TODO game logic
//TODO server accept whithout errors - may be because netbeans can't understand windows function / or some mingw problem
