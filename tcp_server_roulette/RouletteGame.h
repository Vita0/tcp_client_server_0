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

const short MAX_PLAYER_COUNT = 4;
const short NO_VALUE = 37;
const string NO_VALUE_str = "37";

namespace BET_TYPE
{
    const int max_string_len = 6;
    const string onesecond = "1/2";
    const string twosecond = "2/2";
    const string even = "even";
    const string odd = "odd";
    const string number = "number";
    const string no_bet = "no_bet";
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
    Player()
            :money(0)
            ,last_bet(0)
            ,last_win(0)
            ,bet(Bet())
    {}
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
    Game() :m_maxPlayers(MAX_PLAYER_COUNT)
           ,m_croupier(0)
           ,m_croupierPassword("password")
           ,m_rouletteValue(NO_VALUE)
    {};
    virtual ~Game(){};
    
    short getValue() {
        return m_rouletteValue;
    }
    
    map<SOCKET,Player> getPlayers() {
        return m_players;
    }
    
    SOCKET getCroupier() {
        return m_croupier;
    }
    
    bool checkCroupierPassword(string s) {
        return s == m_croupierPassword;
    }
    
    void addPlayer(SOCKET sock, int money, string &error)
    {
        if (m_players.find(sock) != m_players.end())
        {
            return;
        }
        if (m_players.size() == 4)
        {
            error += "max players are plaing now\n";
            return;
        }
        if (money <= 0 || money > 10000)
        {
            error += "invalid money";
            return;
        }
        Player lol;
        lol.money = money;
        lol.last_bet = 0;
        lol.last_win = 0;
        Bet bet;
        lol.bet = bet;
        m_players.insert(pair<SOCKET,Player>(sock, lol));
    }
    
    void addCroupier(SOCKET sock, string const &pass, string &error)
    {
        if (m_croupier != 0)
            error += "croupier plase is busy\n";
        else if ( !checkCroupierPassword(pass) )
            error += "invalid password\n";
        else
            m_croupier = sock;
        return;
    }
    
    void delPlayer(SOCKET sock)
    {
        if (sock == m_croupier) {
            m_croupier = 0;
        }
        m_players.erase(sock);
    }
    
    bool isNoBets(const SOCKET sock) {
        return m_players.at(sock).bet.betValue == BET_TYPE::no_bet;
    }
    
    void setBet(const Bet &bet, SOCKET sock, string &error) {
        if (!isNoBets(sock)) {
            error += "you can't do more then one bet\n";
        }
        else if (bet.money <= m_players.at(sock).money &&
                (bet.number >=0 && bet.number < NO_VALUE || bet.betValue != BET_TYPE::number)) {
            m_players.at(sock).bet = bet;
            m_players.at(sock).money -= bet.money;
        }
        else {
            error += "wrong bet\n";
        }
        return;
    }
    
    void doBets() {
        m_rouletteValue = 15; //TODO rand
        bool odd = m_rouletteValue%2;
        bool onesecond = m_rouletteValue<(NO_VALUE-1)/2;
        
        for(auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            bool number = m_rouletteValue == it->second.bet.number;
            int koef=0;
            
            if (it->second.bet.betValue == BET_TYPE::odd) {
                if (odd) koef = 2; 
            }
            else if (it->second.bet.betValue == BET_TYPE::even) {
                if (!odd) koef = 2; 
            }
            else if (it->second.bet.betValue == BET_TYPE::onesecond) {
                if (onesecond) koef = 2; 
            }
            else if (it->second.bet.betValue == BET_TYPE::twosecond) {
                if (!onesecond) koef = 2; 
            }
            else if (it->second.bet.betValue == BET_TYPE::number) {
                if (number) koef = 36;
            }

            int win = koef * it->second.bet.money;
            it->second.money += win;
            it->second.last_bet = it->second.bet.money;
            it->second.last_win = win;
            it->second.bet = Bet();
        }
    }
};

#endif	/* ROULETTEGAME_H */

