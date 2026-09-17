#pragma once

#include <string>

void logger_init(const std::string& moddir, int keep_days);
void log_rotate();
void log_write(const std::string& level, const std::string& tag, const std::string& msg);
void log_debug(const std::string& tag, const std::string& msg);
void log_info(const std::string& tag, const std::string& msg);
void log_warn(const std::string& tag, const std::string& msg);
void log_error(const std::string& tag, const std::string& msg);
void log_script_start(const std::string& tag);
void log_script_end(const std::string& tag);
