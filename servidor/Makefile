CC=g++
CFLAGS=-g -I. -std=c++11
DEPS = Serializable.h
OBJ = servidor.o Message.o
LIBS=-lpthread -lm -lX11 -std=c++11

%.o: %.cc $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

all: servidor

servidor: $(OBJ) servidor.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o servidor

