#include <libproc.h>
#include <iostream>
#include <thread>
#include <unistd.h>
#include "process.cpp"

int main() {
    ProcessManager manager;

    std::jthread monitorThread(&ProcessManager::monitor, &manager);
    std::jthread userInputThread(&ProcessManager::handle_input, &manager);

    monitorThread.join();
    userInputThread.join();
}
