#include "common.h"
#include "logger.h"
#include "programs.h"

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <limits.h>

static std::string self_exe() {
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return "ai_service";
    buf[n] = '\0';
    return std::string(buf);
}

static void spawn(const std::string& subcmd) {
    pid_t pid = fork();
    if (pid == 0) {
        std::string exe = self_exe();
        execl(exe.c_str(), exe.c_str(), subcmd.c_str(), (char*)nullptr);
        _exit(0);
    }
}

int run_service() {
    signal(SIGCHLD, SIG_IGN);

    std::string moddir = get_moddir();
    std::string conf = moddir + "/config/settings.conf";

    while (getprop("sys.boot_completed") != "1") {
        sleep(3);
    }
    sleep(30);

    ensure_config(conf);
    Config cfg;
    load_config(conf, cfg);
    logger_init(moddir, cfg.log_keep_days);

    log_info("INIT", "========== Ai-智能服务 主服务启动 ==========");

    std::string clean_last_day_file = moddir + "/.clean_last_day";
    std::string suppress_last_file = moddir + "/.suppress_last";

    if (!file_exists(suppress_last_file)) write_text_file(suppress_last_file, "0");

    log_info("INIT", "后台循环已启动");

    while (true) {
        ensure_config(conf);
        load_config(conf, cfg);
        logger_init(moddir, cfg.log_keep_days);

        int clean_hour = cfg.clean_hour;
        if (clean_hour < 0 || clean_hour > 23) clean_hour = 3;
        int interval = cfg.suppress_interval;
        if (interval < 1) interval = 10;

        int cur_hour = std::atoi(now_str("%H").c_str());
        long long cur_day = std::atoll(now_str("%Y%m%d").c_str());
        long long clean_last_day = read_int_file(clean_last_day_file, 0);

        if (cur_hour == clean_hour && clean_last_day != cur_day) {
            log_info("CLEAN", "触发每日清理任务（" + std::to_string(clean_hour) + ":00）");
            spawn("clean");
            write_text_file(clean_last_day_file, std::to_string(cur_day));
        }

        if (cfg.enable_suppress) {
            long long now = now_epoch();
            long long suppress_last = read_int_file(suppress_last_file, 0);
            if ((now - suppress_last) / 60 >= interval) {
                log_info("SUPPRESS", "触发压制任务");
                spawn("suppress");
                write_text_file(suppress_last_file, std::to_string(now));
            }
        }

        sleep(60);
    }
}
