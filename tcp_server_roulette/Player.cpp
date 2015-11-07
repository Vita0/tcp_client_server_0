#include "MyServer.h"
#include <stdio.h>

int readn(SOCKET fd, char *data, size_t data_len)
{
    int cnt;
    int res;
    
    cnt = data_len;
    while( cnt > 0 ) {
        res = recv(fd, data, cnt, 0);
        if ( res < 0 )
        {
            if ( errno == EINTR )
                continue;
            wprintf(L"recv failed: %d\n", WSAGetLastError());
            return -1;
        }
        if ( res == 0 )
            return data_len - cnt;
        data += res;
        cnt -= res;
    }
    return data_len;
}

Player::Player(SOCKET sock, Game *game, mutex *game_mutex)
    :m_socket(sock)
    ,m_started(false)
    ,m_game(game)
    ,m_game_mutex(game_mutex)
    ,m_isCroupier(false)
{
}

Player::~Player()
{
    if ( m_recvThread != nullptr || m_sendThread != nullptr )
    {
        m_recvThread->join();
        m_sendThread->join();
    }
}

void Player::start()
{
    m_started = true;
    m_recvThread = shared_ptr<thread> ( new thread(Player::myRecv, this));
    m_sendThread = shared_ptr<thread> ( new thread(Player::mySend, this));
}

void Player::mySend()
{
    while (m_started)
    {
        this_thread::sleep_for(chrono::microseconds(1000000));
        cout << "send" << endl;
        
        m_game_mutex->lock();
        vector<Game::GamePlayer> pls = m_game->getPlayerList();
        SOCKET cr = m_game->getCroupier();
        m_game_mutex->unlock();
        
        const int recvbuflen = 260;
        char recvbuf[recvbuflen+1];
        const int inc = 10;
        int idx = 0;
        
        //info
        idx = 0;
        strcpy(recvbuf+idx, "info");
        
        idx += inc;
        sprintf(recvbuf+idx, "%d", m_socket);
        
        idx += inc;
        strcpy(recvbuf+idx, "croupier");
        idx += inc;
        sprintf(recvbuf+idx, "%d", cr);
        
        for(const Game::GamePlayer &i: pls)
        {
            idx += inc;
            strcpy(recvbuf+idx, "player");
            idx += inc;
            sprintf(recvbuf+idx, "%d", i.socket);
            idx += inc;
            sprintf(recvbuf+idx, "%d", i.money);
        }
        for(int i = 0; i < m_game->m_maxPlayersCountValue - pls.size(); ++i);
        {
            idx += inc;
            strcpy(recvbuf+idx, "player");
            idx += inc;
            sprintf(recvbuf+idx, "%d", 0);
            idx += inc;
            sprintf(recvbuf+idx, "%d", 0);
        }
        int iResult = send( m_socket, recvbuf, recvbuflen, 0);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(m_socket);
            throw 6;
        }
    }
}

void Player::myRecv()
{
    while (m_started)
    {
        this_thread::sleep_for(chrono::seconds(1));
        
    }
}

void Player::stop()
{
    m_started = false;
    m_recvThread->join();
    m_sendThread->join();
    m_recvThread = nullptr;
    m_sendThread = nullptr;
}
