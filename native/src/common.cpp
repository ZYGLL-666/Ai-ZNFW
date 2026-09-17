#include "common.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

std::string get_moddir() {
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) return std::string(cwd);
        return "/data/adb/modules/Ai-Laoliu";
    }
    buf[n] = '\0';
    std::string exe(buf);
    size_t pos = exe.find_last_of('/');
    if (pos == std::string::npos) return "/data/adb/modules/Ai-Laoliu";
    return exe.substr(0, pos);
}

bool ensure_dir(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
    std::string cur;
    for (char c : path) {
        cur.push_back(c);
        if (c == '/' && cur.size() > 1) {
            if (stat(cur.c_str(), &st) != 0) {
                if (mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) return false;
            }
        }
    }
    if (stat(path.c_str(), &st) != 0) {
        if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) return false;
    }
    return true;
}

bool file_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

std::string read_text_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::stringstream ss;
    ss << in.rdbuf();
    std::string s = ss.str();
    if (!s.empty() && s.back() == '\n') s.pop_back();
    return s;
}

long long read_int_file(const std::string& path, long long def) {
    std::string s = read_text_file(path);
    if (s.empty()) return def;
    char* end = nullptr;
    long long v = strtoll(s.c_str(), &end, 10);
    if (end == s.c_str()) return def;
    return v;
}

bool write_text_file(const std::string& path, const std::string& content) {
    ensure_dir(path.substr(0, path.find_last_of('/')));
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;
    out << content;
    out.close();
    return true;
}

long long now_epoch() {
    return (long long)std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string now_str(const char* fmt) {
    time_t t = time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[64];
    strftime(buf, sizeof(buf), fmt, &tmv);
    return std::string(buf);
}

std::string getprop(const std::string& key) {
#ifdef __ANDROID__
    char buf[PROP_VALUE_MAX];
    buf[0] = '\0';
    __system_property_get(key.c_str(), buf);
    return std::string(buf);
#else
    return trim(exec_cmd("getprop " + key));
#endif
}

std::string exec_cmd(const std::string& cmd) {
    std::string out;
    FILE* p = popen(cmd.c_str(), "r");
    if (!p) return out;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), p)) > 0) out.append(buf, n);
    pclose(p);
    return out;
}

std::string format_size(long long mb) {
    if (mb >= 1024) {
        long long gb = mb / 1024;
        long long rem = mb % 1024;
        long long dec = rem * 10 / 1024;
        return std::to_string(gb) + "." + std::to_string(dec) + "G";
    }
    return std::to_string(mb) + "M";
}

std::string default_config_text() {
    return
        "# Ai-智能服务 配置文件\n"
        "\n"
        "# 清理：每天固定时间执行一次（24小时制，0-23）\n"
        "CLEAN_HOUR=3\n"
        "\n"
        "# 压制：间隔多少分钟检查一次\n"
        "SUPPRESS_INTERVAL=10\n"
        "\n"
        "# 功能开关：1=启用，0=禁用\n"
        "ENABLE_QQ_CLEAN=1\n"
        "ENABLE_WECHAT_CLEAN=1\n"
        "ENABLE_DOUYIN_CLEAN=1\n"
        "ENABLE_BYTEDANCE_CLEAN=1\n"
        "ENABLE_KUAISHOU_CLEAN=1\n"
        "ENABLE_SUPPRESS=1\n"
        "\n"
        "# 日志保留天数\n"
        "LOG_KEEP_DAYS=7\n";
}

bool ensure_config(const std::string& path) {
    if (file_exists(path)) return true;
    ensure_dir(path.substr(0, path.find_last_of('/')));
    return write_text_file(path, default_config_text());
}

static int to_int(const std::string& s, int def) {
    if (s.empty()) return def;
    char* end = nullptr;
    long v = strtol(s.c_str(), &end, 10);
    if (end == s.c_str()) return def;
    return (int)v;
}

bool load_config(const std::string& path, Config& cfg) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (key == "CLEAN_HOUR") cfg.clean_hour = to_int(val, 3);
        else if (key == "SUPPRESS_INTERVAL") cfg.suppress_interval = to_int(val, 10);
        else if (key == "ENABLE_QQ_CLEAN") cfg.enable_qq_clean = to_int(val, 1);
        else if (key == "ENABLE_WECHAT_CLEAN") cfg.enable_wechat_clean = to_int(val, 1);
        else if (key == "ENABLE_DOUYIN_CLEAN") cfg.enable_douyin_clean = to_int(val, 1);
        else if (key == "ENABLE_BYTEDANCE_CLEAN") cfg.enable_bytedance_clean = to_int(val, 1);
        else if (key == "ENABLE_KUAISHOU_CLEAN") cfg.enable_kuaishou_clean = to_int(val, 1);
        else if (key == "ENABLE_SUPPRESS") cfg.enable_suppress = to_int(val, 1);
        else if (key == "LOG_KEEP_DAYS") cfg.log_keep_days = to_int(val, 7);
    }
    return true;
}

