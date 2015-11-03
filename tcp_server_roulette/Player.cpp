#include "MyServer.h"

Player::Player(SOCKET sock)
    :m_socket(sock)
    ,m_started(false)        
{
}

Player::~Player()
{
    if ( m_recv_thread != nullptr || m_send_thread != nullptr )
    {
        m_recv_thread->join();
        m_send_thread->join();
    }
}

void Player::start()
{
    m_started = true;
    m_recv_thread = shared_ptr<thread> ( new thread(Player::recv, this));
    m_send_thread = shared_ptr<thread> ( new thread(Player::send, this));
}

void Player::send()
{
    while (m_started)
    {
        this_thread::sleep_for(chrono::seconds(1));
        my_send();
    }
}

void Player::recv()
{
    while (m_started)
    {
        this_thread::sleep_for(chrono::seconds(1));
        my_recv();
    }
}

void Player::stop()
{
    m_started = false;
    m_recv_thread->join();
    m_send_thread->join();
    m_recv_thread = nullptr;
    m_send_thread = nullptr;
}

ServerSidePlayer::ServerSidePlayer(SOCKET sock)
    :Player(sock)
{
}

ServerSidePlayer::~ServerSidePlayer()
{
}

void ServerSidePlayer::my_recv()
{
    cout << "lol" << endl;
}

void ServerSidePlayer::my_send()
{
}