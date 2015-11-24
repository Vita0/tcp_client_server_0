#ifndef LINUXWINDOWS_H
#define LINUXWINDOWS_H

#ifdef WINDOWS_OS
#include <Winsock2.h>
#else

#endif

#ifdef WINDOWS_OS
typedef SOCKET crossSocket;
#else
typedef int crossSocket;
#endif

#ifdef WINDOWS_OS
#pragma comment(lib,"wsock32.lib")
//linker -lWs2_32
#else
//linker -pthread
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/stat.h> 
//#include <sys/un.h>
//#include <netdb.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>



#endif //LINUXWINDOWS_H
