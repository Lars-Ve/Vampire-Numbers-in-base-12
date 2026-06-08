#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <cstdint>

struct VampireNumber {
    uint64_t decimal;       // decimal value of the vampire number (for sorting)
    std::string vamp_b12;   // vampire number in base 12
    std::string fang1_b12;
    std::string fang2_b12;
    uint64_t fang1_dec;
    uint64_t fang2_dec;
};

// Parses a line like:
//   [FOUND] 10026010 = 2001 * 6010  (dec: 35883660 = 3457 * 10380)
bool parse_line(const std::string& line, VampireNumber& out) {
    // Must start with [FOUND]
    if (line.rfind("[FOUND]", 0) != 0) return false;

    // Format: [FOUND] <vamp_b12> = <fang1_b12> * <fang2_b12>  (dec: <vamp_dec> = <fang1_dec> * <fang2_dec>)
    std::istringstream ss(line);
    std::string token;
    ss >> token; // "[FOUND]"

    ss >> out.vamp_b12;   // e.g. "10026010"
    ss >> token;           // "="
    ss >> out.fang1_b12;  // e.g. "2001"
    ss >> token;           // "*"
    ss >> out.fang2_b12;  // e.g. "6010"
    ss >> token;           // "(dec:"

    ss >> out.decimal;    // decimal value of vampire
    ss >> token;           // "="
    ss >> out.fang1_dec;
    ss >> token;           // "*"
    ss >> out.fang2_dec;

    return !ss.fail();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        std::cerr << "  Input:  raw output from Vampire_numbers_in_base_12\n";
        std::cerr << "  Output: sorted vampire numbers (ascending)\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin) {
        std::cerr << "Error: cannot open input file '" << argv[1] << "'\n";
        return 1;
    }

    std::vector<VampireNumber> vampires;
    std::string line;
    size_t skipped = 0;

    while (std::getline(fin, line)) {
        VampireNumber v;
        if (parse_line(line, v)) {
            vampires.push_back(v);
        } else if (!line.empty() &&
                   line.find("[FOUND]") == std::string::npos &&
                   line.find("---") == std::string::npos) {
            // Header/footer lines — silently skip
        }
    }
    fin.close();

    std::cout << "Read " << vampires.size() << " vampire numbers. Sorting...\n";

    std::sort(vampires.begin(), vampires.end(),
              [](const VampireNumber& a, const VampireNumber& b) {
                  return a.decimal < b.decimal;
              });

    std::ofstream fout(argv[2]);
    if (!fout) {
        std::cerr << "Error: cannot open output file '" << argv[2] << "'\n";
        return 1;
    }

    // Write header
    fout << "# Vampire numbers in base 12 (sorted ascending)\n";
    fout << "# Total: " << vampires.size() << "\n";
    fout << "# Format: <vampire_b12> = <fang1_b12> * <fang2_b12>  (dec: <vampire> = <f1> * <f2>)\n";
    fout << "#\n";

    for (size_t i = 0; i < vampires.size(); ++i) {
        const auto& v = vampires[i];
        fout << v.vamp_b12
             << " = " << v.fang1_b12
             << " * " << v.fang2_b12
             << "  (dec: " << v.decimal
             << " = " << v.fang1_dec
             << " * " << v.fang2_dec
             << ")\n";
    }

    fout.close();
    std::cout << "Done. Sorted output written to: " << argv[2] << "\n";
    return 0;
}