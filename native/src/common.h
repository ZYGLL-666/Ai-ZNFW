#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct Config {
    int clean_hour = 3;
    int suppress_interval = 10;
    int enable_qq_clean = 1;
    int enable_wechat_clean = 1;
    int enable_douyin_clean = 1;
    int enable_bytedance_clean = 1;
    int enable_kuaishou_clean = 1;
    int enable_suppress = 1;
    int log_keep_days = 7;
};

std::string trim(const std::string& s);

// Resolve the module directory from /proc/self/exe.
std::string get_moddir();

bool ensure_dir(const std::string& path);
bool file_exists(const std::string& path);
std::string read_text_file(const std::string& path);
long long read_int_file(const std::string& path, long long def = 0);
bool write_text_file(const std::string& path, const std::string& content);

long long now_epoch();
std::string now_str(const char* fmt);

std::string getprop(const std::string& key);
std::string exec_cmd(const std::string& cmd);

std::string format_size(long long mb);

std::string default_config_text();
bool ensure_config(const std::string& path);
bool load_config(const std::string& path, Config& cfg);

long long dir_size_bytes(const std::string& path);
bool remove_recursive(const std::string& path);
// Delete all immediate children of a directory, return freed bytes.
bool safe_clean_dir(const std::string& dir, long long* freed_bytes);
// Recursively delete files matching a suffix older than N days.
void delete_files_by_suffix_mtime(const std::string& dir, const std::string& suffix, int days);

// Process helpers.
std::vector<int> find_pids(const std::string& pkg);
std::string process_state(int pid);

bool refresh_card(const std::string& moddir);
