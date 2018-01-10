/*
 * symbolThread.h
 * the thread to process a symbol's trade
 */
#ifndef SYMBOLTHREAD_H
#define SYMBOLTHREAD_H

#include <queue>
using std::queue;

#include "Basics.h"
#include "MarketDataList.h"
#include "Fix.h"
#include "EpollClass.h"

struct TradeBuf {
	long price;
	long qty;
	int marketId;
};

class SymbolThread {
public:
	SymbolThread(Mutexed<MarketDataList> *pmDL, Mutexed<OrderQueue> *pmOQ, int i)
	: mDL(*pmDL), mOQ(*pmOQ), symbol(i) {
		buyCurDataList = &mDL.obj;

		ordReqSend.Init(IP_LOCALHOST, PORT_ORDER_REQ);
		while (!ordReqSend.Connect()) {
			sleep(3);
		}
		cout << SYMBOL[symbol] << "'s Order Req Connection Established.\n";
	}
	~SymbolThread() {}

	void Run() {
		Order ord;
		while (true) {
			{
				lock_guard<mutex> lock_ord(mOQ.mtx);
				if (mOQ.obj.Empty()) {
					continue;
				}
				ord = mOQ.obj.Front();
				cout << "Recv CG Order: BUY " 
					 << ord.qty << ' ' << SYMBOL[ord.symbol] << '@' << ord.price/100.0 << '\n';
				mOQ.obj.Pop();
			}
			{
				lock_guard<mutex> lock(mDL.mtx);
				//buyCurDataList->PrintMarketData();

				// make decision
				BuyOrderDecision(ord);
			}
		}
	}

private:
	// shared
	Mutexed<MarketDataList> &mDL;
	Mutexed<OrderQueue> &mOQ;
	int symbol;

	MarketDataList *buyCurDataList;

	Client ordReqSend;
	

	long ProcessTradeBuf(TradeBuf *tradeBuf, int bufIdx, long leftQty, long buyPrice) {
		//cout << "In TradeBuf \n";

		long totQty = 0;
		for (int i = 0; i <= bufIdx; ++i) {
			totQty += tradeBuf[i].qty;
		}
		long tmpLeftQty = leftQty;
		for (int i = 0; i <= bufIdx; ++i) {
			long tradeQty = (leftQty*tradeBuf[i].qty)/totQty;
			long remainder = tradeQty%100;
			tradeQty = remainder == 0? tradeQty:(tradeQty - remainder + 100);
			tradeQty = tradeQty < tmpLeftQty? tradeQty:tmpLeftQty;
			tradeQty = tradeQty < tradeBuf[i].qty? tradeQty:tradeBuf[i].qty;
			tmpLeftQty -= tradeQty;
			if (tradeQty == 0) {
				break;
			}
			// Send trade message
			string message;
			message = codeSortMessage(tradeBuf[i].marketId, symbol, 1, buyPrice, tradeQty, 0);
			ordReqSend.Send((void *)message.data(), message.length());

			cout << "Send Order Req: " 
				 << tradeQty << ' ' << SYMBOL[symbol] << '@' << buyPrice 
				 << " From " << MARKET[tradeBuf[i].marketId] << '\n';

		}
		return tmpLeftQty;
	}

	void BuyOrderDecision(Order &order) {
		long price = order.price;
		long leftQty = order.qty;
		TradeBuf tradeBuf[MARKET_N];
		
		cout << "In Decision \n";

		// Just one pass
		int bufIdx = -1;
		long tmpLeftQty = leftQty;

		for (buyCurDataList->ListIdxInit(); buyCurDataList->ListIdxValid(); buyCurDataList->ListIdxNext()) {
			if (buyCurDataList->GetIdxNodePrice() <= price) { // trade may happen
				// Greedy method
				if (bufIdx < 0 || tradeBuf[bufIdx].price == buyCurDataList->GetIdxNodePrice()) { // append to the buf
					++bufIdx;
					tradeBuf[bufIdx].price = buyCurDataList->GetIdxNodePrice();
					tradeBuf[bufIdx].qty = buyCurDataList->GetIdxNodeQty();
					tradeBuf[bufIdx].marketId = buyCurDataList->GetIdxNodeMarketId();
				} else {
					// process the previous trade chance
					tmpLeftQty = ProcessTradeBuf(tradeBuf, bufIdx, tmpLeftQty, price);
					// refresh the tradeBuf
					bufIdx = 0;
					tradeBuf[bufIdx].price = buyCurDataList->GetIdxNodePrice();
					tradeBuf[bufIdx].qty = buyCurDataList->GetIdxNodeQty();
					tradeBuf[bufIdx].marketId = buyCurDataList->GetIdxNodeMarketId();
				}
			} else {
				break;
			}
		}
		// process the left trade chance
		tmpLeftQty = ProcessTradeBuf(tradeBuf, bufIdx, tmpLeftQty, price);

		// // listen the trade results from markets and update order leftQty
	}
};


#endif


