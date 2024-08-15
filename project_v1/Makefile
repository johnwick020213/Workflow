INCLUDES:=./  ./include
SRCS:=$(wildcard *.cc) 
LIBS:=  -lsrpc -lprotobuf -llz4 -lsnappy -lwfrest -lworkflow -lssl -lcrypto -lcrypt
OBJS:=$(patsubst %.cc, %.o, $(SRCS))
SERVER:= CloudiskServer

# 目标 : 依赖
$(SERVER): main.o CloudiskServer.o Token.o Hash.o signup.pb.o
	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

%.o:%.cc
	g++ -c $^ -o $@  $(addprefix -I, $(INCLUDES)) -g

HASH:=testHash
TOKEN:=testToken

$(HASH): Hash.o testHash.o
	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

$(TOKEN): Token.o testToken.o
	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

echo:
	echo $(INCLUDES)
	echo $(SRCS)

clean:
	rm -rf $(OBJS) $(SERVER) $(HASH) $(TOKEN)
