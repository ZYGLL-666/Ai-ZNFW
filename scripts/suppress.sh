#!/system/bin/sh

MODID="Ai-Laoliu"
MODDIR="/data/adb/modules/$MODID"

if [ ! -f "$MODDIR/config/settings.conf" ]; then
    mkdir -p "$MODDIR/config"
    cat > "$MODDIR/config/settings.conf" << 'CONF_EOF'
# Ai-智能服务 配置文件
CLEAN_HOUR=3
SUPPRESS_INTERVAL=10
ENABLE_QQ_CLEAN=1
ENABLE_WECHAT_CLEAN=1
ENABLE_DOUYIN_CLEAN=1
ENABLE_BYTEDANCE_CLEAN=1
ENABLE_KUAISHOU_CLEAN=1
ENABLE_SUPPRESS=1
LOG_KEEP_DAYS=7
CONF_EOF
fi

[ -f "$MODDIR/scripts/logger.sh" ] || exit 1

. "$MODDIR/config/settings.conf"
. "$MODDIR/scripts/logger.sh"

ENABLE_SUPPRESS=${ENABLE_SUPPRESS:-1}

log_script_start "SUPPRESS"

APP_QQ=0
APP_WECHAT=0

PKG_LIST=$(pm list packages 2>/dev/null)
ACTIVITY_DUMP=$(dumpsys activity activities 2>/dev/null)

is_app_foreground() {
    local pkg=$1
    [ -z "$ACTIVITY_DUMP" ] && return 1
    echo "$ACTIVITY_DUMP" \
        | grep -E "mResumedActivity|topResumedActivity" \
        | grep -q "$pkg"
}

is_app_installed() {
    local pkg=$1
    [ -z "$PKG_LIST" ] && return 0
    echo "$PKG_LIST" | grep -qx "package:${pkg}"
}

get_process_state() {
    grep -m1 "^State:" "/proc/$1/status" 2>/dev/null | awk '{print $2}'
}

suppress_app() {
    local pkg=$1 name=$2

    if ! is_app_installed "$pkg"; then
        log_info "SUPPRESS" "$name 未安装，跳过"
        return 0
    fi

    if is_app_foreground "$pkg"; then
        log_debug "SUPPRESS" "$name 在前台，跳过"
        return 0
    fi

    local pids=$(pidof "$pkg" 2>/dev/null)
    if [ -z "$pids" ]; then
        log_debug "SUPPRESS" "$name 未运行"
        return 0
    fi

    local fg_cache=$(dumpsys activity services "$pkg" 2>/dev/null)
    local has_fg=0
    echo "$fg_cache" | grep -q "isForeground=true" && has_fg=1

    local count=0
    for pid in $pids; do
        local state=$(get_process_state "$pid")
        if [ "$state" = "R" ] || [ "$state" = "S" ]; then
            if [ "$has_fg" = "1" ]; then
                log_debug "SUPPRESS" "$name (PID:$pid) 有前台服务，跳过"
                continue
            fi
            kill -STOP "$pid" 2>/dev/null
            if [ $? -eq 0 ]; then
                log_info "SUPPRESS" "$name (PID:$pid) 已冻结"
                count=$((count + 1))
            fi
        fi
    done

    if [ "$count" -gt 0 ]; then
        log_info "SUPPRESS" "$name 共冻结 $count 个进程"
        case "$name" in
            QQ)   APP_QQ=$count ;;
            微信) APP_WECHAT=$count ;;
        esac
    fi
    return 0
}

log_info "SUPPRESS" "开始进程压制扫描"
suppress_app "com.tencent.mobileqq" "QQ"
suppress_app "com.tencent.mm" "微信"
log_info "SUPPRESS" "进程压制扫描完成"

_now=$(date '+%Y-%m-%d %H:%M')
echo "$_now" > "$MODDIR/.card_time"
echo "$APP_QQ" > "$MODDIR/.card_qq"
echo "$APP_WECHAT" > "$MODDIR/.card_wechat"

refresh_card

log_script_end "SUPPRESS"
