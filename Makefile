CXX = g++


DB_LIBS = -lmysqlcppconnx


CLIENT = src/client
HTTP_SERVER = src/http_server
DB_SERVER = src/db_server
LOAD_GEN = src/load_gen

CLIENT_SRC = src/client.cpp
HTTP_SERVER_SRC = src/http_server.cpp
DB_SERVER_SRC = src/db_server.cpp
LOAD_GEN_SRC = src/load_generator.cpp



#target
all: $(CLIENT) $(HTTP_SERVER) $(DB_SERVER) $(LOAD_GEN)

$(CLIENT): $(CLIENT_SRC)
	$(CXX) -o $(CLIENT) $(CLIENT_SRC)

$(LOAD_GEN): $(LOAD_GEN_SRC)
	$(CXX) -o $(LOAD_GEN) $(LOAD_GEN_SRC)

$(HTTP_SERVER): $(HTTP_SERVER_SRC)
	$(CXX) -o $(HTTP_SERVER) $(HTTP_SERVER_SRC)

$(DB_SERVER): $(DB_SERVER_SRC)
	$(CXX) -o $(DB_SERVER) $(DB_SERVER_SRC) $(DB_LIBS)


clean:
	rm -f $(CLIENT) $(HTTP_SERVER) $(DB_SERVER) $(LOAD_GEN)

.PHONY: all clean