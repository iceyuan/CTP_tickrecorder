CXX=g++
BIN=bin/HangQing
LIB=$(addprefix lib/lib,util.so)
CTPLIBS=$(addprefix libs/,thostmduserapi.so thosttraderapi.so)

all:$(LIB) $(BIN)
	$(info $(BIN))
bin/HangQing:cpp/HangQing.cpp lib/libutil.so
	$(CXX) cpp/HangQing.cpp $(CTPLIBS) -o bin/HangQing -I include -lutil -L lib
lib/libutil.so:include/util.h cpp/util.cpp
	$(CXX) cpp/util.cpp -fpic -shared -g -o lib/libutil.so -I include
clean:
	rm -rf bin/HangQing
