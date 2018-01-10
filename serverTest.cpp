#include "Basics.h"
#include "EpollClass.h" 
#include "MesHandleClass.h"

#define TestingMesHandle TestMesHandle

const int PORT			= PORT_MARKETDATA;

int main() {
	TestingMesHandle mh;
	Server a(IP_LOCALHOST, PORT, &mh);
	a.Bind();
	a.Listen();

	return 0;
}
