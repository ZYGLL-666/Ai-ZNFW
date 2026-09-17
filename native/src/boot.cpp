#include "common.h"
#include "logger.h"
#include "programs.h"

#include <dirent.h>
#include <sys/stat.h>

int run_boot() {
    std::string moddir = get_moddir();

    ensure_dir(moddir + "/logs");
    ensure_dir(moddir + "/config");

    std::string conf = moddir + "/config/settings.conf";
    ensure_config(conf);
    Config cfg;
    load_config(conf, cfg);

    std::string scripts = moddir + "/scripts";
    DIR* d = opendir(scripts.c_str());
    if (d) {
        struct dirent* e;
        while ((e = readdir(d)) != nullptr) {
            std::string name = e->d_name;
            if (name.size() >= 3 && name.compare(name.size() - 3, 3, ".sh") == 0) {
                chmod((scripts + "/" + name).c_str(), 0755);
            }
        }
        closedir(d);
    }

    logger_init(moddir, cfg.log_keep_days);
    log_info("BOOT", "post-fs-data 阶段完成，Ai-智能服务 已加载");
    return 0;
}
