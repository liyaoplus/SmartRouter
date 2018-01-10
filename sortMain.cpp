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
		sleep(1);
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

/*void testClientFun(Mutexed<int> *pmId, Mutexed<long> *pmPrice) {
	Mutexed<int> &mId = *pmId;
	Mutexed<long> &mPrice = *pmPrice;
	while (true) {
		lock_guard<mutex> lock1_(mId.isNewMtx);
		if (!mId.isNew) {
			continue;
		}
		{
			lock_guard<mutex> lock1(mId.mtx);
			lock_guard<mutex> lock2(mPrice.mtx);
			cout << "thread recv: " << mId.obj << '\t' << mPrice.obj << '\n';
		}
		mId.isNew = false;
	}
}
void testSortFun(Mutexed<int> *pmId, Mutexed<long> *pmPrice) {
	SortMesHandle mesHandle(pmId, pmPrice);
	Server sortMarketDataRecv(IP_LOCALHOST, PORT_MARKETDATA, &mesHandle);
	while (!sortMarketDataRecv.Bind()) {
		sleep(1);
	}
	sortMarketDataRecv.Listen();
}*/

int main() {
	const int symbolN = 1;
	Mutexed<MarketDataList> mDLA[symbolN];
	Mutexed<OrderQueue> mOQA[symbolN];

	thread taskAp(symbolFunInThread, &mDLA[0], &mOQA[0], 0);
	taskAp.detach();

	thread taskOrderListen(orderListenFunInThread, mOQA, symbolN);
	taskOrderListen.detach();

	sortMarketDataListen(mDLA, symbolN);

	/*Mutexed<int> mId;
	Mutexed<long> mPrice;
	mId.isNew = false;
	thread taskTest(testClientFun, &mId, &mPrice);
	testSortFun(&mId, &mPrice);*/
	//Init();

	/*
    taskAp.detach();
    taskFa.detach();
    taskTw.detach();
    taskAm.detach();
    taskMi.detach();
    taskGo.detach();
    taskYo.detach();
    taskYe.detach();
	*/

	//thread taskOrderListen(orderListenFunInThread, &orderQueue, &orderQueue_mtx);
	//taskOrderListen.detach();

	//sortMarketDataListen(buyDataIsNewPA, bDIN_mtxPA, buyNewDataListPA, bNDL_mtxPA);

	return 0;
}
