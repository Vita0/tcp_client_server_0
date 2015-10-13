/* 
 * File:   MyServer.h
 * Author: Vita
 *
 * Created on 13 октября 2015 г., 0:28
 */

#ifndef MYSERVER_H
#define	MYSERVER_H

#include <Winsock2.h>
#include <queue>
#include <map>
#include "thread"
#include <mutex>
#include <memory>

#include <cstdlib>
#include <iostream>
#include <stdio.h>

using namespace std;

#define DEFAULT_BUFLEN 7

class MyServer
{
    SOCKET m_listen;
    vector<SOCKET> m_acs;
    bool m_stop;
    shared_ptr<thread> acc;
    shared_ptr<thread> comands;
    map<SOCKET,shared_ptr<thread>> ths;
    queue<SOCKET> ths_for_del;
    mutex ths_mutex;
public:
    MyServer(const char *ip, u_short port);
    ~MyServer();
    void my_accept();
    void exchange(SOCKET sock);
    void getCommands();
    void start();
};

#endif	/* MYSERVER_H */

