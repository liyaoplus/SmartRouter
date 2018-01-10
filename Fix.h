#ifndef FIX_H
#define FIX_H

#include <string>
using std::string;
#include "Basics.h"

const string MARKET[3]	= {"NYSE", "IEX", "NASDAQ"};
const string SYMBOL[8]	= {"Apple", "Facebook", "Twitter", "Amazon", "Microsoft", "Google", "Youtube", "Yelp"};
const string SIDE[2]	= {"SELL", "BUY"};
const string SIDE_LC[2]	= {"sell", "buy"};
const string STATE[5]	= {"null", "partial fill", "fill", "null", "cancel"};

const int SYMBOL_N		= 8;
const int MARKET_N		= 3;

struct OrderInfo
{
    int market;         //10:market 0~2:"NYSE", "IEX", "NASDAQ"
    int symbol;         //55:symbol 0~7:"Apple", "Facebook", "Twitter", "Amazon", "Microsoft", "Google", "Youtube", "Yelp"
    int side;           //54:side   0~1:"sell","buy"
    long price;         //31/44: price
    long qty;           //38/32: qty
    long leaveQty;      //151: leaveQty
    int id;             //11: orderid
    int state;          //150: 1 partial fill 2 fill 4 cancel

	OrderInfo()
	: market(0), symbol(0), side(0), price(0), qty(0), leaveQty(0), id(0), state(0)
	{}
};

void printOrder(OrderInfo &ord) {
	cout << "market: " << MARKET[ord.market] << ", "
		 << "symbol: " << SYMBOL[ord.symbol] << ", "
		 << "side: " << SIDE[ord.side] << ", "
		 << "price: " << ord.price/100.0 << ", "
		 << "qty: " << ord.qty << "\n";
}

string codeMarketMessage(int market, int symbols, int side, long price, long qty, long leaveQty, int id, int state)
{
    string message = "";
    message += "35=8,150=";
    message += std::to_string(state);
    message = message + ",11=" + std::to_string(id);
    message = message + ",10=" + std::to_string(market);
    message = message + ",55=" + std::to_string(symbols);
    message = message + ",54=" + std::to_string(side);
    message = message + ",31=" + std::to_string(price);
    message = message + ",32=" + std::to_string(qty);
    message = message + ",151=" + std::to_string(leaveQty);
    return message;
}

string codeSortMessage(int market, int symbols, int side, long price, long qty, int id)
{
    string message = "";
    message += "35=7";
    message = message + ",11=" + std::to_string(id);
    message = message + ",10=" + std::to_string(market);
    message = message + ",55=" + std::to_string(symbols);
    message = message + ",54=" + std::to_string(side);
    message = message + ",31=" + std::to_string(price);
    message = message + ",32=" + std::to_string(qty);
    return message;
}

string codeOrderMessage(int symbols, int side, long price, long qty)
{
    string message = "";
	message += "35=6";
    message = message + ",55=" + std::to_string(symbols);
    message = message + ",54=" + std::to_string(side);
    message = message + ",31=" + std::to_string(price);
    message = message + ",32=" + std::to_string(qty);
    return message;
}

OrderInfo decodeMessage(string message)
{
    OrderInfo orderInfo; 
    string buf, mid;
    long tag, value;
    bool finished = false;
    int index;
    while(true)
    {
        index = message.find(",");
        if(index==-1)
            finished = true;
        buf = message.substr(0,index);
        message = message.substr(index+1, message.size()-index-1);
        tag = std::stol(buf.substr(0, buf.find("=")));
        value = std::stol(buf.substr(buf.find("=")+1, buf.size()-buf.find("=")-1));
        //std::cout<<tag<<' '<<value<<std::endl;
        switch(tag)
        {
			case 10: orderInfo.market = value;
            case 11: orderInfo.id = value;
            case 55: orderInfo.symbol = value;
            case 54: orderInfo.side = value;
            case 31: orderInfo.price = value;
            case 44: orderInfo.price = value;
            case 38: orderInfo.qty = value;
            case 32: orderInfo.qty = value;
            case 151: orderInfo.leaveQty = value;
            case 150: orderInfo.state = value;
        }
        if(finished)
            break;
    }
    return orderInfo;
}

int decode(char *raw, OrderInfo &ret) {
    string buf, mid, message = raw;
	//cout << "mes: " << message << "\n";
	int size = message.size();

    long tag, value;
    bool finished = false;
    int index;
    while(true)
    {
        index = message.find(",");
        if(index==-1)
            finished = true;
        buf = message.substr(0,index);
        message = message.substr(index+1, message.size()-index-1);
        tag = std::stol(buf.substr(0, buf.find("=")));
        value = std::stol(buf.substr(buf.find("=")+1, buf.size()-buf.find("=")-1));
        //std::cout<<tag<<' '<<value<<std::endl;
        switch(tag)
        {
			case 10: ret.market = value;
            case 11: ret.id = value;
            case 55: ret.symbol = value;
            case 54: ret.side = value;
            case 31: ret.price = value;
            case 44: ret.price = value;
            case 38: ret.qty = value;
            case 32: ret.qty = value;
            case 151: ret.leaveQty = value;
            case 150: ret.state = value;
        }
        if(finished)
            break;
    }
	return size;
}
void decodeStream(char *raw, const int maxSize) {
	OrderInfo ord;
	for (int i = 0; i < maxSize; ) {
		if (raw[i] == '\n') {
			++i;
			continue;
		}
		if (raw[i] == '\0') {
			++i;
			continue;
		}
		// else
		cout << "i: " << i << " = " << (int)raw[i] << "\n";
		int cnt = decode(&raw[i], ord);
		cout << "cnt: " << cnt << "\n";
		i += cnt;
		printOrder(ord);
	}
}


#endif
