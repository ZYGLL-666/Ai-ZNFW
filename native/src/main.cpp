#include "programs.h"

#include <string>

static std::string basename_of(const std::string& p) {
    size_t pos = p.find_last_of('/');
    return pos == std::string::npos ? p : p.substr(pos + 1);
}

int main(int argc, char** argv) {
    std::string cmd;
    if (argc > 1 && argv[1][0] != '\0') {
        cmd = argv[1];
    } else if (argc > 0) {
        cmd = basename_of(argv[0]);
    }

    if (cmd == "clean") return run_clean();
    if (cmd == "suppress") return run_suppress();
    if (cmd == "service") return run_service();
    if (cmd == "action") return run_action();
    if (cmd == "boot" || cmd == "post-fs-data" || cmd == "post_fs_data") return run_boot();

    return run_service();
}
