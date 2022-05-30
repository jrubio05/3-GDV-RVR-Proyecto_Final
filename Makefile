CC=g++
CFLAGS=-g -I. -std=c++11
DEPS = XLDisplay.h Serializable.h
OBJc = cliente.o XLDisplay.o Message.o
OBJs = servidor.o Message.o
LIBS=-lpthread -lm -lX11 -std=c++11

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: cliente servidor

cliente: $(OBJc) cliente.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

servidor: $(OBJs) servidor.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o cliente servidor

