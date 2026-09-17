#!/system/bin/sh
SKIPUNZIP=0

if [ -n "$KSU" ]; then
    ui_print "- 检测到 KernelSU"
elif [ -n "$APATCH" ]; then
    ui_print "- 检测到 APatch"
elif [ -n "$MAGISK_VER" ]; then
    ui_print "- 检测到 Magisk $MAGISK_VER"
else
    ui_print "- 通用模式"
fi

ui_print "*******************************"
ui_print "   Ai-智能服务 正在安装"
ui_print "*******************************"

MODID="Ai-Laoliu"
MODDIR="/data/adb/modules/$MODID"
LOGDIR="$MODDIR/logs"
CONFDIR="$MODDIR/config"

mkdir -p "$LOGDIR"
mkdir -p "$CONFDIR"

set_perm_recursive "$MODPATH" 0 0 0755 0644
set_perm_recursive "$MODPATH/scripts" 0 0 0755 0755
set_perm 0 0 0755 "$MODPATH/service.sh"
set_perm 0 0 0755 "$MODPATH/post-fs-data.sh"
set_perm 0 0 0755 "$MODPATH/action.sh"

if [ ! -f "$CONFDIR/settings.conf" ]; then
    cat > "$CONFDIR/settings.conf" << 'CONF_EOF'
# Ai-智能服务 配置文件

# 清理：每天固定时间执行一次（24小时制，0-23）
CLEAN_HOUR=3

# 压制：间隔多少分钟检查一次
SUPPRESS_INTERVAL=10

# 功能开关：1=启用，0=禁用
ENABLE_QQ_CLEAN=1
ENABLE_WECHAT_CLEAN=1
ENABLE_DOUYIN_CLEAN=1
ENABLE_BYTEDANCE_CLEAN=1
ENABLE_KUAISHOU_CLEAN=1
ENABLE_SUPPRESS=1

# 日志保留天数
LOG_KEEP_DAYS=7
CONF_EOF
fi

_actual_hour=3
_actual_interval=10
if [ -f "$CONFDIR/settings.conf" ]; then
    . "$CONFDIR/settings.conf"
    _actual_hour=${CLEAN_HOUR:-3}
    _actual_interval=${SUPPRESS_INTERVAL:-10}
fi

# 备份累计数据（覆盖升级时保留）
_saved_total=""
_saved_qq=""
_saved_wechat=""
if [ -f "$MODDIR/.card_clean_total" ]; then
    _saved_total=$(cat "$MODDIR/.card_clean_total" 2>/dev/null)
fi
if [ -f "$MODDIR/.card_qq" ]; then
    _saved_qq=$(cat "$MODDIR/.card_qq" 2>/dev/null)
fi
if [ -f "$MODDIR/.card_wechat" ]; then
    _saved_wechat=$(cat "$MODDIR/.card_wechat" 2>/dev/null)
fi

cp "$CONFDIR/settings.conf" "$MODPATH/config/settings.conf"

[ -n "$_saved_total" ] && echo "$_saved_total" > "$MODPATH/.card_clean_total"
[ -n "$_saved_qq" ] && echo "$_saved_qq" > "$MODPATH/.card_qq"
[ -n "$_saved_wechat" ] && echo "$_saved_wechat" > "$MODPATH/.card_wechat"

ui_print "- 配置已初始化"
ui_print "- 清理：每天 ${_actual_hour} 点"
ui_print "- 压制：每 ${_actual_interval} 分钟"
ui_print "- 日志目录: $LOGDIR"
ui_print "- 安装完成，重启后生效"
