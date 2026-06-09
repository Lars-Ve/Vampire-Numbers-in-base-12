#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <stdexcept>

// Parse a line like:
// 1270 = 21 * 70  (dec: 2100 = 25 * 84)
// Returns the decimal vampire number, or -1 if the line should be skipped.
long long parseLine(const std::string& line) {
    // Look for "(dec:" pattern
    size_t decPos = line.find("(dec:");
    if (decPos == std::string::npos) return -1;

    // Extract the part after "(dec:"
    std::string decPart = line.substr(decPos + 5);

    // The vampire decimal value is the first number after "(dec:"
    std::istringstream iss(decPart);
    long long val;
    if (!(iss >> val)) return -1;
    return val;
}

// Read existing b-file, return set of already-present indices and max index used
struct BFileData {
    std::vector<std::pair<long long, long long>> entries; // (index, value)
    std::set<long long> indices;
    std::set<long long> values;
};

BFileData readBFile(const std::string& filename) {
    BFileData data;
    std::ifstream f(filename);
    if (!f.is_open()) return data; // File doesn't exist yet

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        long long idx, val;
        if (iss >> idx >> val) {
            data.entries.push_back({idx, val});
            data.indices.insert(idx);
            data.values.insert(val);
        }
    }
    return data;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <b-file> <input-file>\n";
        std::cerr << "  <b-file>    : path to the OEIS b-file (created if it doesn't exist)\n";
        std::cerr << "  <input-file>: path to input file with vampire numbers, or '-' for stdin\n";
        return 1;
    }

    std::string bfilePath = argv[1];
    std::string inputPath = argv[2];

    // Read existing b-file
    BFileData existing = readBFile(bfilePath);

    // Determine the next index
    long long nextIndex = 1;
    if (!existing.entries.empty()) {
        nextIndex = existing.entries.back().first + 1;
    }

    // Read input
    std::istream* inStream = nullptr;
    std::ifstream inputFile;
    if (inputPath == "-") {
        inStream = &std::cin;
    } else {
        inputFile.open(inputPath);
        if (!inputFile.is_open()) {
            std::cerr << "Error: cannot open input file '" << inputPath << "'\n";
            return 1;
        }
        inStream = &inputFile;
    }

    // Parse new values from input
    std::vector<long long> newValues;
    std::string line;
    while (std::getline(*inStream, line)) {
        if (line.empty() || line[0] == '#') continue;
        long long val = parseLine(line);
        if (val < 0) continue;
        // Skip duplicates already in b-file
        if (existing.values.count(val)) {
            std::cerr << "Skipping duplicate value: " << val << "\n";
            continue;
        }
        // Skip duplicates within this batch
        bool alreadyInBatch = false;
        for (long long v : newValues) if (v == val) { alreadyInBatch = true; break; }
        if (alreadyInBatch) {
            std::cerr << "Skipping duplicate in batch: " << val << "\n";
            continue;
        }
        newValues.push_back(val);
    }

    if (newValues.empty()) {
        std::cout << "No new entries to add.\n";
        return 0;
    }

    // Sort new values ascending
    std::sort(newValues.begin(), newValues.end());

    // Append to b-file
    std::ofstream out(bfilePath, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Error: cannot open b-file '" << bfilePath << "' for writing\n";
        return 1;
    }

    // If the file is new/empty, write a minimal header comment
    if (existing.entries.empty()) {
        out << "# OEIS b-file\n";
        out << "# Format: <index> <value>\n";
    }

    for (long long val : newValues) {
        out << nextIndex << " " << val << "\n";
        std::cout << "Added: " << nextIndex << " " << val << "\n";
        nextIndex++;
    }

    out.close();
    std::cout << "Done. Added " << newValues.size() << " new entries to '" << bfilePath << "'.\n";
    return 0;
}