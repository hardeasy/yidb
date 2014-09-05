yidb : yidb.o index.o store.o net.o
	cc -o yidb yidb.o index.o store.o net.o
yidb.o : yidb.c yidb.h
	cc -c yidb.c
index.o : index.c index.h
	cc -c index.c
store.o : store.c store.h
	cc -c store.c
net.o : net.c yidb.h
	cc -c net.c
clean :
	rm yidb yidb.o index.o store.o net.o