/* 
 * File:   ClientSidePlayer.h
 * Author: Vita
 *
 * Created on 4 ноября 2015 г., 2:16
 */

#ifndef CLIENTSIDEPLAYER_H
#define	CLIENTSIDEPLAYER_H

#include "../tcp_server_roulette/MyServer.h" // class Player

class ClientSidePlayer : public Player
{
public:
    ClientSidePlayer(SOCKET sock);
    virtual ~ClientSidePlayer();
private:
    virtual void my_recv();
    virtual void my_send();
private:
    ClientSidePlayer();
};

#endif	/* CLIENTSIDEPLAYER_H */

