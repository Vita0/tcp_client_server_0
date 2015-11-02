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

using namespace std;

class Player {
    bool m_started;
    Player(SOCKET sock);//start thread
    ~Player();//join thread
};

class MyServer {
private:
    mutex m_cout_mutex;
    mutex m_clients_mutex;
    map<SOCKET,Player> m_clients;
    map<SOCKET,pair<shared_ptr<thread>,shared_ptr<thread>>> m_clients_threads;
    SOCKET m_listen;
    
    bool m_started;
    
    shared_ptr<thread> m_accept_thread;
    shared_ptr<thread> m_comands_thread;
public:
    MyServer(const char *ip, u_short port);
    ~MyServer();
    void start();
    void my_accept();
    void exchange(SOCKET sock);
    void getCommands();
};


#endif	/* MYSERVER_H */

