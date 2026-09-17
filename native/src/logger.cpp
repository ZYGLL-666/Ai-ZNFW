#include "logger.h"
#include "common.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

std::string g_logdir;
std::string g_logfile;
int g_keep_days = 7;
int g_level = 1;  // INFO

int level_num(const std::string& level) {
    if (level == "DEBUG") return 0;
    if (level == "INFO") return 1;
    if (level == "WARN") return 2;
    if (level == "ERROR") return 3;
    return 1;
}

}  // namespace

void logger_init(const std::string& moddir, int keep_days) {
    g_logdir = moddir + "/logs";
    g_logfile = g_logdir + "/module.log";
    g_keep_days = keep_days;
    ensure_dir(g_logdir);
}

void log_rotate() {
    // Remove logs older than keep_days.
    DIR* d = opendir(g_logdir.c_str());
    if (d) {
        time_t now = time(nullptr);
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            std::string name = e->d_name;
            if (name.size() < 4 || name.compare(name.size() - 4, 4, ".log") != 0) continue;
            std::string full = g_logdir + "/" + name;
            struct stat st;
            if (stat(full.c_str(), &st) == 0) {
                if (now - st.st_mtime > (time_t)g_keep_days * 86400) unlink(full.c_str());
            }
        }
        closedir(d);
    }

    struct stat st;
    if (stat(g_logfile.c_str(), &st) == 0 && st.st_size > 5242880) {
        std::string rotated = g_logfile + "." + now_str("%Y%m%d%H%M%S");
        rename(g_logfile.c_str(), rotated.c_str());
        std::ofstream out(g_logfile, std::ios::trunc);
        out.close();
    }
}

void log_write(const std::string& level, const std::string& tag, const std::string& msg) {
    if (level_num(level) < g_level) return;

    std::string prefix = "Ai-智能服务";
    if (tag == "CLEAN") prefix = "Ai-智能清理";
    else if (tag == "SUPPRESS") prefix = "Ai-智能压制";

    std::string line = "[" + now_str("%Y-%m-%d %H:%M:%S") + "] [" + level + "] [" +
                       prefix + "/" + tag + "] " + msg + "\n";

    std::ofstream out(g_logfile, std::ios::app);
    if (out) out << line;
}

void log_debug(const std::string& tag, const std::string& msg) { log_write("DEBUG", tag, msg); }
void log_info(const std::string& tag, const std::string& msg) { log_write("INFO", tag, msg); }
void log_warn(const std::string& tag, const std::string& msg) { log_write("WARN", tag, msg); }
void log_error(const std::string& tag, const std::string& msg) { log_write("ERROR", tag, msg); }

void log_script_start(const std::string& tag) {
    log_rotate();
    log_info(tag, "========== 脚本启动 ==========");
    log_info(tag, "PID: " + std::to_string(getpid()));
}

void log_script_end(const std::string& tag) {
    log_info(tag, "========== 脚本结束 ==========");
}
