/*
 * Basics.h
 * Include basic/default output/input support: cout/cin, mainly for debug
 * Set basic constants
 */
#ifndef BASICS_H
#define BASICS_H

#include <unistd.h>

#include <iostream>
using std::cout;
using std::cin;


// constants
const int BUFMAXSIZE			= 16384; //8192;

char IP_LOCALHOST[]				= "127.0.0.1";
const int PORT_MARKETDATA		= 8000;
const int PORT_ORDER_CG		= 8001; // order from clientGateway
const int PORT_ORDER_REQ		= 8002; // order request from sort
const int PORT_ORDER_MES		= 8003; // order message from marketServer

#include <mutex>
using std::mutex;
using std::lock_guard;

template <class Type>
class Mutexed {
public:
	Type obj;
	bool isNew;
	mutex mtx;
	mutex isNewMtx;
};

struct Order { // order from clientGateway
	int symbol;
	int side;
	long price;
	long qty;
};

#include <queue>
using std::queue;
class OrderQueue {
public:
	OrderQueue() {}
	~OrderQueue() {}
	void Push(const Order &ord) {
		ordQ.push(ord);
	}
	Order &Front() {
		return ordQ.front();
	}
	void Pop() {
		ordQ.pop();
	}
	bool Empty() {
		return ordQ.empty();
	}
private:
	queue<Order> ordQ;
};

#endif
