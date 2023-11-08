build:
	g++ -std=c++20 src/client.cpp -o src/client.out -I/usr/local/include/rpc -L/usr/local/lib -lrpc -lpthread -lgcov

test:
	./src/client.out

clean:
	$(RM) src/client.out