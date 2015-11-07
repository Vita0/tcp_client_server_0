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

using namespace std;

class MyClient
{
private:
    SOCKET m_socket;
    bool m_started;
    
    shared_ptr<thread> m_recv_thread;
    shared_ptr<thread> m_send_thread;
    
public:
    MyClient(const char *server_ip, u_short server_port);
    ~MyClient();
    void start();
    void mySend();
    void myRecv();
};

#endif	/* MYCLIENT_H */

