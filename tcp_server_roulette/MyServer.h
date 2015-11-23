/* 
 * File:   MyServer.h
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 14:16
 */

#ifndef MYSERVER_H
#define	MYSERVER_H

#include "../LinuxWindows.h"
#include <memory>
#include <thread>
#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <string>
#include <queue>

#include "RouletteProtocol.h"

using namespace std;

class MyServer
{
private:
    mutex m_clientsMutex;
    mutex m_gameMutex;
    mutex m_isClientsStartedMutex;
    mutex m_isClientsUpdateMutex;
    
    map<crossSocket,shared_ptr<thread>> m_clients;
    map<crossSocket,bool> m_isClientsStarted;
    map<crossSocket,bool> m_isClientsUpdate;
    
    crossSocket m_listen;
    bool m_started;
    bool m_delStarted;
    
    Game m_game;
    Protocol m_proto;
    
    queue<crossSocket> m_needToDel;
    
    shared_ptr<thread> m_acceptThread;
    shared_ptr<thread> m_comandsThread;
    shared_ptr<thread> m_cleanThread;
public:
    MyServer(const char *ip, u_short port);
    ~MyServer();
    void start();
private:
    void myAccept();
    void printClientInfo(crossSocket ac_sock);
    void getCommands();
    void exchange(crossSocket sock);
    string analize(const string &command, const Player &player_param, const string &pass, 
                 crossSocket sock, string &error);
    void updateAll();
    
    void preDelClient(crossSocket sock);
    void delClients();
};


#endif	/* MYSERVER_H */

//TODO server accept whithout errors - may be because netbeans can't understand windows function / or some mingw problem
