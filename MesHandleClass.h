/*
 * MesHandle.h
 * Handle messages received by the listener in network
 */
#ifndef MESHANDLECLASS_H
#define MESHANDLECLASS_H

#include "Basics.h"
#include "Fix.h"
#include "MarketDataList.h"

class MesHandle {
public:
	MesHandle() {}
	virtual void Handle(const void *addr) {}
	~MesHandle() {}
private:
};

class TestMesHandle: public MesHandle {
public:
	TestMesHandle() {}
	~TestMesHandle() {}
	void Handle(const void *addr) {
		char *raw = (char *)addr;
		OrderInfo ord;
		for (int i = 0; i < BUFMAXSIZE; ) {
			if (raw[i] == '\n') {
				++i;
				continue;
			}
			if (raw[i] == '\0') {
				++i;
				continue;
			}
			// else
			int cnt = decode(&raw[i], ord);
			cout << "Decode char cnt: " << cnt << "\n";
			i += cnt;
			cout << i << '\n';
			printOrder(ord);
		}
	}
};

class OrderRecvMesHandle: public MesHandle {
public:
	OrderRecvMesHandle()
	{}
	~OrderRecvMesHandle() {}
	void Handle(const void *addr) {
		char *raw = (char *)addr;
		OrderInfo ord;
		for (int i = 0; i < BUFMAXSIZE; ) {
			if (raw[i] == '\n' || raw[i] == '\0') {
				++i;
				continue;
			}
			// else
			int cnt = decode(&raw[i], ord);
			//cout << "Decode char cnt: " << cnt << "\n";
			i += cnt;
			//printOrder(ord);
			// process
			cout << "Recv Order Req: " << SIDE[ord.side] << ' ' 
				 << ord.qty << ord.symbol << '@'
				 << ord.price << " From " << MARKET[ord.market] << '\n';
		}
	}
private:
};

class SortOrderMesHandle: public MesHandle {
public:
	SortOrderMesHandle(Mutexed<OrderQueue> *pmOQ, int n)
	:pmOQ(pmOQ), symbolN(n)
	{}
	~SortOrderMesHandle() {}
	void Handle(const void *addr) {
		char *raw = (char *)addr;
		OrderInfo ord;
		Order order;
		for (int i = 0; i < BUFMAXSIZE; ) {
			if (raw[i] == '\n' || raw[i] == '\0') {
				++i;
				continue;
			}
			// else
			int cnt = decode(&raw[i], ord);
			//cout << "Decode char cnt: " << cnt << "\n";
			i += cnt;
			//printOrder(ord);
			if (ord.symbol >= symbolN) {
				continue;
			}
			order.symbol = ord.symbol;
			order.price = ord.price;
			order.qty = ord.qty;
			// process
			{
				lock_guard<mutex> lock(pmOQ[order.symbol].mtx);
				pmOQ[order.symbol].obj.Push(order);
				//cout << "Order recv and push: " << SYMBOL[order.symbol] << '\t'
				//	 << order.qty << " @ " << order.price/100.0 << '\n';
			}
		}
	}
private:
	Mutexed<OrderQueue> *pmOQ;
	int symbolN;
};

class SortMesHandle: public MesHandle {
public:
	SortMesHandle(Mutexed<MarketDataList> *pmDL, int n)
	:pmDL(pmDL), symbolN(n)
	{}
	~SortMesHandle() {}
	void Handle(const void *addr) {
		char *raw = (char *)addr;
		OrderInfo ord;
		for (int i = 0; i < BUFMAXSIZE; ) {
			if (raw[i] == '\n' || raw[i] == '\0') {
				++i;
				continue;
			}
			// else
			int cnt = decode(&raw[i], ord);
			//cout << "Decode char cnt: " << cnt << "\n";
			i += cnt;
			//printOrder(ord);
			// process
			int symbol = ord.symbol;
			if (symbol >= symbolN) {
				break;
			}
			{
				lock_guard<mutex> lock(pmDL[symbol].mtx);
				pmDL[symbol].obj.WriteMarketData(ord.market, ord.price, ord.qty);
				cout << ord.market << '\t' << ord.price << '\t' << ord.qty << '\n';
				//pmDL[symbol].obj.PrintMarketData();
			}
		}
	}

private:
	Mutexed<MarketDataList> *pmDL;
	int symbolN;
};

#endif
