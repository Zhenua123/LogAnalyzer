#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <vector>

////Нужно для задержки////
#include <chrono>
#include <thread>
//////////////////////////


using error_type = std::map<std::string, int>;

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

std::string parseArgs(int argc, char* argv[])
{
    //Добавить обработку одного флага --cycled 60 (-c 60)
    //Этот флаг будет включать циклическую проверку файла лога через каждые несколько секунд или минуту

    std::string filename;
    // Если файл не передан как аргумент - запрашиваем у пользователя
    if (argc != 2) {
        std::cout << "Enter log file path: ";
        std::cin >> filename;
    }
    else {
        filename = argv[1];
    }
    return filename;
}

error_type findErrors(std::ifstream& file)
{
    // Подсчёт частоты ошибок
    error_type errors;
    std::string line;
    
    //Добавить заполнение не "ручками", а автоматическое из файла config.cfg.
    //Если это не удаётся, то устанавливают какие-нибудь стандартные значения

    std::vector<std::string> possible_issues = { "error", "warning", "log", "critical", "info" };

    while (std::getline(file, line)) {

        std::transform(line.begin(), line.end(), line.begin(),
            [](char ch)
            {
                return std::tolower(ch);
            });

        for (size_t i = 0; i < possible_issues.size(); i++)
        {
            std::string current_error = possible_issues[i];

            if (line.find(current_error) != std::string::npos) {
                errors[current_error]++;
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

    //Вероятно, где-нибудь здесь будет задержка (нужен thread и chrono)
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