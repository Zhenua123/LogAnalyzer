#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <cctype>

////Нужно для задержки////
#include <chrono>
#include <thread>
//////////////////////////


using error_type = std::map<std::string, int>;

// Функция загрузки конфигурации из файла config.cfg
std::vector<std::string> loadConfig(const std::string& configFile = "config.cfg") {
    std::vector<std::string> keywords;

    // Стандартные значения (если файл не откроется)
    std::vector<std::string> defaultKeywords = { "error", "warning", "critical", "info", "debug" };

    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cout << "Config file not found. Using default keywords." << std::endl;
        return defaultKeywords;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') continue;

        // Удаляем пробелы в начале и конце
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (!line.empty()) {
            keywords.push_back(line);
        }
    }
    file.close();

    if (keywords.empty()) {
        std::cout << "Config file is empty. Using default keywords." << std::endl;
        return defaultKeywords;
    }

    std::cout << "Loaded keywords: ";
    for (const auto& kw : keywords) {
        std::cout << kw << " ";
    }
    std::cout << std::endl;

    return keywords;
}

void exportJSON(const error_type& errors)
{
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
}

void exportCSV(const error_type& errors)
{
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
}

void histogram(const error_type& errors)
{
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
}

// Функция для циклической проверки файла
void cyclicCheck(const std::string& filename, int intervalSeconds) {
    std::cout << "\n=== CYCLIC MODE ENABLED ===" << std::endl;
    std::cout << "Checking file every " << intervalSeconds << " seconds" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    while (true) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
        }
        else {
            error_type errors = findErrors(file);
            file.close();

            if (!errors.empty()) {
                std::cout << "\n--- New analysis at "
                    << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
                    << " ---" << std::endl;
                exportJSON(errors);
                exportCSV(errors);
                histogram(errors);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }
}

std::string parseArgs(int argc, char* argv[])
{
    std::string filename;
    bool cyclicMode = false;
    int interval = 60; // по умолчанию 60 секунд

    // Обработка аргументов командной строки
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--cycled" || arg == "-c") {
            cyclicMode = true;
            if (i + 1 < argc) {
                interval = std::stoi(argv[i + 1]);
                i++;
            }
        }
        else if (filename.empty()) {
            filename = arg;
        }
    }

    // Если файл не передан как аргумент - запрашиваем у пользователя
    if (filename.empty()) {
        std::cout << "Enter log file path: ";
        std::cin >> filename;
    }

    // Если включен циклический режим - запускаем его
    if (cyclicMode) {
        cyclicCheck(filename, interval);
    }

    return filename;
}

error_type findErrors(std::ifstream& file)
{
    // Подсчёт частоты ошибок
    error_type errors;
    std::string line;

    // Автоматическая загрузка из файла config.cfg
    std::vector<std::string> possible_issues = loadConfig();

    while (std::getline(file, line)) {

        std::transform(line.begin(), line.end(), line.begin(),
            [](char ch)
            {
                return std::tolower(ch);
            });

        for (const auto& keyword : possible_issues) {
            if (line.find(keyword) != std::string::npos) {
                // Преобразуем в верхний регистр для вывода
                std::string upperKeyword = keyword;
                std::transform(upperKeyword.begin(), upperKeyword.end(),
                    upperKeyword.begin(), ::toupper);
                errors[upperKeyword]++;
            }
        }
    }

    return errors;
}

int main(int argc, char* argv[]) {
    std::string filename = parseArgs(argc, argv);

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.ignore();
        std::cin.get();
        return 1;
    }

    error_type errors = findErrors(file);

    file.close();

    // Задержка на 30 секунд (для демонстрации)
    std::cout << "\nWaiting 30 seconds before analysis..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Проверка на пустую статистику
    if (errors.empty()) {
        std::cout << "\nNo errors or warnings found in the log file." << std::endl;
        std::cout << "\nPress Enter to exit...";
        std::cin.ignore();
        std::cin.get();
        return 0;
    }

    exportJSON(errors);
    exportCSV(errors);
    histogram(errors);

    std::cout << "\nAnalysis complete!" << std::endl;
    std::cout << "\nPress Enter to exit...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}