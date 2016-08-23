ROOT_PATH=$(shell pwd)
INCLUDE=-I$(ROOT_PATH)/../lib/include
LIB_PATH=-L$(ROOT_PATH)/../lib/lib
LIB=-lmysqlclient

.PHONY:all
all:cgi_insert cgi_select

cgi_insert:cgi_insert.cpp sql_api.cpp
	@g++ -o $@ $^ $(INCLUDE) $(LIB_PATH) $(LIB)


cgi_select:cgi_select.cpp sql_api.cpp
	@g++ -o $@ $^ $(INCLUDE) $(LIB_PATH) $(LIB)

.PHONY:clean
clean:
	@rm -f cgi_insert cgi_select
