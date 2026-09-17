#include "common.h"
#include "logger.h"
#include "programs.h"

#include <csignal>
#include <sstream>
#include <unistd.h>

static bool is_app_foreground(const std::string& dump, const std::string& pkg) {
    if (dump.empty()) return false;
    std::istringstream iss(dump);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("mResumedActivity") == std::string::npos &&
            line.find("topResumedActivity") == std::string::npos)
            continue;
        if (line.find(pkg) != std::string::npos) return true;
    }
    return false;
}

static bool is_app_installed(const std::string& list, const std::string& pkg) {
    if (list.empty()) return true;
    std::istringstream iss(list);
    std::string line;
    while (std::getline(iss, line)) {
        if (line == "package:" + pkg) return true;
    }
    return false;
}

static void suppress_app(const std::string& pkg, const std::string& name,
                         const std::string& pkg_list, const std::string& activity_dump,
                         int& app_qq, int& app_wechat) {
    if (!is_app_installed(pkg_list, pkg)) {
        log_info("SUPPRESS", name + " 未安装，跳过");
        return;
    }

    if (is_app_foreground(activity_dump, pkg)) {
        log_debug("SUPPRESS", name + " 在前台，跳过");
        return;
    }

    std::vector<int> pids = find_pids(pkg);
    if (pids.empty()) {
        log_debug("SUPPRESS", name + " 未运行");
        return;
    }

    std::string fg_cache = exec_cmd("dumpsys activity services " + pkg + " 2>/dev/null");
    bool has_fg = fg_cache.find("isForeground=true") != std::string::npos;

    int count = 0;
    for (int pid : pids) {
        std::string state = process_state(pid);
        if (state == "R" || state == "S") {
            if (has_fg) {
                log_debug("SUPPRESS", name + " (PID:" + std::to_string(pid) + ") 有前台服务，跳过");
                continue;
            }
            if (kill(pid, SIGSTOP) == 0) {
                log_info("SUPPRESS", name + " (PID:" + std::to_string(pid) + ") 已冻结");
                ++count;
            }
        }
    }

    if (count > 0) {
        log_info("SUPPRESS", name + " 共冻结 " + std::to_string(count) + " 个进程");
        if (name == "QQ") app_qq = count;
        else if (name == "微信") app_wechat = count;
    }
}

int run_suppress() {
    std::string moddir = get_moddir();
    std::string conf = moddir + "/config/settings.conf";

    ensure_config(conf);
    Config cfg;
    load_config(conf, cfg);
    logger_init(moddir, cfg.log_keep_days);

    log_script_start("SUPPRESS");

    int app_qq = 0;
    int app_wechat = 0;

    std::string pkg_list = exec_cmd("pm list packages 2>/dev/null");
    std::string activity_dump = exec_cmd("dumpsys activity activities 2>/dev/null");

    log_info("SUPPRESS", "开始进程压制扫描");
    suppress_app("com.tencent.mobileqq", "QQ", pkg_list, activity_dump, app_qq, app_wechat);
    suppress_app("com.tencent.mm", "微信", pkg_list, activity_dump, app_qq, app_wechat);
    log_info("SUPPRESS", "进程压制扫描完成");

    write_text_file(moddir + "/.card_time", now_str("%Y-%m-%d %H:%M"));
    write_text_file(moddir + "/.card_qq", std::to_string(app_qq));
    write_text_file(moddir + "/.card_wechat", std::to_string(app_wechat));

    refresh_card(moddir);

    log_script_end("SUPPRESS");
    return 0;
}
