package com.hcn.media_view.uitls;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.os.Build;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.MalformedInputException;

/**
 * media-view 的工具类
 * <p> 后续可以抽出去，共性比较强；
 *
 * @author 86158
 */
public class Utils {
    private static final String TAG = "media-view";

    /**
     * 是调试版本
     * <pre>
     *    -1: 未知
     *    0: 是调试版本（eng/userdebug）
     *    1：非调试版本 (user)
     * </pre>
     */
    private static int sDebugFlag= -1;

    /**
     * 判断系统是否是 Debug 版本
     * @return 是/否
     */
    public static boolean isDebugVersion() {
        if (sDebugFlag == -1) {
            sDebugFlag = 0;

            // 检查系统版本类型
            if ("user".equals(Build.TYPE)) {
                sDebugFlag = 1;
            }
        }

        return sDebugFlag == 0;
    }

    /**
     * 获取目标视图所在的 Activity
     *
     * @param view 目标视图
     * @return 当前 Activity 对象
     */
    public static Activity getActivityFromView(View view) {
        if (null != view) {
            Context context = view.getContext();
            while (context instanceof ContextWrapper) {
                if (context instanceof Activity) {
                    return (Activity) context;
                }
                context = ((ContextWrapper) context).getBaseContext();
            }
        }
        return null;
    }

    /**
     * 简单地返回文件的字符集类型
     *
     * @param file The file.
     * @return the charset of file simply
     */
    public static String getFileCharsetSimple(final File file) {
        if (file == null) {
            return "";
        }

        if (isUtf8(file)) {
            return "UTF-8";
        }

        int p = 0;
        InputStream is = null;
        try {
            is = new BufferedInputStream(new FileInputStream(file));
            p = (is.read() << 8) + is.read();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        switch (p) {
            case 0xfffe:
                return "unicode";
            case 0xfeff:
                return "UTF-16BE";
            default:
                return "GBK";
        }
    }

    /**
     * Return whether the charset of file is utf8.
     *
     * @param filePath The path of file.
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isUtf8(final String filePath) {
        return isUtf8(new File(filePath));
    }

    /**
     * Return whether the charset of file is utf8.
     *
     * @param file The file.
     * @return {@code true}: yes<br>{@code false}: no
     */
    public static boolean isUtf8(final File file) {
        if (file == null) {
            return false;
        }

        InputStream is = null;
        try {
            byte[] bytes = new byte[24];
            is = new BufferedInputStream(new FileInputStream(file));
            int read = is.read(bytes);
            if (read != -1) {
                byte[] readArr = new byte[read];
                System.arraycopy(bytes, 0, readArr, 0, read);
                return isUtf8(readArr) == 100;
            } else {
                return false;
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    /**
     * UTF-8编码方式
     * ----------------------------------------------
     * 0xxxxxxx
     * 110xxxxx 10xxxxxx
     * 1110xxxx 10xxxxxx 10xxxxxx
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    private static int isUtf8(byte[] raw) {
        int i, len;
        int utf8 = 0, ascii = 0;
        if (raw.length > 3) {
            if ((raw[0] == (byte) 0xEF) && (raw[1] == (byte) 0xBB) && (raw[2] == (byte) 0xBF)) {
                return 100;
            }
        }
        len = raw.length;
        int child = 0;
        for (i = 0; i < len; ) {
            // UTF-8 byte shouldn't be FF and FE
            if ((raw[i] & (byte) 0xFF) == (byte) 0xFF || (raw[i] & (byte) 0xFE) == (byte) 0xFE) {
                return 0;
            }
            if (child == 0) {
                // ASCII format is 0x0*******
                if ((raw[i] & (byte) 0x7F) == raw[i] && raw[i] != 0) {
                    ascii++;
                } else if ((raw[i] & (byte) 0xC0) == (byte) 0xC0) {
                    // 0x11****** maybe is UTF-8
                    for (int bit = 0; bit < 8; bit++) {
                        if ((((byte) (0x80 >> bit)) & raw[i]) == ((byte) (0x80 >> bit))) {
                            child = bit;
                        } else {
                            break;
                        }
                    }
                    utf8++;
                }
                i++;
            } else {
                child = Math.min(raw.length - i, child);
                boolean currentNotUtf8 = false;
                for (int children = 0; children < child; children++) {
                    // format must is 0x10******
                    if ((raw[i + children] & ((byte) 0x80)) != ((byte) 0x80)) {
                        if ((raw[i + children] & (byte) 0x7F) == raw[i + children] && raw[i] != 0) {
                            // ASCII format is 0x0*******
                            ascii++;
                        }
                        currentNotUtf8 = true;
                    }
                }
                if (currentNotUtf8) {
                    utf8--;
                    i++;
                } else {
                    utf8 += child;
                    i += child;
                }
                child = 0;
            }
        }
        // UTF-8 contains ASCII
        if (ascii == len) {
            return 100;
        }
        return (int) (100 * ((float) (utf8 + ascii) / (float) len));
    }

    /** 获取文本文件编码格式 **/
    public static String parseTextFileCharset(final File file) {
        long beginTime = SystemClock.elapsedRealtime();
        String[] charsets = {"US-ASCII", "UTF-8", "GB2312",
                "BIG5", "GBK", "GB18030", "UTF-16BE", "UTF-16LE", "UTF-16", "UNICODE"};

        CharsetDecoder decoder;
        int charsetsIndex = 0;
        BufferedReader br = null;
        FileInputStream fis = null;
        String lineContent;
        String charset = "unknown";

        while (charsetsIndex < charsets.length) {
            decoder = Charset.forName(charsets[charsetsIndex]).newDecoder();
            try {
                fis = new FileInputStream(file);
                br = new BufferedReader(new InputStreamReader(fis, decoder));

                // 读 4 行就基本够了
                int lineIndex = 0;
                do {
                    lineIndex++;
                    lineContent = br.readLine();
                } while (lineContent != null && lineIndex < 5);

                charset = charsets[charsetsIndex];
                break;
            } catch (MalformedInputException e) {
                charsetsIndex++;
            } catch (IOException e) {
                break;
            } finally {
                if (br != null) {
                    try {
                        br.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                if (fis != null) {
                    try {
                        fis.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        if (isDebugVersion()) {
            long diffTime = SystemClock.elapsedRealtime() - beginTime;
            Log.v(TAG, "parseTextFileCharset: " + diffTime);
        }

        return charset;
    }
}