long long dir_size_bytes(const std::string& path) {
    long long total = 0;
    DIR* d = opendir(path.c_str());
    if (!d) return 0;
    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        std::string full = path + "/" + e->d_name;
        struct stat st;
        if (lstat(full.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            total += dir_size_bytes(full);
        } else if (S_ISREG(st.st_mode)) {
            total += (long long)st.st_blocks * 512;
        }
    }
    closedir(d);
    return total;
}

bool remove_recursive(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;
    if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode)) {
        DIR* d = opendir(path.c_str());
        if (d) {
            struct dirent* e;
            while ((e = readdir(d)) != nullptr) {
                if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
                remove_recursive(path + "/" + e->d_name);
            }
            closedir(d);
        }
        rmdir(path.c_str());
    } else {
        unlink(path.c_str());
    }
    return true;
}

bool safe_clean_dir(const std::string& dir, long long* freed_bytes) {
    struct stat st;
    if (stat(dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        if (freed_bytes) *freed_bytes = 0;
        return false;
    }
    long long before = dir_size_bytes(dir);
    DIR* d = opendir(dir.c_str());
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            remove_recursive(dir + "/" + e->d_name);
        }
        closedir(d);
    }
    long long after = dir_size_bytes(dir);
    long long freed = before - after;
    if (freed < 0) freed = 0;
    if (freed_bytes) *freed_bytes = freed;
    return true;
}

static void delete_mtime_walk(const std::string& dir, const std::string& suffix, int days, time_t now) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;
    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        std::string full = dir + "/" + e->d_name;
        struct stat st;
        if (lstat(full.c_str(), &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            delete_mtime_walk(full, suffix, days, now);
        } else if (S_ISREG(st.st_mode)) {
            bool match = suffix.empty() ||
                (full.size() >= suffix.size() &&
                 full.compare(full.size() - suffix.size(), suffix.size(), suffix) == 0);
            if (match && now - st.st_mtime > (time_t)days * 86400) unlink(full.c_str());
        }
    }
    closedir(d);
}

void delete_files_by_suffix_mtime(const std::string& dir, const std::string& suffix, int days) {
    delete_mtime_walk(dir, suffix, days, time(nullptr));
}

std::vector<int> find_pids(const std::string& pkg) {
    std::vector<int> pids;
    DIR* d = opendir("/proc");
    if (!d) return pids;
    struct dirent* e;
    while ((e = readdir(d)) != nullptr) {
        if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
        std::string cmdline_path = std::string("/proc/") + e->d_name + "/cmdline";
        std::ifstream in(cmdline_path, std::ios::binary);
        std::string name;
        if (in) {
            char c;
            while (in.get(c) && c != '\0') name.push_back(c);
        }
        if (name == pkg || (name.size() > pkg.size() && name.compare(0, pkg.size(), pkg) == 0 && name[pkg.size()] == ':')) {
            pids.push_back(atoi(e->d_name));
        }
    }
    closedir(d);
    return pids;
}

std::string process_state(int pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.compare(0, 6, "State:") == 0) {
            size_t pos = line.find_first_not_of(" \t", 6);
            if (pos == std::string::npos) return "";
            return line.substr(pos, 1);
        }
    }
    return "";
}

bool refresh_card(const std::string& moddir) {
    std::string prop = moddir + "/module.prop";
    if (!file_exists(prop)) return false;

    std::string t = read_text_file(moddir + "/.card_time");
    if (t.empty()) t = "未执行";

    long long qq = read_int_file(moddir + "/.card_qq", 0);
    long long wx = read_int_file(moddir + "/.card_wechat", 0);
    long long cn = read_int_file(moddir + "/.card_clean_now", 0);
    long long ct = read_int_file(moddir + "/.card_clean_total", 0);

    std::string desc = "Ai-智能服务：" + t + "｜♻️:" + format_size(cn) + "/累计" +
                       format_size(ct) + "｜压:🐧" + std::to_string(qq) + "进程/💬" +
                       std::to_string(wx) + "进程";

    std::ifstream in(prop);
    std::vector<std::string> lines;
    bool has_id = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("description=", 0) == 0) continue;
        if (line.rfind("id=", 0) == 0) has_id = true;
        lines.push_back(line);
    }
    in.close();

    if (lines.empty() || !has_id) return false;

    // Atomic replace: write to tmp then rename, so a crash mid-write
    // can never corrupt module.prop.
    std::string tmp = prop + ".tmp";
    std::ofstream out(tmp, std::ios::trunc);
    if (!out) return false;
    for (const auto& l : lines) out << l << "\n";
    out << "description=" << desc << "\n";
    out.close();
    chmod(tmp.c_str(), 0644);
    if (rename(tmp.c_str(), prop.c_str()) != 0) {
        unlink(tmp.c_str());
        return false;
    }
    return true;
}
