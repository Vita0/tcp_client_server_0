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
    SOCKET m_socket;
public:
    shared_ptr<thread> m_recv_thread;
    shared_ptr<thread> m_send_thread;
    Player(SOCKET sock);
    ~Player();
    void start();//start thread
    void stop();//join thread
    void recv();
    void send();
private:
    Player();
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
    void myAccept();
    void exchange(SOCKET sock);
    void getCommands();
};


#endif	/* MYSERVER_H */

//TODO game logic
//TODO server accept whithout errors
