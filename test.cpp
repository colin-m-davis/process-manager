#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::this_thread::sleep_for(std::chrono::seconds(8));
    std::vector v(10000, 4);
    std::this_thread::sleep_for(std::chrono::seconds(8));
    std::cout << "hello test\n";
    return 0;
}
