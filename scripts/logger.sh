#!/system/bin/sh

MODID="Ai-Laoliu"
MODDIR="/data/adb/modules/$MODID"
LOGDIR="$MODDIR/logs"
LOGFILE="$LOGDIR/module.log"

LOG_LEVEL_DEBUG=0
LOG_LEVEL_INFO=1
LOG_LEVEL_WARN=2
LOG_LEVEL_ERROR=3
CURRENT_LOG_LEVEL=${LOG_LEVEL:-$LOG_LEVEL_INFO}

[ -d "$LOGDIR" ] || mkdir -p "$LOGDIR"

log_rotate() {
    local keep_days=${LOG_KEEP_DAYS:-7}
    find "$LOGDIR" -name "*.log" -mtime +$keep_days -delete 2>/dev/null
    if [ -f "$LOGFILE" ]; then
        local size=$(stat -c%s "$LOGFILE" 2>/dev/null || echo 0)
        if [ "$size" -gt 5242880 ]; then
            mv "$LOGFILE" "${LOGFILE}.$(date +%Y%m%d%H%M%S)"
            touch "$LOGFILE"
        fi
    fi
}

write_log() {
    local level=$1 tag=$2 msg=$3
    local ts=$(date '+%Y-%m-%d %H:%M:%S')

    local prefix="Ai-智能服务"
    case "$tag" in
        CLEAN)    prefix="Ai-智能清理" ;;
        SUPPRESS) prefix="Ai-智能压制" ;;
        *)        prefix="Ai-智能服务" ;;
    esac

    case $level in
        DEBUG) level_num=$LOG_LEVEL_DEBUG ;;
        INFO)  level_num=$LOG_LEVEL_INFO ;;
        WARN)  level_num=$LOG_LEVEL_WARN ;;
        ERROR) level_num=$LOG_LEVEL_ERROR ;;
        *)     level_num=$LOG_LEVEL_INFO ;;
    esac
    [ "$level_num" -lt "$CURRENT_LOG_LEVEL" ] && return
    echo "[$ts] [$level] [$prefix/$tag] $msg" >> "$LOGFILE"
}

log_debug() { write_log "DEBUG" "$1" "$2"; }
log_info()  { write_log "INFO"  "$1" "$2"; }
log_warn()  { write_log "WARN"  "$1" "$2"; }
log_error() { write_log "ERROR" "$1" "$2"; }

log_script_start() {
    log_rotate
    log_info "$1" "========== 脚本启动 =========="
    log_info "$1" "PID: $$"
}

log_script_end() {
    log_info "$1" "========== 脚本结束 =========="
}

format_size() {
    local mb=$1
    case "$mb" in ''|*[!0-9]*) mb=0 ;; esac
    if [ "$mb" -ge 1024 ]; then
        local gb=$((mb / 1024))
        local rem=$((mb % 1024))
        local dec=$((rem * 10 / 1024))
        echo "${gb}.${dec}G"
    else
        echo "${mb}M"
    fi
}

refresh_card() {
    local prop="$MODDIR/module.prop"
    [ -f "$prop" ] || return 0

    local t=$(cat "$MODDIR/.card_time" 2>/dev/null)
    [ -z "$t" ] && t="未执行"

    local qq=$(cat "$MODDIR/.card_qq" 2>/dev/null)
    case "$qq" in ''|*[!0-9]*) qq=0 ;; esac

    local wx=$(cat "$MODDIR/.card_wechat" 2>/dev/null)
    case "$wx" in ''|*[!0-9]*) wx=0 ;; esac

    local cn=$(cat "$MODDIR/.card_clean_now" 2>/dev/null)
    case "$cn" in ''|*[!0-9]*) cn=0 ;; esac

    local ct=$(cat "$MODDIR/.card_clean_total" 2>/dev/null)
    case "$ct" in ''|*[!0-9]*) ct=0 ;; esac

    local cn_fmt=$(format_size "$cn")
    local ct_fmt=$(format_size "$ct")

    local desc="Ai-智能服务：${t}｜♻️:${cn_fmt}/累计${ct_fmt}｜压:🐧${qq}进程/💬${wx}进程"

    local tmp="${prop}.tmp"
    grep -v '^description=' "$prop" > "$tmp" 2>/dev/null

    if [ -s "$tmp" ] && grep -q '^id=' "$tmp"; then
        echo "description=${desc}" >> "$tmp"
        mv "$tmp" "$prop"
        chmod 0644 "$prop" 2>/dev/null
    else
        rm -f "$tmp"
        return 1
    fi
    return 0
}
