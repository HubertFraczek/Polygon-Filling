#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <omp.h>
#include <SDL.h>

#define NOMINMAX
#define DEBUG false
#include <Windows.h>

std::vector<std::pair<int, int>> points;
std::vector<std::vector<int>> map;

std::pair<std::pair<int, int>, std::pair<int, int>> readPoints(std::string fileName) {
	std::ifstream infile(fileName);
	std::string line;
	int x, y, yMin, yMax, xMin, xMax, counter = 0;

	if (!infile) {
		system("echo %cd%");
		std::cerr << "Nie można otworzyć pliku: " + fileName + "\n";
		exit(1);
	}

	while (std::getline(infile, line)) {
		std::istringstream iss(line);

		if (!(iss >> x >> y)) {
			std::cout << "Błąd. Nieprawidłowy format pliku.\n";
			break;
		}
		else {
			if (counter == 0) {
				yMin = y;
				yMax = y;
				xMin = x;
				xMax = x;
			}
			else {
				if (y > yMax) yMax = y;
				if (y < yMin) yMin = y;
				if (x > xMax) xMax = x;
				if (x < xMin) xMin = x;
			}
			points.push_back(std::pair<double, double>(y, x));
		}
		counter++;
	}
	return std::pair<std::pair<int, int>, std::pair<int, int>>(
		std::pair<int, int>(yMin, yMax), std::pair<int, int>(xMin, xMax));
}

void printAllPoints() {
	for (auto& i : points)
		std::cout << "(" << i.second << ", " << i.first << ")\n";
}

void printMap() {
	int c = 0;
	for (auto& i : map) {
		c++;
		int z = 0;
		for (auto& j : i) {
			z++;
			std::cout << j << " ";
			if (j != 0) {
				std::cout << j << " ";
			}
		}
	}
}

void initializeMap(int yMin, int yMax, int xMin, int xMax) {
	for (int i = 0; i <= yMax + yMin; i++) {
		std::vector<int> tmp;
		for (int j = 0; j <= xMax + xMin; j++) {
			tmp.push_back(0);
		}
		map.push_back(tmp);
	}
}

void calculatedges() {
	int y1, x1, y2, x2, translation, acc, tmp;
	for (int i = 0; i < points.size(); i++) {
		y1 = points[i].first;
		x1 = points[i].second;
		if (i == points.size() - 1) {
			y2 = points[0].first;
			x2 = points[0].second;
		}
		else {
			y2 = points[i + 1].first;
			x2 = points[i + 1].second;
		}
		translation = 0;
		map[y1][x1] = 3;
		map[y2][x2] = 3;
		if (y1 == y2) {
			for (int x = std::min(x1, x2) + 1;
				x < std::max(x1, x2); x++)
				map[y1][x] = 2;
		}
		else if (x1 == x2) {
			for (int y = std::min(y1, y2) + 1;
				y < std::max(y1, y2); y++)
				map[y][x1] = 2;
		}
		else {
			if (y1 > y2) {
				map[y1][x1 + 1] = 2;
				tmp = y1;
				y1 = y2;
				y2 = tmp;
				tmp = x1;
				x1 = x2;
				x2 = tmp;
			}
			for (int j = std::min(y1, y2) + 1; j < std::max(y1, y2); j++) {
				acc = std::ceil((float)abs(x1 - x2) / abs(y1 - y2));
				if (x1 > x2)
					acc *= -1;
				map[j][x1 + acc] = 2;
				if (x1 > x2)
					x1--;
				else
					x1++;
			}
		}
	}
}

void scanLine(int yMin, int yMax, int xMin, int xMax) {
	int vertexCounter;
	for (int i = yMin + 1; i < yMax; i++) {
		vertexCounter = 0;
		for (int j = xMin; j < xMax; j++) {
			if (map[i][j] == 2 || map[i][j] == 3) {
				vertexCounter++;
				continue;
			}

			if (vertexCounter % 2 == 1)
				map[i][j] = 1;
		}
	}
}

void drawUsingSDL() {
	SDL_Event event;
	SDL_Renderer* renderer;
	SDL_Window* window;
	int i;

	SDL_Init(SDL_INIT_VIDEO);
	int const WINDOW_WIDTH = 1000;
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	int c = 0;
	for (auto& i : map) {
		c++;
		int z = 0;
		for (auto& j : i) {
			z++;
			if (j != 0) {
				SDL_RenderDrawPoint(renderer, z, c);
			}
		}
	}

	SDL_RenderPresent(renderer);
	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
			break;
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char** argv) {

	std::string fileName;
	if (argc < 2)
		std::cout << "Niewystarczająca liczba argumentów. Podaj scieżkę do pliku z punktami wielokąta.\n";
	else
		fileName = argv[1];

	std::pair<std::pair<int, int>, std::pair<int, int>> dimensions = readPoints(fileName);
	initializeMap(dimensions.first.first, dimensions.first.second, dimensions.second.first, dimensions.second.second);
	calculatedges();
	scanLine(dimensions.first.first, dimensions.first.second, dimensions.second.first, dimensions.second.second);

	if (DEBUG) {
		printAllPoints();
		std::cout << "Polygon height = (" << dimensions.first.first << ", " << dimensions.first.second << ")\n";
		std::cout << "Polygon width = (" << dimensions.second.first << ", " << dimensions.second.second << ")\n";
		std::cout << "Map:\n";
		printMap();
	}

	drawUsingSDL();
	return 0;
}