/* 
 * File:   ProtocolRoulette.h
 * Author: Vita
 *
 * Created on 12 ноября 2015 г., 21:30
 */

#ifndef PROTOCOLROULETTE_H
#define	PROTOCOLROULETTE_H

#include <stdio.h>
#include "RouletteGame.h"

int readn(SOCKET fd, char *data, size_t data_len);

class Protocol
{
public:
    const int sendServerBufLen;
    const int sendClientBufLen;
    const int headerLen;
    
    Protocol()
    :sendServerBufLen(100)
    ,sendClientBufLen(20)
    ,headerLen(10)
    {}
    
    void convert(char* recv_buf, int len, string &command, Player &player_param, string &pass)
    {
        char buf[headerLen + 1];
        sscanf(recv_buf, "%s", buf);
        recv_buf += headerLen;
        command = buf;
        
        // Server commands
        if (command == "stop") {
            return;
        }
        else if (command == "enter_p") {
            sscanf(recv_buf, "%d", &player_param.money);
        }
        else if (command == "enter_c") {
            char p[50];
            sscanf(recv_buf, "%s", p);
            pass = p;
        }
        else if (command == "bet") {
            char bet_val[BET_TYPE::max_string_len + 1];
            sscanf(recv_buf, "%s", bet_val);
            player_param.bet.betValue = bet_val;
            recv_buf += (strlen(bet_val));
            if (bet_val == BET_TYPE::number)
                sscanf(recv_buf, "%d %d", &player_param.bet.number, &player_param.bet.money );
            else
                sscanf(recv_buf, "%d", &player_param.bet.money );
        }
        else if (command == "rotate") {
            return;
        }
        
        // Client commands
        
    }
};

//const int Protocol::sendServerBufLen = 100;
//const int Protocol::sendClientBufLen = 20;
//const int Protocol::headerLen = 10;

#endif	/* PROTOCOLROULETTE_H */

