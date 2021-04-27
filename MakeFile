CC=gcc
CFLAGS=-O3 -DINFO

LOG=build/log.o
LOG_SH=/usr/lib/liblog_shared.so
ID=build/ID.o
ID_TCP=build/ID_TCP.o
PACK=build/packet.o
ERROR=build/error.o


LIBRARY_O=$(LOG) $(ID) $(PACK) $(ERROR) $(ID_TCP)
DYNAMIC=-llog_shared


all: server_compile server_slave_compile client_compile server_compileTCP server_slave_compileTCP client_compileTCP
	@echo "Files were compiled!"

server_compile: LIB
	$(CC) $(CFLAGS) udp/server.c $(LIBRARY_O) -o build/server.o $(DYNAMIC)
server_slave_compile: LIB
	$(CC) $(CFLAGS) udp/server_slave.c $(LIBRARY_O) -o build/server_slave.o $(DYNAMIC)
client_compile: LIB
	$(CC) $(CFLAGS) udp/client.c $(LIBRARY_O) -o build/client.o $(DYNAMIC)
server_compileTCP: LIB
	$(CC) $(CFLAGS) tcp/server_tcp.c $(LIBRARY_O) -o build/server_tcp.o $(DYNAMIC)
server_slave_compileTCP: LIB
	$(CC) $(CFLAGS) tcp/server_slave_tcp.c $(LIBRARY_O) -o build/server_slave_tcp.o $(DYNAMIC)
client_compileTCP: LIB
	$(CC) $(CFLAGS) tcp/client_tcp.c $(LIBRARY_O) -o build/client_tcp.o $(DYNAMIC)

LIB: LIB_LOG LIB_ERR LIB_ID LIB_ID_TCP LIB_PACK

LIB_LOG:
	@$(CC) $(CFLAGS) -c -fPIC log/log.c -o $(LOG)
	@$(CC) $(CFLAGS) -shared $(LOG) -o build/liblog_shared.so
	@sudo cp build/liblog_shared.so /usr/lib
	@sudo chmod 0755 $(LOG_SH)
LIB_ERR:
	$(CC) $(CFLAGS) -c error/Error.c -o $(ERROR)
LIB_ID:
	$(CC) $(CFLAGS) -c ID/ID.c -o $(ID)
LIB_ID_TCP:
	$(CC) $(CFLAGS) -c ID/ID_TCP.c -o $(ID_TCP)
LIB_PACK:
	$(CC) $(CFLAGS) -c packet/packet.c -o $(PACK)


clean:
	rm build/*.o
	sudo rm $(LOG_SH)