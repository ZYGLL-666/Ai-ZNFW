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

ENABLE_QQ_CLEAN=${ENABLE_QQ_CLEAN:-1}
ENABLE_WECHAT_CLEAN=${ENABLE_WECHAT_CLEAN:-1}
ENABLE_DOUYIN_CLEAN=${ENABLE_DOUYIN_CLEAN:-1}
ENABLE_BYTEDANCE_CLEAN=${ENABLE_BYTEDANCE_CLEAN:-1}
ENABLE_KUAISHOU_CLEAN=${ENABLE_KUAISHOU_CLEAN:-1}

log_script_start "CLEAN"

TOTAL_FREED=0

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

safe_clean_dir() {
    local dir=$1 desc=$2
    [ -d "$dir" ] || return 1

    local before=$(du -sm "$dir" 2>/dev/null | awk '{print $1}')
    before=${before:-0}
    find "$dir" -mindepth 1 -maxdepth 1 -exec rm -rf {} + 2>/dev/null
    local after=$(du -sm "$dir" 2>/dev/null | awk '{print $1}')
    after=${after:-0}

    local freed=$((before - after))
    [ "$freed" -lt 0 ] && freed=0

    [ "$freed" -gt 0 ] && log_info "CLEAN" "[$desc] 释放 ${freed}MB"
    TOTAL_FREED=$((TOTAL_FREED + freed))
    return 0
}

clean_app() {
    local pkg=$1 name=$2

    if ! is_app_installed "$pkg"; then
        log_info "CLEAN" "$name 未安装，跳过"
        return
    fi

    log_info "CLEAN" "--- $name 清理开始 ---"

    if is_app_foreground "$pkg"; then
        log_warn "CLEAN" "$name 正在前台，跳过"
        log_info "CLEAN" "--- $name 清理完成 ---"
        return
    fi

    local base="/storage/emulated/0/Android/data/$pkg"
    local media="/storage/emulated/0/Android/media/$pkg"

    safe_clean_dir "$base/cache" "$name cache"
    safe_clean_dir "$base/Cache" "$name Cache"
    safe_clean_dir "$base/files/cache" "$name files/cache"
    safe_clean_dir "$base/cache/video_cache" "$name 视频缓存"
    safe_clean_dir "$base/cache/image_cache" "$name 图片缓存"
    safe_clean_dir "$base/cache/download" "$name 下载缓存"
    safe_clean_dir "$base/cache/videocache" "$name 视频缓存2"
    safe_clean_dir "$base/cache/imagecache" "$name 图片缓存2"

    case "$pkg" in
        com.tencent.mm)
            safe_clean_dir "$base/MicroMsg/Cache" "$name MicroMsg/Cache"
            safe_clean_dir "$base/MicroMsg/appbrand" "$name 小程序缓存"
            ;;
        com.tencent.mobileqq)
            safe_clean_dir "$base/Tencent/QQ_Spaces/Cache" "$name 空间缓存"
            if [ -d "$media/Tencent/QQ_Images" ]; then
                find "$media/Tencent/QQ_Images" -type f -mtime +7 -delete 2>/dev/null
                log_info "CLEAN" "$name 图片缓存：清理7天前文件"
            fi
            ;;
    esac

    find "$base" -type f -name "*.tmp" -mtime +3 -delete 2>/dev/null
    find "$base" -type f -name "*.log" -mtime +7 -delete 2>/dev/null

    log_info "CLEAN" "--- $name 清理完成 ---"
}

log_info "CLEAN" "开始清理任务"

[ "$ENABLE_QQ_CLEAN" = "1" ] && clean_app "com.tencent.mobileqq" "QQ"
[ "$ENABLE_WECHAT_CLEAN" = "1" ] && clean_app "com.tencent.mm" "微信"

if [ "$ENABLE_DOUYIN_CLEAN" = "1" ]; then
    clean_app "com.ss.android.ugc.aweme" "抖音"
    clean_app "com.ss.android.ugc.aweme.lite" "抖音极速版"
    clean_app "com.ss.android.ugc.live" "抖音火山版"
fi

if [ "$ENABLE_BYTEDANCE_CLEAN" = "1" ]; then
    clean_app "com.ss.android.article.news" "今日头条"
    clean_app "com.ss.android.article.lite" "今日头条极速版"
    clean_app "com.ss.android.article.video" "西瓜视频"
    clean_app "com.dragon.read" "番茄小说"
    clean_app "com.xs.fm" "番茄畅听"
    clean_app "com.lemon.lv" "剪映"
    clean_app "com.xt.retouch" "醒图"
    clean_app "com.sup.android.superb" "皮皮虾"
fi

if [ "$ENABLE_KUAISHOU_CLEAN" = "1" ]; then
    clean_app "com.smile.gifmaker" "快手"
    clean_app "com.kuaishou.nebula" "快手极速版"
fi

log_info "CLEAN" "清理任务全部完成"

_now=$(date '+%Y-%m-%d %H:%M')
echo "$_now" > "$MODDIR/.card_time"
echo "$TOTAL_FREED" > "$MODDIR/.card_clean_now"

_prev=$(cat "$MODDIR/.card_clean_total" 2>/dev/null)
case "$_prev" in ''|*[!0-9]*) _prev=0 ;; esac
_new=$((_prev + TOTAL_FREED))
echo "$_new" > "$MODDIR/.card_clean_total"

refresh_card

log_script_end "CLEAN"
