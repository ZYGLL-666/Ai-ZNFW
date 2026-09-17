#!/system/bin/sh

case "$0" in
    */*) MODDIR=${0%/*} ;;
    *)   MODDIR=$(pwd) ;;
esac

mkdir -p "$MODDIR/logs"
mkdir -p "$MODDIR/config"
chmod 0755 "$MODDIR/scripts/"*.sh 2>/dev/null

[ -f "$MODDIR/scripts/logger.sh" ] || exit 1
. "$MODDIR/scripts/logger.sh"

log_info "BOOT" "post-fs-data 阶段完成，Ai-智能服务 已加载"
