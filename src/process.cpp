#include <unistd.h>
#include <signal.h>
#include <libproc.h>
#include <vector>
#include <algorithm>
#include <optional>
#include <filesystem>

class ProcessManager {
public:
    void monitor() {
        while (true) {
            std::lock_guard<std::mutex> lock(mtx);
            for (const auto pid : pids) {
                monitor_one(pid);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void handle_input() {
        while (true) {
            std::cout << "> ";
            std::string cmd;
            std::cin >> cmd;

            if (cmd == "run") {
                std::string path_str;
                std::cin >> path_str;
                std::filesystem::path path{path_str};
                handle_run(path);
            }
            if (cmd == "kill") {
                int pid;
                std::cin >> pid;
                handle_kill(pid);
            }
            // Handle other commands...
        }
    }

    ~ProcessManager() {
        for (const auto pid : pids) {
            handle_kill(pid);
        }
    }

private:
    std::vector<pid_t> pids{};
    std::mutex mtx{};

    auto monitor_one(const pid_t pid) -> bool {
        struct proc_taskinfo info;
        if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &info, sizeof(info)) <= 0) {
            std::cerr << "monitor failed for pid: " << pid << '\n';
            return false;
        }
        std::cout << "pid: " << pid <<'\n';
        std::cout << "CPU usage: " << info.pti_total_user + info.pti_total_system << '\n';
        std::cout << "Memory usage: " << info.pti_resident_size << '\n';
        return true;
    }

    auto find(const pid_t target)
      -> std::optional<std::vector<pid_t>::iterator> {
        auto it = std::find(pids.begin(), pids.end(), target);
        if (it != pids.end()) {
            return it;
        }
        return std::nullopt;
    }

    void handle_run(const std::filesystem::path& path) {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            pid_t pid = fork();
            if (pid == 0) {
                execv(path.c_str(), nullptr);
                exit(1);
            } else if (pid > 0) {
                add_pid(pid);
            } else {
                std::cerr << "Fork failed.\n";
            }
        } else {
            std::cout << "Invalid path.\n";
        }
    }

    void handle_kill(const pid_t pid) {
        remove_pid(pid);
        if (kill(pid, SIGTERM) == -1) {
            std::cout << "Kill failed\n";
        }
    }

    void add_pid(const pid_t pid) {
        std::lock_guard<std::mutex> lock(mtx);
        pids.push_back(pid);
    }

    auto remove_pid(const pid_t pid) -> bool {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = find(pid);
        if (it) {
            std::swap(**it, pids.back());
            pids.pop_back();
        }
        return it != std::nullopt;
    }
};
