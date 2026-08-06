package com.hcn.media_data.debug;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.PrintWriter;

/**
 * 媒体调试器
 * <pre>
 *    用来接收外部的 dumpsys 调试命令；
 *    e.g. dumpsys activity service ${class-name} -d music-ui [0-4]
 * </pre>
 *
 * @author 65821
 */
public class MediaDebugger {

    /**
     * 解析 dump 的调试信息参数
     * <p> dumpsys activity service ${class-name} -d service 3
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param args dump 请求的其他参数。
     */
    public static void dumpDebugEvent(@NonNull PrintWriter fout,
                                      @Nullable String[] args) {
        final String DEBUG_CMD = "-d";
        final int DEBUG_ARGS_LEN = 3;

        assert args != null;
        String cmd = args[0];
        if (!DEBUG_CMD.equals(cmd)) {
            fout.println("Error [cmd = " + cmd + "][args.length = " + args.length + "]");
            return;
        }

        if (args.length == DEBUG_ARGS_LEN) {
            String debugType = args[1];
            try {
                int debugLevel = Integer.parseInt(args[2]);
                MediaDebugger.dumpDebugLevel(fout, debugType, debugLevel);
            } catch (Exception e){
                fout.println("unsupported parameter type.");
            }
        } else {
            fout.println("Error [args.length = " + args.length + "]");
            fout.println("    -d option requires parameter error.");
            fout.println("    e.g. open model log 'verbose' level: -d model 0");
            fout.println("    e.g. open service log 'debug' level: -d service 1");
            fout.println("    e.g. open music ui log 'info' level: -d music-ui 2");
            fout.println("    e.g. open video ui log 'warn' level: -d video-ui 3");
            fout.println("    e.g. open all log 'error' level: -d all 4");
        }
    }

    /**
     * 解析调试等级信息
     * <pre>
     *    | --------------------------------------------------- |
     *    | type  | music-ui | video-ui | service | model | all |
     *    | --------------------------------------------------- |
     *    | level |    0    |    1   |    2   |    3   |    4   |
     *    |       | ------- | ------ | ------ | ------ | ------ |
     *    |       | VERBOSE |  DEBUG |  INFO  |  WARN  |  ERROR |
     *    | --------------------------------------------------- |
     * </pre>
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param type 调试类型
     * @param level 调试等级
     */
    private static void dumpDebugLevel(@NonNull PrintWriter fout,
                                       @NonNull String type,
                                       int level) {
        final String MUSIC_UI = "music-ui";
        final String VIDEO_UI = "video-ui";
        final String SERVICE = "service";

        switch (type) {
            case MUSIC_UI:
                dumpMusicUiDebugLevel(fout, level);
                break;
            case VIDEO_UI:
                dumpVideoUiDebugLevel(fout, level);
                break;
            case SERVICE:
            default:
                fout.println("unsupported parameter type.");
                break;
        }
    }

    /**
     * 解析音乐 UI 调试等级
     * <p> 0-V、1-D、2-I、3-W、4-E
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param level 等级
     */
    private static void dumpMusicUiDebugLevel(@NonNull PrintWriter fout,
                                              int level) {
        DebugUiData.MUSIC_DEBUG_V = false;
        DebugUiData.MUSIC_DEBUG_D = false;
        DebugUiData.MUSIC_DEBUG_I = false;

        switch (level) {
            case 0:
                DebugUiData.MUSIC_DEBUG_V = true;
                DebugUiData.MUSIC_DEBUG_D = true;
                DebugUiData.MUSIC_DEBUG_I = true;
                fout.println("open music ui logV/D/I.");
                return;
            case 1:
                DebugUiData.MUSIC_DEBUG_D = true;
                DebugUiData.MUSIC_DEBUG_I = true;
                fout.println("open music ui logD/I.");
                return;
            case 2:
                DebugUiData.MUSIC_DEBUG_I = true;
                fout.println("open music ui logI.");
                return;
            case 3:
            case 4:
            default:
                break;
        }

        fout.println("close music ui logV/D/I.");
    }

    /**
     * 解析视频 UI 调试等级
     * <p> 0-V、1-D、2-I、3-W、4-E
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param level 等级
     */
    private static void dumpVideoUiDebugLevel(@NonNull PrintWriter fout,
                                              int level) {
        DebugUiData.VIDEO_DEBUG_V = false;
        DebugUiData.VIDEO_DEBUG_D = false;
        DebugUiData.VIDEO_DEBUG_I = false;

        switch (level) {
            case 0:
                DebugUiData.VIDEO_DEBUG_V = true;
                DebugUiData.VIDEO_DEBUG_D = true;
                DebugUiData.VIDEO_DEBUG_I = true;
                fout.println("open video ui logV/D/I.");
                return;
            case 1:
                DebugUiData.VIDEO_DEBUG_D = true;
                DebugUiData.VIDEO_DEBUG_I = true;
                fout.println("open video ui logD/I.");
                return;
            case 2:
                DebugUiData.VIDEO_DEBUG_I = true;
                fout.println("open video ui logI.");
                return;
            case 3:
            case 4:
            default:
                break;
        }

        fout.println("close video ui logV/D/I.");
    }

    /**
     * 解析 dump 的调试事件参数
     * <p> dumpsys activity service ${class-name} -e ${event} ${arg}
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param args dump 请求的其他参数。
     */
    public static void dumpMediaEvent(@NonNull PrintWriter fout,
                                      @Nullable String[] args) {
        final String EVENT_CMD = "-e";
        final int EVENT_ARGS_LEN = 3;

        assert args != null;
        String cmd = args[0];
        if (!EVENT_CMD.equals(cmd)) {
            fout.println("Error [cmd = " + cmd + "][args.length = " + args.length + "]");
            return;
        }

        if (args.length == EVENT_ARGS_LEN) {
            String eventType = args[1];
            try {
                String eventArgs = args[2];
                if (eventType.equals("config")) {
                    String event = eventArgs.split(" ")[0];
                    MediaDebugger.dumpConfigEvent(fout, eventArgs);
                }
            } catch (Exception e){
                fout.println("unsupported parameter type.");
            }
        } else {
            fout.println("Error [args.length = " + args.length + "]");
            fout.println("    -e option requires parameter error.");
            fout.println("    e.g. force goto media EQ page: -e config enable_media_eq");
            fout.println("    e.g. force goto media EQ page: -e config disable_media_eq");
        }
    }

    /**
     * 强制设置默认打开内置 EQ 页面
     *
     * @param fout 您应该将状态 dump 到的 PrintWriter
     * @param event dump 请求的配置类型
     */
    private static void dumpConfigEvent(@NonNull PrintWriter fout, String event) {
        switch (event) {
            case "enable_media_eq":
                DebugUiData.FORCE_ENABLE_MEDIA_EQ = true;
                fout.println("force enable media EQ page.");
                break;
            case "disable_media_eq":
                DebugUiData.FORCE_ENABLE_MEDIA_EQ = false;
                fout.println("force disable media EQ page.");
                break;
            default:
                fout.println("unsupported parameter type.");
                break;
        }
    }
}
