main:
	g++ src/main.cpp -o main -fopenmp -lpthread

clean:
	rm -f main
