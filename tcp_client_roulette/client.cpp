/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 11:00
 */

#pragma comment(lib,"wsock32.lib")

#include "MyClient.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    try
    {
        MyClient client("127.0.0.1",27015);
        client.start();
    }
    catch (int &er) { cout << "some error: " << er << endl; }
    return 0;
}

