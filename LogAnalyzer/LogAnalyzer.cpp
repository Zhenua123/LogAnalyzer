#include <iostream>
#include <fstream>
#include <string>
#include <map>

int main(int argc, char* argv[]) {
    std::string filename;

    // Если файл не передан как аргумент - запрашиваем у пользователя
    if (argc != 2) {
        std::cout << "Enter log file path: ";
        std::cin >> filename;
    }
    else {
        filename = argv[1];
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.ignore();
        std::cin.get();
        return 1;
    }

    // Подсчёт частоты ошибок
    std::map<std::string, int> errors;
    std::string line;

    while (std::getline(file, line)) {
        if (line.find("ERROR") != std::string::npos) {
            errors["ERROR"]++;
        }
        else if (line.find("WARNING") != std::string::npos) {
            errors["WARNING"]++;
        }
    }
    file.close();

    // Проверка на пустую статистику
    if (errors.empty()) {
        std::cout << "\nNo errors or warnings found in the log file." << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.ignore();
        std::cin.get();
        return 0;
    }

    // Гистограмма в консоли
    std::cout << "\n=== LOG ANALYZER ===\n" << std::endl;

    int maxCount = 0;
    for (auto& e : errors) {
        if (e.second > maxCount) maxCount = e.second;
    }

    for (auto& e : errors) {
        int barLen = e.second * 40 / maxCount;
        std::cout << e.first << " |";
        for (int i = 0; i < barLen; i++) std::cout << "#";
        std::cout << " " << e.second << std::endl;
    }

    // Экспорт в CSV
    std::ofstream csv("stats.csv");
    if (csv.is_open()) {
        csv << "Type,Count\n";
        for (auto& e : errors) {
            csv << e.first << "," << e.second << "\n";
        }
        csv.close();
        std::cout << "\nCSV exported: stats.csv" << std::endl;
    }

    // Экспорт в JSON
    std::ofstream json("stats.json");
    if (json.is_open()) {
        json << "{\n";
        json << "  \"statistics\": {\n";

        int count = 0;
        int total = errors.size();
        for (auto& e : errors) {
            json << "    \"" << e.first << "\": " << e.second;
            if (count < total - 1) json << ",";
            json << "\n";
            count++;
        }

        json << "  }\n";
        json << "}\n";
        json.close();
        std::cout << "JSON exported: stats.json" << std::endl;
    }

    std::cout << "\nAnalysis complete!" << std::endl;
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}