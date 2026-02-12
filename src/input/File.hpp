#pragma once

#include <string>
#include <vector>

class File {

    private:
        std::string path;
        std::vector<std::string> lines;

    public:
        File(const std::string& path);
        bool load();
        const std::vector<std::string>& getLines() const;
        bool File::save(const std::vector<std::string>& lines);
        const std::string& getPath() const;
    };