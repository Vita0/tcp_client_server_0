#include "ClientSidePlayer.h"

ClientSidePlayer::ServerSidePlayer(SOCKET sock)
    :Player(sock)
{
}

ClientSidePlayer::~ServerSidePlayer()
{
}

void ClientSidePlayer::my_recv()
{
    cout << "lol" << endl;
}

void ClientSidePlayer::my_send()
{
}
