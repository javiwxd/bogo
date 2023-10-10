#include <iostream>
#include <chrono>
#include <string>
#include <mpir.h>
#include <cstring>
#include <algorithm>
#include <vector>
#include <thread>
#include <atomic>
#include <Windows.h>
#include <mutex>

std::atomic<bool> foundSorted(false);
std::mutex mtx;
std::vector<int> threadIterations;

bool is_sorted(const char* numStr) {
    size_t length = std::strlen(numStr);

    mpz_t num;
    mpz_init(num);
    mpz_set_str(num, numStr, 10);

    bool ascending = true;
    for (size_t i = 1; i < length; ++i) {
        if (numStr[i] < numStr[i - 1]) {
            ascending = false;
            break;
        }
    }

    bool descending = true;
    for (size_t i = 1; i < length; ++i) {
        if (numStr[i] > numStr[i - 1]) {
            descending = false;
            break;
        }
    }

    bool sorted = ascending || descending;
    mpz_clear(num);

    return sorted;
}

void randomize_digits(char* numStr) {
    size_t length = std::strlen(numStr);

    mpz_t num;
    mpz_init(num);
    mpz_set_str(num, numStr, 10);

    std::vector<char> digits;
    for (size_t i = 0; i < length; ++i) {
        digits.push_back(numStr[i]);
    }

    std::random_shuffle(digits.begin(), digits.end());

    for (size_t i = 0; i < length; ++i) {
        numStr[i] = digits[i];
    }

    mpz_clear(num);
}

void bogosort_thread(const char* input, int threadId) {
    int count = 0;
    char* num = _strdup(input);

    while (!foundSorted.load()) {
        randomize_digits(num);
        ++count;

        if (is_sorted(num)) {
            foundSorted.store(true);
            std::cout << "Thread " << threadId << " found the sorted number: " << num << " after " << count << " iterations." << std::endl;
        }
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        threadIterations[threadId] = count;
    }

    free(num);
}

BOOL WINAPI ConsoleHandlerRoutine(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_BREAK_EVENT || fdwCtrlType == CTRL_CLOSE_EVENT) {
        std::cout << "Process terminated, loading data..." << std::endl;
        foundSorted.store(true);
		return true;
	}
    else {
		return false;
	}
}

std::string format_duration(const std::chrono::steady_clock::time_point& start,
    const std::chrono::steady_clock::time_point& end) {

    std::chrono::duration<double> durationSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    int hours = std::chrono::duration_cast<std::chrono::hours>(durationSeconds).count();
    durationSeconds -= hours * std::chrono::hours(1);
    int minutes = std::chrono::duration_cast<std::chrono::minutes>(durationSeconds).count();
    durationSeconds -= minutes * std::chrono::minutes(1);
    int seconds = std::chrono::duration_cast<std::chrono::seconds>(durationSeconds).count();
    durationSeconds -= seconds * std::chrono::seconds(1);
    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(durationSeconds).count();
    durationSeconds -= milliseconds * std::chrono::milliseconds(1);
    int microseconds = std::chrono::duration_cast<std::chrono::microseconds>(durationSeconds).count();
    durationSeconds -= microseconds * std::chrono::microseconds(1);
    int nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(durationSeconds).count();

    std::string formattedInterval = std::to_string(hours) + "h " +
        std::to_string(minutes) + "m " +
        std::to_string(seconds) + "s " +
        std::to_string(milliseconds) + "ms " +
        std::to_string(microseconds) + "us " +
        std::to_string(nanoseconds) + "ns";

    return formattedInterval;
}

int __cdecl _main(int argc, char* argv[]) {

    SetConsoleCtrlHandler(ConsoleHandlerRoutine, true);

    std::string input;
    std::cout << "Enter a number: ";
    std::cin >> input;

    const char* num = input.c_str();

    if (is_sorted(num)) {
        std::cout << "The number is sorted" << std::endl;
    }
    else {
        int numThreads = 8;

        std::cout << "Enter the number of threads to use (1 for single-threaded): ";
        std::cin >> numThreads;

        std::vector<std::thread> threads;
        threadIterations.resize(numThreads, 0);

        std::cout << std::endl << "Starting " << numThreads << " threads to find the sorted number." << std::endl << std::endl;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(bogosort_thread, num, i);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        int totalIterations = 0;
        for (int i = 0; i < numThreads; ++i) {
            totalIterations += threadIterations[i];
        }

        std::cout << std::endl << "=======================================" << std::endl;
        std::cout << "Total iterations for all threads: " << totalIterations << std::endl;
        std::cout << "Average iterations per thread: " << static_cast<double>(totalIterations) / static_cast<double>(numThreads) << std::endl;
        std::cout << "Average iterations per second: " << static_cast<double>(totalIterations) / std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << std::endl;
        std::cout << "Average iterations per second per thread: " << static_cast<double>(totalIterations) / static_cast<double>(numThreads) / std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << std::endl;
        std::cout << "Total time: " << format_duration(begin, end) << std::endl;    
        std::cout << "=======================================" << std::endl << std::endl;
    }

    return 0;
}

int __stdcall wmain(int argc, char* argv[])
{
    __security_init_cookie();
    int result = -1;
    __try
    {
        result = _main(argc, argv);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return result;
}