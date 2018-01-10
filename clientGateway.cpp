/*
 * clientGateway.cpp
 *
 * To send order request to the sort
 */

#ifndef CLIENTGATEWAY_CPP
#define CLIENTGATEWAY_CPP

#include "Basics.h"
#include "Fix.h"
#include "EpollClass.h"

int main() {
	Client ordSender(IP_LOCALHOST, PORT_ORDER_CG);
	while (!ordSender.Connect()) {
		sleep(3);
		cout << "Waiting the Sort to Set Up ......\n";
	}
	cout << "ClientGateway's Order Connection Established.\n";

	int symbol;
	double price;
	long qty;
	while (true) {
		cout << '\n';
		for (int i = 0; i < SYMBOL_N; ++i) {
			cout << SYMBOL[i] << ": " << i << " , ";
		}
		cout << "\nChoose a symbol's stock to buy (in number): ";
		cin >> symbol;
		
		cout << "Price: ";
		cin >> price;

		cout << "Qty (100/unit): ";
		cin >> qty;

		// side: 1 (BUY)

		string message;
		message = codeOrderMessage(symbol, 1, long(price*100), qty);
		ordSender.Send((void *)message.data(), message.length());
	}
}


#endif
