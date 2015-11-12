/* 
 * File:   RouletteGame.h
 * Author: Vita
 *
 * Created on 12 ноября 2015 г., 19:29
 */

#ifndef ROULETTEGAME_H
#define	ROULETTEGAME_H

#include <map>
#include <utility> //pair

using namespace std;

const short NO_VALUE = 37;

namespace BET_TYPE
{
    const int max_string_len = 6;
    const string onesecond = "1/2";
    const string twosecond = "2/2";
    const string even = "even";
    const string odd = "odd";
    const string number = "number";
    const string no_bet = "no bet";
//    const string onethird = "1/3";
//    const string twothird = "2/3";
//    const string treethird = "3/3";
}

struct Bet
{
    Bet(string betValue, int number, int money)
       :betValue(betValue)
       ,number(number)
       ,money(money)
    {}
    Bet()
       :betValue(BET_TYPE::no_bet)
       ,number(NO_VALUE)
       ,money(0)
    {}
    string betValue;
    int number;
    int money;
};

struct Player
{
    int money;
    int last_bet;
    int last_win;
    Bet bet;
};

class Game
{
private:
    const short m_maxPlayers;
    SOCKET m_croupier;
    const string m_croupierPassword;
    short m_rouletteValue;

    map<SOCKET,Player> m_players;
public:
    Game() :m_maxPlayers(4)
           ,m_croupier(0)
           ,m_croupierPassword("password")
           ,m_rouletteValue(NO_VALUE)
    {};
    virtual ~Game(){};
    //short getPlayersCount() { return m_players.size(); }
    bool checkCroupierPassword(string s) { return s == m_croupierPassword; }
    void addPlayer(SOCKET sock, int money)
    {
        Player lol;
        lol.money = money;
        lol.last_bet = 0;
        lol.last_win = 0;
        Bet bet;
        lol.bet = bet;
        m_players.insert(pair<SOCKET,Player>(sock, lol));
    }
    void delPlayer(SOCKET sock)
    {
        if (sock == m_croupier) {
            m_croupier = 0;
        }
        m_players.erase(sock);
    }
    bool isNoBets(SOCKET sock) {
        return m_players.at(sock).bet.money > 0;
    }
    
    void setBet(string betValue, int number, int bet_money, SOCKET sock) {
        m_players.at(sock).bet = Bet(betValue, number, bet_money);
        m_players.at(sock).money -= bet_money;
        return;
    }
    
    void doBet(short rouletteValue, SOCKET sock) {
        auto it = m_players.find(sock);
        if (it == m_players.end())
            return;
        
        bool odd = rouletteValue%2;
        bool onesecond = rouletteValue<(NO_VALUE-1)/2;
        bool number = rouletteValue == it->second.bet.number;
        int koef=0;
        if (it->second.bet.betValue == BET_TYPE::odd) {
            if (odd) koef = 2; 
        } else if (it->second.bet.betValue == BET_TYPE::even) {
            if (!odd) koef = 2; 
        } else if (it->second.bet.betValue == BET_TYPE::onesecond) {
            if (onesecond) koef = 2; 
        } else if (it->second.bet.betValue == BET_TYPE::twosecond) {
            if (!onesecond) koef = 2; 
        } else if (it->second.bet.betValue == BET_TYPE::twosecond) {
            if (number) koef = 36;
        }
        
        m_rouletteValue = rouletteValue;
        int win = koef * it->second.money;
        it->second.money += win;
        it->second.last_bet = it->second.bet.money;
        it->second.last_win = win;
        it->second.bet = Bet();
    }
};

#endif	/* ROULETTEGAME_H */

