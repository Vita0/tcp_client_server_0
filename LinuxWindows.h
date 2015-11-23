#ifdef WINDOWS_OS
#include <Winsock2.h>
#else

#endif

#ifdef WINDOWS_OS
typedef SOCKET crossSocket;
#else
typedef int crossSocket;
#endif

