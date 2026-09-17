#include "common.h"
#include "logger.h"
#include "programs.h"

#include <unistd.h>

static void out_line(const std::string& s) {
    std::string m = s + "\n";
    write(STDOUT_FILENO, m.c_str(), m.size());
}

int run_action() {
    std::string moddir = get_moddir();
    std::string conf = moddir + "/config/settings.conf";

    ensure_config(conf);
    Config cfg;
    load_config(conf, cfg);
    logger_init(moddir, cfg.log_keep_days);

    std::string logdir = moddir + "/logs";

    out_line("==========================================");
    out_line("     Ai-智能服务 - 手动操作");
    out_line("==========================================");
    out_line("");
    out_line("终端命令：");
    out_line("  su -c '" + moddir + "/ai_service clean'");
    out_line("  su -c '" + moddir + "/ai_service suppress'");
    out_line("  su -c 'tail -50 " + logdir + "/module.log'");
    out_line("");
    out_line("日志目录: " + logdir);
    out_line("==========================================");

    run_clean();
    out_line("清理任务已执行，详情见日志");

    return 0;
}
