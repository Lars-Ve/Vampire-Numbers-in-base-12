#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

// CONFIGURATION: K is the number of digits for each fang
// K = 1 searches for 2-digit vampire numbers: no such vampire numbers exist
// K = 2 searches for 4-digit vampire numbers (milliseconds)
// K = 3 searches for 6-digit vampire numbers (milliseconds)
// K = 4 searches for 8-digit vampire numbers (seconds)
// K = 5 searches for 10-digit vampire numbers (minutes)
// K = 6 searches for 12-digit vampire numbers (hours)
const int K = 6; 

std::mutex mtx;
int vampire_count = 0;

// Helper to convert a number to Base 12 string representation
std::string to_base12(uint64_t n) {
    if (n == 0) return "0";
    std::string s = "";
    const char digits[] = "0123456789AB";
    while (n > 0) {
        s += digits[n % 12];
        n /= 12;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

// Generates a bit-profile of digit frequencies in Base 12
// Each of the 12 digits gets 4 bits in a 64-bit integer
inline uint64_t get_profile(uint64_t n) {
    uint64_t profile = 0;
    while (n > 0) {
        uint64_t digit = n % 12;
        profile += (1ULL << (digit << 2)); // digit * 4
        n /= 12;
    }
    return profile;
}

// Compile-time calculation of powers of 12
constexpr uint64_t power_of_12(int exponent) {
    uint64_t result = 1;
    for (int i = 0; i < exponent; ++i) result *= 12;
    return result;
}

void search_vampires(uint64_t a_start, uint64_t a_end, 
                     uint64_t min_fang, uint64_t max_fang, 
                     uint64_t min_vamp, uint64_t max_vamp, 
                     const std::vector<uint64_t>& fang_profiles) {
                     
    for (uint64_t a = a_start; a < a_end; ++a) {
        uint64_t prof_a = fang_profiles[a - min_fang];
        bool a_has_trailing_zero = (a % 12 == 0);

        // Calculate exact boundaries for B to keep A * B within the 2*K digit range
        uint64_t b_start = (min_vamp + a - 1) / a;
        if (b_start < a) b_start = a; // Avoid duplicate pairs (a <= b)

        uint64_t b_end = max_vamp / a;
        if (b_end > max_fang) b_end = max_fang;

        for (uint64_t b = b_start; b <= b_end; ++b) {
            // Skip if both fangs have a trailing zero (Base 12 rule)
            if (a_has_trailing_zero && (b % 12 == 0)) continue;

            uint64_t v = a * b;
            uint64_t prof_v = get_profile(v);

            // Bitwise check: profile A + profile B == profile V
            if (prof_a + fang_profiles[b - min_fang] == prof_v) {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "[FOUND] " << to_base12(v) << " = " 
                          << to_base12(a) << " * " << to_base12(b) 
                          << "  (Decimal: " << v << " = " << a << " * " << b << ")\n";
                vampire_count++;
            }
        }
    }
}

int main() {
    const uint64_t MIN_FANG = power_of_12(K - 1);
    const uint64_t MAX_FANG = power_of_12(K) - 1;
    const uint64_t MIN_VAMP = power_of_12(2 * K - 1);
    const uint64_t MAX_VAMP = power_of_12(2 * K) - 1;

    std::cout << "Pre-calculating fang profiles...\n";
    size_t range_size = MAX_FANG - MIN_FANG + 1;
    std::vector<uint64_t> fang_profiles(range_size);
    
    for (uint64_t i = MIN_FANG; i <= MAX_FANG; ++i) {
        fang_profiles[i - MIN_FANG] = get_profile(i);
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    std::cout << "Starting search using " << num_threads << " threads...\n";
    std::cout << "Searching for " << (2 * K) << "-digit vampire numbers in Base 12...\n";
    std::cout << "--------------------------------------------------\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    uint64_t total_work = MAX_FANG - MIN_FANG + 1;
    uint64_t work_per_thread = total_work / num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        uint64_t a_start = MIN_FANG + i * work_per_thread;
        uint64_t a_end = (i == num_threads - 1) ? (MAX_FANG + 1) : (a_start + work_per_thread);
        
        threads.push_back(std::thread(search_vampires, a_start, a_end, 
                                      MIN_FANG, MAX_FANG, MIN_VAMP, MAX_VAMP, 
                                      std::ref(fang_profiles)));
    }

    for (auto& th : threads) {
        th.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "--------------------------------------------------\n";
    std::cout << "Total found: " << vampire_count << "\n";
    std::cout << "Completed in: " << elapsed.count() << " seconds.\n";
    return 0;
}