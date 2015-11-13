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

#include "RouletteProtocol.h"

using namespace std;

class MyServer
{
private:
    mutex m_clientsMutex;
    mutex m_gameMutex;
    mutex m_isClientsStartedMutex;
    mutex m_isClientsUpdateMutex;
    
    map<SOCKET,shared_ptr<thread>> m_clients;
    map<SOCKET,bool> m_isClientsStarted;
    map<SOCKET,bool> m_isClientsUpdate;
    
    SOCKET m_listen;
    bool m_started;
    
    Game m_game;
    Protocol m_proto;
    
    shared_ptr<thread> m_acceptThread;
    shared_ptr<thread> m_comandsThread;
public:
    MyServer(const char *ip, u_short port);
    ~MyServer();
    void start();
private:
    void myAccept();
    void delAndJoinAll();
    void delAndJoin(SOCKET sock);
    void printClientInfo(SOCKET ac_sock);
    void getCommands();
    void exchange(SOCKET sock);
    string analize(const string &command, const Player &player_param, const string &pass, 
                 SOCKET sock, string &error);
    void updateAll();
};


#endif	/* MYSERVER_H */

//TODO Proto::convert 
//TODO update
//TODO rand ...
//TODO remove player, when recive failed
//TODO server accept whithout errors - may be because netbeans can't understand windows function / or some mingw problem
