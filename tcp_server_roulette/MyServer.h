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
    shared_ptr<thread> m_recv_thread;
    shared_ptr<thread> m_send_thread;
public:
    Player(SOCKET sock);
    virtual ~Player();
    virtual void start() final;//start thread
    virtual void stop() final;//join thread
    virtual void recv() final;
    virtual void send() final;
protected:
    virtual void my_recv() = 0;
    virtual void my_send() = 0;
private:
    Player();
};

class ServerSidePlayer: public Player
{
public:
    ServerSidePlayer(SOCKET sock);
    virtual ~ServerSidePlayer();
private:
    virtual void my_recv();
    virtual void my_send();
private:
    ServerSidePlayer();
};

class MyServer {
private:
    mutex m_cout_mutex;
    mutex m_clients_mutex;
    map<SOCKET,ServerSidePlayer> m_clients;
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

//TODO use class Player in client app
//TODO game logic
//TODO server accept whithout errors - may be because netbeans can't understand windows function / or some mingw problem
