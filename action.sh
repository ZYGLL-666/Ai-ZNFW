#!/system/bin/sh
case "$0" in
    */*) MODDIR=${0%/*} ;;
    *)   MODDIR=$(pwd) ;;
esac
[ -x "$MODDIR/ai_service" ] || MODDIR=/data/adb/modules/Ai-Laoliu
[ -x "$MODDIR/ai_service" ] || chmod 0755 "$MODDIR/ai_service" 2>/dev/null
exec "$MODDIR/ai_service" action
