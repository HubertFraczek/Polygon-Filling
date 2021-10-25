#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define DEBUG true

std::vector<std::pair<int, int>> points;

void readPoints(std::string fileName) {
    std::ifstream infile(fileName);
    std::string line;
    double x, y;

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
        } else {
            points.push_back(std::pair<double, double>(x, y));
        }
    }
}

void printAllPoints() {
    for (auto &i : points)
        std::cout << "(" << i.first << ", " << i.second << ")\n";
}

int main(int argc, char** argv) {
    std::string fileName;
    if (argc < 2)
        std::cout << "Niewystarczająca liczba argumentów. Podaj scieżkę do pliku z punktami wielokąta.\n";
    else
        fileName = argv[1];

    readPoints(fileName);
    
    if (DEBUG) 
        printAllPoints(); 

    return 0;
}