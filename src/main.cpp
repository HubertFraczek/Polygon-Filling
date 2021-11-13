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
  for (auto &i : map) {
    for (auto &j : i) {
      std::cout << j << " ";
    }
    std::cout << "\n";
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

void calculatEdges() {
	int y1, x1, y2, x2, tmp, y_acc, x_acc, old_x, counter;
  float accV2, x, y;

	for (int i = 0; i < points.size(); i++) {
		y1 = points[i].first;
		x1 = points[i].second;
		if (i == points.size() - 1) {
			y2 = points[0].first;
			x2 = points[0].second;
		} else {
			y2 = points[i + 1].first;
			x2 = points[i + 1].second;
		}

		map[y1][x1] = 3;
		map[y2][x2] = 3;
		if (y1 == y2) {
			for (int x = std::min(x1, x2) + 1;
				x < std::max(x1, x2); x++)
				map[y1][x] = 2;
		} else if (x1 == x2) {
			for (int y = std::min(y1, y2) + 1;
				y < std::max(y1, y2); y++)
				map[y][x1] = 2;
		} else {
      counter = 0;
      y_acc = y1 < y2 ? 1 : -1;
      x_acc = x1 < x2 ? 1 : -1;
      x = x1;
      accV2 = x_acc * ((float) abs(x1 - x2) / abs(y1 - y2));

      while (abs(y1 - y2) > 1) {
        y1 += y_acc;
        x += accV2;
        map[y1][(int)x] = 2;
      }
		}
	}
}

bool evenNumberOfEdgesToTheLeft(int y, int x, int xMax) {
  int counter = 0;
  for (int i = x + 1; i <= xMax; i++)
    if (map[y][i] == 2)
      counter++;
  return counter % 2 == 0 ? true : false;
}

bool evenNumberOfEdgesToTheRight(int y, int x, int xMin) {
  int counter = 0;
  for (int i = x - 1; i >= xMin; i--)
    if (map[y][i] == 2)
      counter++;
  return counter % 2 == 0 ? true : false;
}


bool evenNumberOfEdgesToTheLeft3(int y, int x, int xMax) {
  int counter = 0;
  for (int i = x + 1; i <= xMax; i++)
    if (map[y][i] == 2 || map[y][i] == 3)
      counter++;
  return counter % 2 == 0 ? true : false;
}

bool evenNumberOfEdgesToTheRight3(int y, int x, int xMin) {
  int counter = 0;
  for (int i = x - 1; i >= xMin; i--)
    if (map[y][i] == 2 || map[y][i] == 3)
      counter++;
  return counter % 2 == 0 ? true : false;
}

void scanLine(int yMin, int yMax, int xMin, int xMax) {
	int vertexCounter;
	for (int i = yMin + 1; i < yMax; i++) {
		vertexCounter = 0;
		for (int j = xMin; j < xMax; j++) {
      if (!evenNumberOfEdgesToTheLeft3(i, j, xMax) && !evenNumberOfEdgesToTheRight3(i, j, xMin) && map[i][j] != 2 && map[i][j] != 3)
        map[i][j] = 1;

      if (!evenNumberOfEdgesToTheLeft(i, j, xMax) && !evenNumberOfEdgesToTheRight(i, j, xMin) && map[i][j] != 2 && map[i][j] != 3)
        map[i][j] = 1;
    }
	}

	for (int i = yMin + 1; i < yMax; i++) {
		vertexCounter = 0;
		for (int j = xMin; j < xMax; j++) {
      if (map[i+1][j] == 1 && map[i-1][j] == 1 && map[i][j]==0)
        map[i][j]=1;
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
	calculatEdges();
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
