/* 
 * File:   main.cpp
 * Author: Vita
 *
 * Created on 20 сентября 2015 г., 0:06
 */


#include <windows.h>
#include "MyServer.h"

//#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"wsock32.lib")

using namespace std;

int main(int argc, char** argv) {
    try 
    {
        MyServer server("127.0.0.1",27015);
        server.start();
    }
    catch(...){}
    return 0;
}

