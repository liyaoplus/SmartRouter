#ifndef MARKETDATANODE_H
#define MARKETDATANODE_H

struct MarketDataNode
{
	MarketDataNode(int mktId, long price, long qy):
		marketId(mktId), price(price), qty(qy)
	{}
	MarketDataNode():
	    prev(nullptr), next(nullptr)
    {}
	void ChangeQty(long newQty) {
		this->qty = newQty;
	}
	// pointers
	MarketDataNode *prev;
	MarketDataNode *next;

	// fix attributes
	int marketId;
	long price;
	long qty;

};

#endif
