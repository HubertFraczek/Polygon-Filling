#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
//#include <SDL.h>


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

void printMap(std::vector<std::vector<int>> map) {
	for (auto& i : map) {
		for (auto& j : i) {
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

void scaleImg(int scale) {
	for (auto& p : points) {
		p.first = p.first * scale;
		p.second = p.second * scale;
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
		}
		else {
			y2 = points[i + 1].first;
			x2 = points[i + 1].second;
		}

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
			counter = 0;
			y_acc = y1 < y2 ? 1 : -1;
			x_acc = x1 < x2 ? 1 : -1;
			x = x1;
			accV2 = x_acc * ((float)abs(x1 - x2) / abs(y1 - y2));

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

void clean(int yMin, int yMax, int xMin, int xMax) {
	for (int i = yMin + 1; i < yMax; i++) {
		for (int j = xMin; j < xMax; j++) {
			if (map[i + 1][j] == 1 && map[i - 1][j] == 1 && map[i][j] == 0)
				map[i][j] = 1;
		}
	}
}

void scanLine(int yMin, int yMax, int xMin, int xMax) {
	for (int i = yMin + 1; i < yMax; i++) {
		for (int j = xMin; j < xMax; j++) {
			if (!evenNumberOfEdgesToTheLeft3(i, j, xMax) && !evenNumberOfEdgesToTheRight3(i, j, xMin) && map[i][j] != 2 && map[i][j] != 3)
				map[i][j] = 1;

			if (!evenNumberOfEdgesToTheLeft(i, j, xMax) && !evenNumberOfEdgesToTheRight(i, j, xMin) && map[i][j] != 2 && map[i][j] != 3)
				map[i][j] = 1;
		}
	}

	clean(yMin, yMax, xMin, xMax);
}

///////////////////////////////////////////////////////////////////////////////
//                                   mpi                                     //
///////////////////////////////////////////////////////////////////////////////

bool evenNumberOfEdgesToTheLeft_MPI(std::vector<int> row, int x, int xMax) {
	int counter = 0;
	for (int i = x + 1; i <= xMax; i++)
		if (row[i] == 2)
			counter++;
	return counter % 2 == 0 ? true : false;
}

bool evenNumberOfEdgesToTheRight_MPI(std::vector<int> row, int x, int xMin) {
	int counter = 0;
	for (int i = x - 1; i >= xMin; i--)
		if (row[i] == 2)
			counter++;
	return counter % 2 == 0 ? true : false;
}

bool evenNumberOfEdgesToTheLeft3_MPI(std::vector<int> row, int x, int xMax) {
	int counter = 0;
	for (int i = x + 1; i <= xMax; i++)
		if (row[i] == 2 || row[i] == 3)
			counter++;
	return counter % 2 == 0 ? true : false;
}

bool evenNumberOfEdgesToTheRight3_MPI(std::vector<int> row, int x, int xMin) {
	int counter = 0;
	for (int i = x - 1; i >= xMin; i--)
		if (row[i] == 2 || row[i] == 3)
			counter++;
	return counter % 2 == 0 ? true : false;
}

std::vector<int> scanLineRow_MPI(std::vector<int> row) {
	int xMax = row.size();
	for (int x = 0; x < xMax; x++) {
		if (!evenNumberOfEdgesToTheLeft3_MPI(row, x, xMax) && !evenNumberOfEdgesToTheRight3_MPI(row, x, 0) && row[x] != 2 && row[x] != 3)
			row[x] = 1;

		if (!evenNumberOfEdgesToTheLeft_MPI(row, x, xMax) && !evenNumberOfEdgesToTheRight_MPI(row, x, 0) && row[x] != 2 && row[x] != 3)
			row[x] = 1;
	}

	return row;
}

std::vector<int> transformTo1d(std::vector<std::vector<int>> map) {
	std::vector<int> result;
	for (auto& row : map)
		for (auto& v : row)
			result.push_back(v);
	return result;
}

std::vector<std::vector<int>> scanLine_MPI() {
	int rowLength = map[0].size();
	std::vector<int> map1d = transformTo1d(map);
	std::vector<std::vector<int>> result;

	for (int i = 0; i <= map1d.size() - rowLength; i += rowLength) {
		std::vector<int> tmp(&map1d[i], &map1d[i + rowLength]);
		std::vector<int> res = scanLineRow_MPI(tmp);
		result.push_back(res);
	}
	return result;
}

void printVector(std::vector<int> v, int rank) {
	v.push_back(0 - rank);
	for (int i = 0; i < v.size(); i++) {
		std::cout << v.at(i) << " ";
	}
}

//void drawUsingSDL(std::vector<std::vector<int>> map) {
//	SDL_Event event;
//	SDL_Renderer* renderer;
//	SDL_Window* window;
//	int i;
//
//	SDL_Init(SDL_INIT_VIDEO);
//	int const WINDOW_WIDTH = 800;
//	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
//	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
//	SDL_RenderClear(renderer);
//	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
//	int c = 0;
//	for (auto& i : map) {
//		c++;
//		int z = 0;
//		for (auto& j : i) {
//			z++;
//			if (j != 0) {
//				SDL_RenderDrawPoint(renderer, z, c);
//			}
//		}
//	}
//
//	SDL_RenderPresent(renderer);
//	while (1) {
//		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
//			break;
//	}
//	SDL_DestroyRenderer(renderer);
//	SDL_DestroyWindow(window);
//	SDL_Quit();
//}

int main(int argc, char** argv) {
	int currentRank, numberOfRanks;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &currentRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfRanks);
	double start = MPI_Wtime();

	std::vector<int> partialResult;
	std::string fileName = "C:\\Users\\Michal\\Downloads\\poly.txt";

	std::pair<std::pair<int, int>, std::pair<int, int>> dimensions = readPoints(fileName);
	initializeMap(dimensions.first.first, dimensions.first.second, dimensions.second.first, dimensions.second.second);
	int rowLength = map[0].size();
	int portionSize = map.size() / numberOfRanks;

	if (map.size() < numberOfRanks) {
		printf("Uzyj %d procesow. Lub wprowadz wielokat o wiekszej wysokosci. Uzycie %d procesow jest nieefektywne.", map.size(), numberOfRanks);
		MPI_Finalize();
		exit(1);
	}
	calculatEdges();
	int startAt = currentRank * portionSize;
	int endAt = startAt + portionSize;
	for (int i = startAt; i < endAt; i++) {
		std::vector<int> tmp = scanLineRow_MPI(map.at(i));
		partialResult.insert(partialResult.end(), tmp.begin(), tmp.end());
	}

	int count = partialResult.size() * numberOfRanks;
	std::vector<int> buff;
	buff.resize(count);
	MPI_Barrier(MPI_COMM_WORLD);
	if (currentRank == 0) {
		MPI_Gather(partialResult.data(), partialResult.size(), MPI_INT, buff.data(), partialResult.size(), MPI_INT, 0, MPI_COMM_WORLD);
		std::vector<std::vector<int>> gatheredMap;
		for (int i = 0; i <= buff.size() - rowLength; i += rowLength) {
			std::vector<int> tmp(&buff[i], &buff[i + rowLength]);
			gatheredMap.push_back(tmp);
		}


		if (gatheredMap.size() < map.size()) {
			for (int i = gatheredMap.size(); i < map.size(); i++) {
				std::vector<int> tmp = scanLineRow_MPI(map.at(i));
				gatheredMap.push_back(tmp);
			}
		}

		printMap(gatheredMap);
	}
	else
	{
		MPI_Gather(partialResult.data(), partialResult.size(), MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
	}
	double duration =MPI_Wtime() - start;
	MPI_Finalize();
	return 0;
}

