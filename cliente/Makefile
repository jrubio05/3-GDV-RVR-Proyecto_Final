CC=g++
CFLAGS=-g -I. -std=c++11
DEPS = XLDisplay.h Serializable.h
OBJ = cliente.o XLDisplay.o Message.o
LIBS=-lpthread -lm -lX11 -std=c++11

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: cliente

cliente: $(OBJ) cliente.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o cliente

