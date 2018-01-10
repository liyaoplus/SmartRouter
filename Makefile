.PHONY: all

all: marketServer sortMain clientGateway

marketServer: Basics.h EpollClass.h Fix.h marketServer.cpp
	g++ -pthread marketServer.cpp -o marketServer

sortMain: Basics.h EpollClass.h Fix.h MarketDataList.h MesHandleClass.h SymbolThread.h sortMain.cpp
	g++ -pthread sortMain.cpp -o sortMain

clientGateway: Basics.h EpollClass.h Fix.h clientGateway.cpp
	g++ -pthread clientGateway.cpp -o clientGateway


.PHONY: clean
clean:
	rm clientTest serverTest marketServer sortMain clientGateway
