#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "../include/Environment.h"


struct FileEntry {
    std::string name;
    int size;
};

int parser() {
    std::string filePath;
    std::cin >> filePath;
    std::ifstream file(filePath);

    if (!file) {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return 1;
    }

    std::string line;
    std::vector<FileEntry> files;
    std::vector<std::vector<std::string>> folders;
    std::vector<int> fileSizes;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string fullPath;
        int size;

        if (!(iss >> fullPath >> size)) {
            std::cerr << "Error: Invalid input format on line: " << line << std::endl;
            continue;
        }

        std::vector<std::string> folderHierarchy;
        size_t lastSlash = fullPath.find_last_of('/');
        if (lastSlash == std::string::npos) {
            std::cerr << "Error: Invalid path format on line: " << line << std::endl;
            continue;
        }

        for(auto i : folderHierarchy) cout << i << " "<< size << "\n";

        std::string path = fullPath.substr(0, lastSlash);
        std::string fileName = fullPath.substr(lastSlash + 1);
        std::stringstream pathStream(path);
        std::string folder;
        while (std::getline(pathStream, folder, '/')) {
            folderHierarchy.push_back(folder);
        }

        FileEntry entry = { fileName, size };
        files.push_back(entry);
        fileSizes.push_back(size);
        folders.push_back(folderHierarchy);
    }

    file.close();

    std::cout << "Parsed file entries:\n";
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "File: " << files[i].name << ", Size: " << files[i].size << " bytes\n";
        std::cout << "Folders: ";
        for (const auto& folder : folders[i]) {
            std::cout << folder << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
