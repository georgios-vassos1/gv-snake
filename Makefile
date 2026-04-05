CC     = g++
CFLAGS = -c -Wall -std=c++11
FLG    = -O2
NAME   = snake

all: snake

snake: Main.o Terrain.o mygetch.o ShitList.o List.o Point.o
	$(CC) $(FLG) -o $(NAME) Main.o Terrain.o mygetch.o ShitList.o List.o Point.o -pthread

Main.o: Main.cpp
	$(CC) $(FLG) Main.cpp $(CFLAGS)

Terrain.o: Terrain.cpp
	$(CC) $(FLG) Terrain.cpp $(CFLAGS)

mygetch.o: mygetch.cpp
	$(CC) $(FLG) mygetch.cpp $(CFLAGS)

ShitList.o: ShitList.cpp
	$(CC) $(FLG) ShitList.cpp $(CFLAGS)

List.o: List.cpp
	$(CC) $(FLG) List.cpp $(CFLAGS)

Point.o: Point.cpp
	$(CC) $(FLG) Point.cpp $(CFLAGS)

clean:
	rm -f *.o *.out *.exe *.bin $(NAME)
