/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 27 октября 2015 г., 11:00
 */

#pragma comment(lib,"wsock32.lib")

#include "MyServer.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    try 
    {
        MyServer server("127.0.0.1",27015);
        server.start();     //run this, until server don't get command stop in his thread
    }
    catch (...) { cout << "some error" << endl; }
    return 0;
}

