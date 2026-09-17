#!/system/bin/sh

case "$0" in
    */*) MODDIR=${0%/*} ;;
    *)   MODDIR=$(pwd) ;;
esac

while [ "$(getprop sys.boot_completed)" != "1" ]; do
    sleep 3
done
sleep 30

[ -f "$MODDIR/scripts/logger.sh" ] || exit 1
. "$MODDIR/scripts/logger.sh"

log_info "INIT" "========== Ai-智能服务 主服务启动 =========="

CLEAN_LAST_DAY_FILE="$MODDIR/.clean_last_day"
SUPPRESS_LAST_FILE="$MODDIR/.suppress_last"

[ -f "$SUPPRESS_LAST_FILE" ] || echo 0 > "$SUPPRESS_LAST_FILE"

log_info "INIT" "后台循环已启动"

while true; do
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
    . "$MODDIR/config/settings.conf"

    CLEAN_HOUR=${CLEAN_HOUR:-3}
    case "$CLEAN_HOUR" in
        ''|*[!0-9]*) CLEAN_HOUR=3 ;;
    esac

    SUPPRESS_INTERVAL=${SUPPRESS_INTERVAL:-10}
    case "$SUPPRESS_INTERVAL" in
        ''|*[!0-9]*) SUPPRESS_INTERVAL=10 ;;
    esac

    ENABLE_SUPPRESS=${ENABLE_SUPPRESS:-1}

    cur_hour=$(date +%H)
    case "$cur_hour" in
        ''|*[!0-9]*) cur_hour=0 ;;
    esac

    cur_day=$(date +%Y%m%d)
    case "$cur_day" in
        ''|*[!0-9]*) cur_day=0 ;;
    esac

    clean_last_day=$(cat "$CLEAN_LAST_DAY_FILE" 2>/dev/null)
    case "$clean_last_day" in
        ''|*[!0-9]*) clean_last_day=0 ;;
    esac

    cur_hour_num=$((10#$cur_hour))
    clean_hour_num=$((10#$CLEAN_HOUR))

    if [ "$cur_hour_num" -eq "$clean_hour_num" ] && [ "$clean_last_day" != "$cur_day" ]; then
        log_info "CLEAN" "触发每日清理任务（${CLEAN_HOUR}:00）"
        sh "$MODDIR/scripts/clean.sh" &
        echo "$cur_day" > "$CLEAN_LAST_DAY_FILE"
    fi

    if [ "$ENABLE_SUPPRESS" = "1" ]; then
        now=$(date +%s)
        case "$now" in
            ''|*[!0-9]*) now=0 ;;
        esac

        suppress_last=$(cat "$SUPPRESS_LAST_FILE" 2>/dev/null)
        case "$suppress_last" in
            ''|*[!0-9]*) suppress_last=0 ;;
        esac

        if [ $(( (now - suppress_last) / 60 )) -ge "$SUPPRESS_INTERVAL" ]; then
            log_info "SUPPRESS" "触发压制任务"
            sh "$MODDIR/scripts/suppress.sh" &
            echo "$now" > "$SUPPRESS_LAST_FILE"
        fi
    fi

    sleep 60
done
