#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>

// CONFIGURATION: K is the number of digits for each fang
// K = 4 searches for 8-digit vampire numbers  (milliseconds)
// K = 5 searches for 10-digit vampire numbers (minutes)
// K = 6 searches for 12-digit vampire numbers (hours)
const int K = 6;

// Compile-time sanity check: K=6 with uint64_t is safe (12^12 < 2^64),
// but K=7 would overflow — block it.
static_assert(K >= 2 && K <= 6, "K must be between 2 and 6");

std::mutex mtx;
std::atomic<uint64_t> found_count{0};

// ── Helpers ──────────────────────────────────────────────────────────────────

std::string to_base12(uint64_t n) {
    if (n == 0) return "0";
    static constexpr char digits[] = "0123456789AB";
    std::string s;
    s.reserve(16);
    while (n > 0) {
        s += digits[n % 12];
        n /= 12;
    }
    std::reverse(s.begin(), s.end());
    return s;
}

// Digit-frequency profile: 4 bits per Base-12 digit packed into a uint64_t.
// Supports up to 15 occurrences of any single digit without overflow —
// safe for numbers up to 15 digits in Base 12 (covers K ≤ 6, i.e. 12 digits).
inline uint64_t get_profile(uint64_t n) {
    uint64_t profile = 0;
    while (n > 0) {
        profile += (1ULL << ((n % 12) << 2));
        n /= 12;
    }
    return profile;
}

// Exact digit-sort verification — used after a profile hit to eliminate any
// theoretical false positive (e.g. nibble wrap-around for pathological inputs).
inline bool verify_vampire(uint64_t v, uint64_t a, uint64_t b) {
    std::string sv      = to_base12(v);
    std::string fangs   = to_base12(a) + to_base12(b);
    std::sort(sv.begin(),    sv.end());
    std::sort(fangs.begin(), fangs.end());
    return sv == fangs;
}

constexpr uint64_t power_of_12(int exp) {
    uint64_t r = 1;
    for (int i = 0; i < exp; ++i) r *= 12;
    return r;
}

// ── Worker ───────────────────────────────────────────────────────────────────

void search_vampires(uint64_t a_start, uint64_t a_end,
                     uint64_t min_fang, uint64_t max_fang,
                     uint64_t min_vamp, uint64_t max_vamp,
                     const std::vector<uint64_t>& fang_profiles)
{
    // Thread-local buffer: flush to stdout under lock in one shot per find.
    for (uint64_t a = a_start; a < a_end; ++a) {
        const uint64_t prof_a            = fang_profiles[a - min_fang];
        const bool     a_trailing_zero   = (a % 12 == 0);

        // Tight bounds: keep a*b inside the 2K-digit window.
        uint64_t b_start = (min_vamp + a - 1) / a;
        if (b_start < a) b_start = a;           // enforce a ≤ b (no duplicates)

        uint64_t b_end = max_vamp / a;
        if (b_end > max_fang) b_end = max_fang;

        for (uint64_t b = b_start; b <= b_end; ++b) {
            // Rule: both fangs may not end in 0 simultaneously.
            if (a_trailing_zero && (b % 12 == 0)) continue;

            const uint64_t v      = a * b;
            const uint64_t prof_v = get_profile(v);

            if (prof_a + fang_profiles[b - min_fang] != prof_v) continue;

            // Secondary exact check — guards against nibble false positives.
            if (!verify_vampire(v, a, b)) continue;

            found_count.fetch_add(1, std::memory_order_relaxed);

            // Build the output string before taking the lock.
            std::string line =
                "[FOUND] " + to_base12(v) +
                " = "      + to_base12(a) +
                " * "      + to_base12(b) +
                "  (dec: " + std::to_string(v) +
                " = "      + std::to_string(a) +
                " * "      + std::to_string(b) + ")\n";

            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << line;
            }
        }
    }
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main() {
    constexpr uint64_t MIN_FANG = power_of_12(K - 1);
    constexpr uint64_t MAX_FANG = power_of_12(K)     - 1;
    constexpr uint64_t MIN_VAMP = power_of_12(2*K - 1);
    constexpr uint64_t MAX_VAMP = power_of_12(2*K)   - 1;

    std::cout << "Pre-calculating fang profiles...\n";

    const size_t range_size = MAX_FANG - MIN_FANG + 1;
    std::vector<uint64_t> fang_profiles(range_size);
    for (uint64_t i = MIN_FANG; i <= MAX_FANG; ++i)
        fang_profiles[i - MIN_FANG] = get_profile(i);

    const unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    std::cout << "Starting search using " << num_threads << " threads...\n"
              << "Searching for " << (2*K) << "-digit vampire numbers in Base 12...\n"
              << "--------------------------------------------------\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    const uint64_t total_work      = MAX_FANG - MIN_FANG + 1;
    const uint64_t work_per_thread = total_work / num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        uint64_t a_start = MIN_FANG + i * work_per_thread;
        uint64_t a_end   = (i == num_threads - 1)
                               ? (MAX_FANG + 1)
                               : (a_start + work_per_thread);

        threads.emplace_back(search_vampires,
                             a_start, a_end,
                             MIN_FANG, MAX_FANG,
                             MIN_VAMP, MAX_VAMP,
                             std::cref(fang_profiles));
    }

    for (auto& th : threads) th.join();

    auto elapsed = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now() - start_time).count();

    std::cout << "--------------------------------------------------\n"
              << "Total found : " << found_count.load() << "\n"
              << "Completed in: " << elapsed << " seconds.\n";

    return 0;
}