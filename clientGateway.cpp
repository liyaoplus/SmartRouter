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
		sleep(1);
	}
	int symbol;
	long price;
	long qty;
	while (true) {
		for (int i = 0; i < SYMBOL_N; ++i) {
			cout << SYMBOL[i] << ": " << i << " ";
		}
		cout << "\nChoose a symbol's stock to buy (in number): ";
		cin >> symbol;
		
		cout << "Price: ";
		cin >> price;

		cout << "Qty (100/unit): ";
		cin >> qty;

		// side: 1 (BUY)

		string message;
		message = codeOrderMessage(symbol, 1, price, qty);
		ordSender.Send((void *)message.data(), message.length());
	}
}


#endif
