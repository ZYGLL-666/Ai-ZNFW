#!/system/bin/sh

case "$0" in
    */*) MODDIR=${0%/*} ;;
    *)   MODDIR=$(pwd) ;;
esac

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

exec 5>&1
exec 1>"$LOGDIR/action_$(date +%Y%m%d_%H%M%S).log" 2>&1

printMsg() { echo "$*" >&5; }

printMsg "=========================================="
printMsg "     Ai-智能服务 - 手动操作"
printMsg "=========================================="
printMsg ""
printMsg "终端命令："
printMsg "  su -c 'sh $MODDIR/scripts/clean.sh'"
printMsg "  su -c 'sh $MODDIR/scripts/suppress.sh'"
printMsg "  su -c 'tail -50 $LOGDIR/module.log'"
printMsg ""
printMsg "日志目录: $LOGDIR"
printMsg "=========================================="

sh "$MODDIR/scripts/clean.sh"
printMsg "清理任务已执行，详情见日志"
