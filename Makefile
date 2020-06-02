LIBS = -pthread #-lsocket
httpserver: http.c main.c mysocket.c
	gcc -g -W -Wall $^ $(LIBS) -o $@ 

clean:
	rm httpserver
