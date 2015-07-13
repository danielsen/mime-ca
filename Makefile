# libmime-ca build services
CC=g++
CCFLAGS=-std=c++11
COFLAGS=-fPIC -std=c++11 -c
HDR=src/mime.h src/mimecode.h src/mimechar.h
CPP=src/mime.cpp src/mimecode.cpp src/mimechar.cpp src/mimetype.cpp
TGT=build/Release

%.o: src/%.cpp $(HDR)
	@rm -fr $(TGT)
	@mkdir -p $(TGT)
	$(CC) $(COFLAGS) -o $@ $<

all: mime.o mimecode.o mimechar.o mimetype.o
	$(CC) -shared -o $(TGT)/libmime-ca.so *.o

clean:
	rm -fr *.o build

libtest:
	$(CC) $(CCFLAGS) $(CPP) test/mimetest.cpp -o mimetest
