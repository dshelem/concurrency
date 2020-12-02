#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <mutex>
#include <thread>

std::mutex mutex;
constexpr long SIZE = 100'000'000;
constexpr long FIRST_QUARTER = 25'000'000;
constexpr long SECOND_QUARTER = 50'000'000;
constexpr long THIRD_QUARTER = 75'000'000;
constexpr long FOURTH_QUARTER = 100'000'000;

short nextRandomNumber() noexcept {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<short> distrib(1, 10);

    return distrib(gen);
}

void initVector(std::vector<short>& vec, short (*func)()) {
    if(vec.capacity() < SIZE) {
        vec.reserve(SIZE);
    }
    for(auto i = 0L; i < SIZE; ++i) {
        vec.push_back((*func)());
    }
}

void sumVectorMultiThreaded(long long& sum, const std::vector<short>& vec, const long long beg, const long long end) noexcept {
    std::unique_lock lck(mutex, std::defer_lock);
    for(auto i = beg; i < end; ++i) {
        lck.lock();
        sum += vec[i];
        lck.unlock();
    }
}

int main() {
    std::vector<short> vec;
    long long sum = 0;

    auto start = std::chrono::steady_clock::now();
    initVector(vec, &nextRandomNumber);
    auto finish = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = finish - start;
    std::cout << "init duration = " << duration.count() << " secs\n";

    start = std::chrono::steady_clock::now();
    
    std::thread t1(sumVectorMultiThreaded, std::ref(sum), std::ref(vec), 0, FIRST_QUARTER);
    std::thread t2(sumVectorMultiThreaded, std::ref(sum), std::ref(vec), FIRST_QUARTER, SECOND_QUARTER);
    std::thread t3(sumVectorMultiThreaded, std::ref(sum), std::ref(vec), SECOND_QUARTER, THIRD_QUARTER);
    std::thread t4(sumVectorMultiThreaded, std::ref(sum), std::ref(vec), THIRD_QUARTER, FOURTH_QUARTER);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    finish = std::chrono::steady_clock::now();
    duration = finish - start;
    std::cout << "sum vector duration = " << duration.count() << " secs\n";

    std::cout << "sum(vec) = " << sum << "\n";

    return 0;
}