/*
 * sortMain.cpp
 * sort implementation
 */

#include <thread>
using std::thread;

#include "Basics.h"
#include "Fix.h"
#include "MarketDataList.h"
#include "MesHandleClass.h"
#include "EpollClass.h"
#include "SymbolThread.h"


void sortMarketDataListen(Mutexed<MarketDataList> *pmDL, int n) {
	SortMesHandle mesHandle(pmDL, n);
	Server sortMarketDataRecv(IP_LOCALHOST, PORT_MARKETDATA, &mesHandle);
	while (!sortMarketDataRecv.Bind()) {
		sleep(3);
	}
	sortMarketDataRecv.Listen();
}

void symbolFunInThread(Mutexed<MarketDataList> *pmDL, Mutexed<OrderQueue> *pmOQ, int i) {
	SymbolThread symbolDecision(pmDL, pmOQ, i);
	symbolDecision.Run();
}

void orderListenFunInThread(Mutexed<OrderQueue> *pmOQ, int n) {
	SortOrderMesHandle mesHandle(pmOQ, n);
	Server sortOrderRecv(IP_LOCALHOST, PORT_ORDER_CG, &mesHandle);
	while (!sortOrderRecv.Bind()) {
		sleep(1);
	}
	sortOrderRecv.Listen();
}


int main() {
	const int symbolN = SYMBOL_N;
	Mutexed<MarketDataList> mDLA[symbolN];
	Mutexed<OrderQueue> mOQA[symbolN];

	thread taskAp(symbolFunInThread, &mDLA[0], &mOQA[0], 0);
	thread taskFa(symbolFunInThread, &mDLA[1], &mOQA[1], 1);
	thread taskTw(symbolFunInThread, &mDLA[2], &mOQA[2], 2);
	thread taskAm(symbolFunInThread, &mDLA[3], &mOQA[3], 3);
	thread taskMi(symbolFunInThread, &mDLA[4], &mOQA[4], 4);
	thread taskGo(symbolFunInThread, &mDLA[5], &mOQA[5], 5);
	thread taskYo(symbolFunInThread, &mDLA[6], &mOQA[6], 6);
	thread taskYe(symbolFunInThread, &mDLA[7], &mOQA[7], 7);
    taskAp.detach();
    taskFa.detach();
    taskTw.detach();
    taskAm.detach();
    taskMi.detach();
    taskGo.detach();
    taskYo.detach();
    taskYe.detach();

	thread taskOrderListen(orderListenFunInThread, mOQA, symbolN);
	taskOrderListen.detach();

	sortMarketDataListen(mDLA, symbolN);

	return 0;
}
