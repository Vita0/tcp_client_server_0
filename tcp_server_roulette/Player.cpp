//#include "MyServer.h"
//#include <stdio.h>
//
//Player::Player(SOCKET sock, Game *game, mutex *game_mutex)
//    :m_socket(sock)
//    ,m_started(false)
//    ,m_game(game)
//    ,m_game_mutex(game_mutex)
//    ,m_isCroupier(false)
//{
//}
//
//Player::~Player()
//{
//    if ( m_recvThread != nullptr || m_sendThread != nullptr )
//    {
//        m_recvThread->join();
//        m_sendThread->join();
//    }
//}
//
//void Player::start()
//{
//    m_started = true;
//    m_recvThread = shared_ptr<thread> ( new thread(Player::myRecv, this));
//    m_sendThread = shared_ptr<thread> ( new thread(Player::mySend, this));
//}
//
//void Player::mySend()
//{
//    while (m_started)
//    {
//        this_thread::sleep_for(chrono::microseconds(1000000));
//        
//        m_game_mutex->lock();
//        vector<Game::GamePlayer> pls = m_game->getPlayerList();
//        SOCKET cr = m_game->getCroupier();
//        short rv = m_game->getRouletteValue();
//        short maxPls = m_game->m_maxPlayersCountValue;
//        m_game_mutex->unlock();
//        
//        const int sendbuflen = 380;
//        char sendbuf[sendbuflen+1];
//        const int inc = 10;
//        int idx = 0;
//        
//        //info
//        idx = 0;
//        strcpy(sendbuf+idx, "info");
//        
//        idx += inc;
//        sprintf(sendbuf+idx, "%d\0", m_socket);
//        idx += inc;
//        sprintf(sendbuf+idx, "%d\0", rv);
//        
//        idx += inc;
//        strcpy(sendbuf+idx, "croupier");
//        idx += inc;
//        sprintf(sendbuf+idx, "%d\0", cr);
//        
//        for(const Game::GamePlayer &i: pls)
//        {
//            idx += inc;
//            strcpy(sendbuf+idx, "player");
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.socket);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.money);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.last_bet);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.last_win);
//            idx += inc;
//            strcpy(sendbuf+idx, i.bet.betValue.c_str());
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.bet.number);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", i.bet.money);
//        }
//        for(int i = 0; i < maxPls - pls.size(); ++i)
//        {
//            idx += inc;
//            strcpy(sendbuf+idx, "player");
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", 0);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", 0);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", 0);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", 0);
//            idx += inc;
//            strcpy(sendbuf+idx, BET_TYPE::no_bet.c_str());
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", NO_VALUE);
//            idx += inc;
//            sprintf(sendbuf+idx, "%d\0", 0);
//        }
//        //cout << "send, idx = " << idx << endl;
//        int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//        if (iResult == SOCKET_ERROR) {
//            wprintf(L"send failed with error: %d\n", WSAGetLastError());
//            closesocket(m_socket);
//            throw 6;
//        }
//    }
//}
//
//void Player::myRecv()
//{
//    while (m_started)
//    {
//        const int recvbuflen = 70;
//        char recvbuf[recvbuflen+1];
//        int iResult = readn(m_socket, recvbuf, recvbuflen);
//        if ( iResult > 0 ) {
//            wprintf(L"%s\n",recvbuf);
//            wprintf(L"Bytes received: %d\n", iResult);
//        }
//        else if ( iResult == 0 ) {
//            wprintf(L"Connection closed\n");
//            m_started = false;
//        }
//        else {
//            wprintf(L"recv failed: %d\n", WSAGetLastError());
//            m_started = false;
//        }
//
//        const int buflen = 10;
//        char buf[buflen+1];
//        const int inc = 10;
//        int idx = 0;
//        
//        const int sendbuflen = 260;
//        char sendbuf[sendbuflen+1];
//        int sendidx = 0;
//        
//        if (strcmp(recvbuf+idx, "enter_p") == 0)
//        {
//            idx += inc;
//            int money = atoi(recvbuf+idx);
//            cout << "enter_p " << money << endl;
//            m_game_mutex->lock();
//            if (m_game->getPlayersCount() < m_game->m_maxPlayersCountValue)
//            {
//                m_game->addPlayer(m_socket, money);
//                m_game_mutex->unlock();
//            }
//            else
//            {
//                cout << "no lol" << endl;
//                m_game_mutex->unlock();
//                
//                strcpy(sendbuf+sendidx, "error");
//                sendidx += inc;
//                strcpy(sendbuf+sendidx, "maximum count of players are playing now");
//                int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//                if (iResult == SOCKET_ERROR) {
//                    wprintf(L"send failed with error: %d\n", WSAGetLastError());
//                    closesocket(m_socket);
//                    throw 6;
//                }
//            }
//        }
//        else if (strcmp(recvbuf+idx, "enter_c") == 0)
//        {
//            idx += inc;
//            string pass = recvbuf+idx;
//            cout << "enter_c " << pass << endl;
//            m_game_mutex->lock();
//            if (m_game->getCroupier() == 0)
//            {
//                cout << "lol" << endl;
//                if (m_game->checkCroupierPassword(pass))
//                {
//                    m_game->setCroupier(m_socket);
//                    m_game_mutex->unlock();
//                }
//                else
//                {
//                    cout << "no lol" << endl;
//                    m_game_mutex->unlock();
//
//                    strcpy(sendbuf+sendidx, "error");
//                    sendidx += inc;
//                    strcpy(sendbuf+sendidx, "croupier password is not correct");
//                    int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//                    if (iResult == SOCKET_ERROR) {
//                        wprintf(L"send failed with error: %d\n", WSAGetLastError());
//                        closesocket(m_socket);
//                        throw 6;
//                    }
//                }
//            }
//            else
//            {
//                cout << "no lol" << endl;
//                m_game_mutex->unlock();
//                
//                strcpy(sendbuf+sendidx, "error");
//                sendidx += inc;
//                strcpy(sendbuf+sendidx, "croupier place is busy");
//                int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//                if (iResult == SOCKET_ERROR) {
//                    wprintf(L"send failed with error: %d\n", WSAGetLastError());
//                    closesocket(m_socket);
//                    throw 6;
//                }
//            }
//        }
//        else if (strcmp(recvbuf+idx, "bet") == 0)
//        {
//            m_game_mutex->lock();
//            int my_money = m_game->getPlMoney(m_socket);
//            m_game_mutex->unlock();
//            idx += inc;
//            string bet_type = recvbuf+idx;
//            string er = "";
//            int money = 0;
//            int number = NO_VALUE;
//            if (bet_type == BET_TYPE::even || bet_type == BET_TYPE::odd 
//                || bet_type == BET_TYPE::onesecond || bet_type == BET_TYPE::twosecond)
//            {
//                idx += inc;
//                money = atoi(recvbuf+idx);
//                if (money<=0 || money>my_money)
//                    er = "money wrong!";
//            }
//            else if (bet_type == BET_TYPE::number)
//            {
//                idx += inc;
//                number = atoi(recvbuf+idx);
//                idx += inc;
//                if (number >=0 && number < NO_VALUE)
//                    er = "number wrong!";
//                money = atoi(recvbuf+idx);
//                if (money<=0 || money>my_money)
//                    er += " money wrong!";
//            }
//            else
//            {
//                er = "wrong bet type!";
//            }
//            if (er == "")
//            {
//                m_game_mutex->lock();
//                if (m_game->isNoBets(m_socket))
//                {
//                    m_game->setBet(bet_type,number,money,m_socket);
//                }
//                else
//                {
//                    er = "can't bet more, than one";
//                }
//                m_game_mutex->unlock();
//            }
//            if (er != "")
//            {
//                strcpy(sendbuf+sendidx, "error");
//                sendidx += inc;
//                strcpy(sendbuf+sendidx, er.c_str());
//                int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//                if (iResult == SOCKET_ERROR) {
//                    wprintf(L"send failed with error: %d\n", WSAGetLastError());
//                    closesocket(m_socket);
//                    throw 6;
//                }
//            }
//        }
//        else if (strcmp(recvbuf+idx, "bye") == 0)
//        {
//            continue;
//        }
//        else
//        {
//            strcpy(sendbuf+sendidx, "error");
//            sendidx += inc;
//            strcpy(sendbuf+sendidx, "wrong command!");
//            int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//            if (iResult == SOCKET_ERROR) {
//                wprintf(L"send failed with error: %d\n", WSAGetLastError());
//                closesocket(m_socket);
//                throw 6;
//            }
//        }
//    }
//}
//
//void Player::stop()
//{
//    const int sendbuflen = 260;
//    char sendbuf[sendbuflen+1];
//    strcpy(sendbuf, "stop");
//    int iResult = send( m_socket, sendbuf, sendbuflen, 0);
//    if (iResult == SOCKET_ERROR) {
//        wprintf(L"send failed with error: %d\n", WSAGetLastError());
//        closesocket(m_socket);
//        throw 6;
//    }
//    m_started = false;
//    m_recvThread->join();
//    m_sendThread->join();
//    m_recvThread = nullptr;
//    m_sendThread = nullptr;
//}