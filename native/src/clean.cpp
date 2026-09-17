#include "common.h"
#include "logger.h"

#include <cstdlib>
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

int run_clean() {
    std::string moddir = get_moddir();
    std::string conf = moddir + "/config/settings.conf";

    ensure_config(conf);
    Config cfg;
    load_config(conf, cfg);
    logger_init(moddir, cfg.log_keep_days);

    log_script_start("CLEAN");

    long long total_freed = 0;

    std::string pkg_list = exec_cmd("pm list packages 2>/dev/null");
    std::string activity_dump = exec_cmd("dumpsys activity activities 2>/dev/null");

    auto clean_dir = [&](const std::string& dir, const std::string& desc) {
        long long freed = 0;
        if (!safe_clean_dir(dir, &freed)) return;
        long long mb = freed / (1024 * 1024);
        if (mb > 0) log_info("CLEAN", "[" + desc + "] 释放 " + std::to_string(mb) + "MB");
        total_freed += freed;
    };

    auto clean_app = [&](const std::string& pkg, const std::string& name) {
        if (!is_app_installed(pkg_list, pkg)) {
            log_info("CLEAN", name + " 未安装，跳过");
            return;
        }
        log_info("CLEAN", "--- " + name + " 清理开始 ---");

        if (is_app_foreground(activity_dump, pkg)) {
            log_warn("CLEAN", name + " 正在前台，跳过");
            log_info("CLEAN", "--- " + name + " 清理完成 ---");
            return;
        }

        std::string base = "/storage/emulated/0/Android/data/" + pkg;
        std::string media = "/storage/emulated/0/Android/media/" + pkg;

        clean_dir(base + "/cache", name + " cache");
        clean_dir(base + "/Cache", name + " Cache");
        clean_dir(base + "/files/cache", name + " files/cache");
        clean_dir(base + "/cache/video_cache", name + " 视频缓存");
        clean_dir(base + "/cache/image_cache", name + " 图片缓存");
        clean_dir(base + "/cache/download", name + " 下载缓存");
        clean_dir(base + "/cache/videocache", name + " 视频缓存2");
        clean_dir(base + "/cache/imagecache", name + " 图片缓存2");

        if (pkg == "com.tencent.mm") {
            clean_dir(base + "/MicroMsg/Cache", name + " MicroMsg/Cache");
            clean_dir(base + "/MicroMsg/appbrand", name + " 小程序缓存");
        } else if (pkg == "com.tencent.mobileqq") {
            clean_dir(base + "/Tencent/QQ_Spaces/Cache", name + " 空间缓存");
            if (file_exists(media + "/Tencent/QQ_Images")) {
                delete_files_by_suffix_mtime(media + "/Tencent/QQ_Images", "", 7);
                log_info("CLEAN", name + " 图片缓存：清理7天前文件");
            }
        }

        delete_files_by_suffix_mtime(base, ".tmp", 3);
        delete_files_by_suffix_mtime(base, ".log", 7);

        log_info("CLEAN", "--- " + name + " 清理完成 ---");
    };

    log_info("CLEAN", "开始清理任务");

    if (cfg.enable_qq_clean) clean_app("com.tencent.mobileqq", "QQ");
    if (cfg.enable_wechat_clean) clean_app("com.tencent.mm", "微信");

    if (cfg.enable_douyin_clean) {
        clean_app("com.ss.android.ugc.aweme", "抖音");
        clean_app("com.ss.android.ugc.aweme.lite", "抖音极速版");
        clean_app("com.ss.android.ugc.live", "抖音火山版");
    }

    if (cfg.enable_bytedance_clean) {
        clean_app("com.ss.android.article.news", "今日头条");
        clean_app("com.ss.android.article.lite", "今日头条极速版");
        clean_app("com.ss.android.article.video", "西瓜视频");
        clean_app("com.dragon.read", "番茄小说");
        clean_app("com.xs.fm", "番茄畅听");
        clean_app("com.lemon.lv", "剪映");
        clean_app("com.xt.retouch", "醒图");
        clean_app("com.sup.android.superb", "皮皮虾");
    }

    if (cfg.enable_kuaishou_clean) {
        clean_app("com.smile.gifmaker", "快手");
        clean_app("com.kuaishou.nebula", "快手极速版");
    }

    log_info("CLEAN", "清理任务全部完成");

    write_text_file(moddir + "/.card_time", now_str("%Y-%m-%d %H:%M"));
    write_text_file(moddir + "/.card_clean_now", std::to_string(total_freed / (1024 * 1024)));

    long long prev = read_int_file(moddir + "/.card_clean_total", 0);
    long long total_mb = total_freed / (1024 * 1024);
    write_text_file(moddir + "/.card_clean_total", std::to_string(prev + total_mb));

    refresh_card(moddir);

    log_script_end("CLEAN");
    return 0;
}
