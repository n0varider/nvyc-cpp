#include "File.hpp"
#include <fstream>
#include <sstream>

File::File(const std::string& filepath)
    : path(filepath) {}

bool File::load() {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    lines.clear();
    std::string line;

    // read file line by line
    while (std::getline(in, line)) {
        lines.push_back(line);
    }

    return true;
}

bool File::save(const std::vector<std::string>& lines) {
    std::ofstream out(path);
    if(!out.is_open()) return false;

    for(const auto& line: lines) {
        out << line << "\n";
    }
    return true;
}

const std::vector<std::string>& File::getLines() const {
    return lines;
}

const std::string& File::getPath() const {
    return path;
}
