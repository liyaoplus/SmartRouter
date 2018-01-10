#include <string>
#include <fstream> 
#include <queue>

#include <stdio.h>

#include <thread>
using std::thread;

#include "Basics.h"
#include "MesHandleClass.h"
#include "EpollClass.h"
#include "Fix.h"

using std::string;
using std::ifstream;
using std::queue;


/* All symbols:
    Apple
    Facebook
    Twitter
    Amazon
    Microsoft
    Google
    Youtube
    Yelp

    FIX message:
        order_id, price, qty, symbol, side
*/

void orderRecv()
{
	OrderRecvMesHandle mesHandle;
    Server orderRecv(IP_LOCALHOST, PORT_ORDER_REQ, &mesHandle);
    while (!orderRecv.Bind()) {
		sleep(3);
	}
    orderRecv.Listen();
    return;
}

void sendPrice(int market, int symbols, int side, long price, long qty, Client &PriceSender)
{
    string message = "";
    message += "35=6";
    message = message + ",10=" + std::to_string(market);
    message = message + ",55=" + std::to_string(symbols);
    message = message + ",54=" + std::to_string(side);
    message = message + ",31=" + std::to_string(price);
    message = message + ",32=" + std::to_string(qty);
    /*char buf[64];
    memset(buf, 0, 64);
    message.copy(buf, message.length(), 0);
	std::cout << "sendPrice buf: " << buf << "\n";*/
    PriceSender.Send((void *)message.data(), message.length());
    return;
}

void sendMessage(int market, int symbols, int side, long price, long qty, long leaveQty, int id, long **allPrice[3][8][2], int state, Client &FixSender)
{
    string message;
    char buf[64];
    memset(buf, 0, 64);
    int len;
    message = codeMarketMessage(market, symbols, side, price, qty, leaveQty, id, state);
    /*len = message.length();
    message.copy(buf, len, 0);*/
    FixSender.Send((void *)message.data(), message.length());
    return;
}

void bookingHandle(int market, int symbols, int side, long price, long qty, int id, long **allPrice[3][8][2], Client &FixSender)
{
    int length, remainQty;
    length = allPrice[market][symbols][side][0][0];
    remainQty = qty;
    //std::cout<<length;
    for (int i=1; i<length+1; i++)
    {
        if (side == 0)
            if (allPrice[market][symbols][side][i][0]<=price)
            {
                if (remainQty>allPrice[market][symbols][side][i][1])
                {
                    remainQty -= allPrice[market][symbols][side][i][1];
                    sendMessage(market, symbols, side, allPrice[market][symbols][side][i][0], allPrice[market][symbols][side][i][1], remainQty, id, allPrice, 1, FixSender);
                    allPrice[market][symbols][side][i][1] = 0;
                }
                else
                {
                    sendMessage(market, symbols, side, allPrice[market][symbols][side][i][0], remainQty, 0, id, allPrice, 2, FixSender);
                    allPrice[market][symbols][side][i][1] -= remainQty;
                    remainQty = 0;
                }
            }
        if (side == 1)
            if (allPrice[market][symbols][side][i][0]>=price)
            {
                if (remainQty>allPrice[market][symbols][side][i][1])
                {
                    remainQty -= allPrice[market][symbols][side][i][1];
                    sendMessage(market, symbols, side, allPrice[market][symbols][side][i][0], allPrice[market][symbols][side][i][1], remainQty, id, allPrice, 1, FixSender);
                    allPrice[market][symbols][side][i][1] = 0;
                }
                else
                {
                    sendMessage(market, symbols, side, allPrice[market][symbols][side][i][0], remainQty, 0, id, allPrice, 2, FixSender);
                    allPrice[market][symbols][side][i][1] -= remainQty;
                    remainQty = 0;
                }
            }
        if (remainQty == 0)
            break;
    }
    if (remainQty)
        sendMessage(market, symbols, side, price, remainQty, remainQty, id, allPrice, 4, FixSender);
    return;
}

int main() 
{
	thread ordReqListen(orderRecv);
	ordReqListen.detach();
	
    // Initial the price of symbols in each market
    long **allPrice[3][8][2]; //market;symbol;side;

    ifstream infile;
    Client PriceSender(IP_LOCALHOST, PORT_MARKETDATA);
    while (!PriceSender.Connect()) {
		sleep(3);
		cout << "Waiting the Sort to Set Up ......\n";
	}
	cout << "MarketServer Send Price Connection is Established.\n";

	char current_path[BUFMAXSIZE];
	int cnt = readlink("/proc/self/exe", current_path, BUFMAXSIZE);
	for (int i = cnt; i >= 0; --i) {
		if (current_path[i] == '/') {
			current_path[i] = '\0';
			break;
		}
	}
	std::string currentPath(current_path);
	std::cout << currentPath;
    for (int i = 0; i<3; i++) {
        for (int j = 0; j<8; j++) {
            for (int k = 0; k<2; k++) {
                string content;
				//std::string tmp = "/home/liyao/marketService/price/"+SYMBOL[j]+"_"+MARKET[i]+"_"+SIDE_LC[k];
				std::string tmp = currentPath + "/savedMarketData/" + SYMBOL[j] + "_" + MARKET[i] + "_" + SIDE_LC[k];
				std::cout << "Filename: " << tmp << '\n';
                infile.open(tmp);
				if (!infile.is_open()) {
					std::cout << "File open fail.\n";
					break;
				}
                queue<string> price;
                while(std::getline(infile,content))
                    price.push(content);
                infile.close();
                int length;
                length = price.size();
                allPrice[i][j][k] = new long*[length+1];
                for (int l=0; l<length+1; l++)
                    allPrice[i][j][k][l] = new long[2];
                allPrice[i][j][k][0][0] = allPrice[i][j][k][1][0] = length;
                int index = 1;
                long priceSymbol, qty;
                while(!price.empty())
                {
                    content = price.front();
                    priceSymbol = std::stol(content.substr(0, content.find(' ')));
                    qty = std::stol(content.substr(content.find(' ')+1, content.size()-content.find(' ')-1));
                    allPrice[i][j][k][index][0] = priceSymbol;
                    allPrice[i][j][k][index][1] = qty;
                    price.pop();
					//sleep(1);
                    sendPrice(i, j, k, priceSymbol, qty, PriceSender);
                    index++;
                }
				sleep(0.1);
                //std::cout<<length<<std::endl;
            }
		}
	}
	while (true) {}
    //Initial the client part of epoll
    /*Client FixSender(IP_LOCALHOST, PORT_BOOK_MES);
    while (!FixSender.Connect()) {
		sleep(1);
        continue;
	}
    queue<string> orderQueue;
    string order;
    while(true)
    {
        if(orderQueue.empty())
            break;

        break;
    }
    bookingHandle(0, 3, 0, 23914, 10000, 0, allPrice, FixSender);
    for (int i = 0; i<3; i++) {
        for (int j = 0; j<8; j++)
            for (int k = 0; k<2; k++)
            {
                for (int l=0; l < 10; l++)
                    delete []allPrice[i][j][k][l];
                delete []allPrice[i][j][k];
            } 
	}
	*/
	return 0;
}
