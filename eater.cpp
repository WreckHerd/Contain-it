#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>

int main() {
    std::cout << "\n[Container] Starting memory stress test...\n";
    std::vector<char*> allocations;
    int mb_allocated = 0;

    while (true) {
        // 1. Ask the OS for 1 Megabyte
        char* chunk = new char[1024 * 1024];
        
        // 2. Force the kernel to actually map the physical RAM
        std::memset(chunk, 1, 1024 * 1024);
        
        allocations.push_back(chunk);
        mb_allocated++;
        
        std::cout << "  ---> Consuming RAM: " << mb_allocated << " MB\n";
        
        // 3. Pause for 100ms for visual effect
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}