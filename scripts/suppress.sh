#!/system/bin/sh
case "$0" in
    */*) SCRIPT_DIR=${0%/*} ;;
    *)   SCRIPT_DIR=$(pwd) ;;
esac
MODDIR=$(dirname "$SCRIPT_DIR")
[ -x "$MODDIR/ai_service" ] || MODDIR=/data/adb/modules/Ai-Laoliu
[ -x "$MODDIR/ai_service" ] || chmod 0755 "$MODDIR/ai_service" 2>/dev/null
exec "$MODDIR/ai_service" suppress
