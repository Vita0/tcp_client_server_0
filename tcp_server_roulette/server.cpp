/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 11:00
 */

#pragma comment(lib,"wsock32.lib")

#include "MyServer.h"

using namespace std;

void tests()
{
    Protocol p;
    
    string command;
    Player player_param;
    string pass;
    
    int recv_buf_len = p.sendClientBufLen;
    char recv_buf[recv_buf_len+1] = "";
    
    int send_buf_len = p.sendServerBufLen;
    char send_buf[send_buf_len+1] = "";

    //server recive
    strcpy(recv_buf,"stop");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "stop" )
        cout << "false 1" << endl;
    
    strcpy(recv_buf,"enter_p   500");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "enter_p" || player_param.money != 500)
        cout << "false 2" << endl;
    
    strcpy(recv_buf,"enter_c   password");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "enter_c" || pass != "password")
        cout << "false 3" << endl;
    
    strcpy(recv_buf,"bet       odd 500");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "bet" || player_param.bet.betValue != "odd" || player_param.bet.money != 500)
        cout << "false 4" << endl;
    
    strcpy(recv_buf,"bet       number 15 500");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "bet" || player_param.bet.betValue != "number" || player_param.bet.money != 500 || player_param.bet.number != 15)
        cout << "false 5" << endl;
    
    strcpy(recv_buf,"rotate    ");
    p.convert(recv_buf, recv_buf_len, command, player_param, pass);
    if ( command != "rotate" )
        cout << "false 6" << endl;
    
    //server send
    
    Bet bet(BET_TYPE::number, 2047483647, 2147483647);
    Player pl;
    pl.bet = bet;
    pl.last_bet = 1047483647;
    pl.last_win = 1147483647;
    pl.money = 1247483647;
    map<SOCKET, Player> pls;
    string res;
    
    pls.insert(make_pair(1347483647, pl));
    pls.insert(make_pair(1447483647, pl));
    pls.insert(make_pair(1547483647, pl));
    pls.insert(make_pair(1647483647, pl));
    res = p.convert("info", 1747483647, 1847483647, 1947483647, pls,  "some_error");
    cout << "res len: " << res.length() << endl;;
    cout << res << "!!!" << endl;
    
    cout << "tests done !!!" << endl;
}

/*
 * 
 */
int main(int argc, char** argv) {
    try 
    {
        //MyServer server("127.0.0.1",27015);
        MyServer server("192.168.56.1",27015);
        server.start();     //run this, until server don't get command stop in his thread
    }
    catch (int &er) { cout << "some error: " << er << endl; }
    return 0;
}

