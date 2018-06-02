# SmartRouter
*By Jan. 11, 2018*

------

## About
Course "Financial Support Computing (金融服务计算)" Group Project.

This project is an immature framework designed for doing High Frequency Trading (HFT).

## Code
- All in C++
- Only support building in Linux
- Using **pthread** for multi-thread support
- Using **epoll** for quick network response
- Using STL's **\<mutex\>** to handle data race
- In savedMarketData directory: simulated stock prices
- Congiguration (example: port definition): in Basics.h
- Some functions to be completed

## Build
- Build *all*: make *all*
- Build *<cpp_filename>*: make *<cpp_filename>*
- Clean: make *clean*

## Usage
- clientGateway: the buyers' interface
- sortMain: the center to handle trades
- marketServer: the market simulater
- *serverTest: a price sender to test the system*

## Work Division
- marketServer: by 李能([@lnfjpt](https://github.com/lnfjpt))
- epoll: by 马启晨
- sort: by 李姚([@liyaoplus](https://github.com/liyaoplus))
- clientGateway: by 侯易初, 李一人
