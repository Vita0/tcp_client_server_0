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
#include <string.h>

using namespace std;

class Protocol
{
public:
    const int sendServerBufLen;
    const int sendClientBufLen;
    const int headerLen;
    
    Protocol()
    :sendServerBufLen(400)
    ,sendClientBufLen(100)
    ,headerLen(10)
    {}
    
    void convert(char* recv_buf, int len, string &command, Player &player_param, string &pass)
    {
        char buf[headerLen + 1];
        sscanf(recv_buf, "%s", buf);
        recv_buf += strlen(buf)+1;//headerLen;
        command = buf;
        
        // Server commands
        if (command == "stop") {
            return;
        }
        else if (command == "ok") {
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
            recv_buf += strlen(bet_val);
            
            if (bet_val == BET_TYPE::number)
                sscanf(recv_buf, "%d %d", &player_param.bet.number, &player_param.bet.money );
            else
                sscanf(recv_buf, "%d", &player_param.bet.money );
        }
        else if (command == "rotate") {
            return;
        }
    }
    
    string convert(const string &command, const int val,
                   const SOCKET sock, const SOCKET croupier,
                   const map<SOCKET, Player> &pls, const string &error)
    {
        if (command == "ok" || command == "stop") {
            return command;
        }
        else if (command == "info") {
            char buf[sendServerBufLen + 1];
            int idx = 0;
            string s = "info      ";
            sprintf(buf, "%s %d %d %d ", s.c_str(), val, sock, croupier);
            idx += strlen(buf+idx);
            
            auto it = pls.begin();
            for(int i = 0; i != MAX_PLAYER_COUNT; ++i)
            {
                char *from = buf+idx;
                if (it != pls.end()) {
                    sprintf(from, "%d %s %d %d %d %d %d ", 
                            it->first, it->second.bet.betValue.c_str(), it->second.bet.number,
                            it->second.bet.money, it->second.last_bet,
                            it->second.last_win, it->second.money);
                    ++it;
                }
                else {
                    sprintf(from, "%d %s %d %d %d %d %d ",
                            0, BET_TYPE::no_bet.c_str(), NO_VALUE, 0, 0, 0, 0);
                }
                idx += strlen(buf+idx);
            }
            string res = buf;
            return res;
        }
        else if (command == "error") {
            return "error     " + error; 
        }
    }
};

#endif	/* PROTOCOLROULETTE_H */

