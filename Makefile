ROOT_PATH=$(shell pwd)
BIN=httpd
CC=gcc
SRC=httpd.c
OBJ=$(SRC:.c=.o)
LIB=-lpthread
CGI=$(ROOT_PATH)/htdoc/cgi $(ROOT_PATH)/mysql_connect

.PHONY:all
all:
	$(BIN) cgi

$(BIN):$(OBJ)
	@$(CC) -o $@ $^ $(LIB)

%.o:%.c
	@$(CC) -c $@ $<

.PHONY:cgi
cgi:
	for name in `echo $(CGI)`;\
		do\
			cd $$name;\
			make;\
			cd -;\
		done

.PHONY:clean
clean:
	@rm -rf *.o $(BIN) output
	for name in `echo $(CGI)`;\
		do\
			cd $$name;\
			make clean;\
			cd -;\
		done

.PHONY:output
output:
	@mkdir -p output/lib
	@mkdir -p output/htdoc/cgi
	@mkdir -p output/htdoc/img
	@cp -f httpd output
	@cp htdoc/cgi/cgi_math output/htdoc/cgi
	@cp htdoc/img/* output/htdoc/img
	@cp htdoc/index.html output/htdoc
	@cp htdoc/select.html output/htdoc
	@cp mysql_connect/cgi_insert output/htdoc/cgi
	@cp mysql_connect/cgi_select output/htdoc/cgi
	@cp start.sh output
	@cp lib/lib/*  output/lib

.PHONY:debug
debug:
	@echo $(ROOT_PATH)
	@echo $(BIN)
	@echo $(SRC)
	@echo $(OBJ)
