main:
	g++ src/main.cpp -o main -fopenmp -lpthread -lSDL2main -lSDL2

clean:
	rm -f main
