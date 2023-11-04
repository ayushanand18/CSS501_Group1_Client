build:
	g++ src/client.cpp -o src/client.out -I/usr/local/include/rpc -L/usr/local/lib -lrpc -lpthread

test:
	./src/client.out

clean:
	$(RM) src/client.out