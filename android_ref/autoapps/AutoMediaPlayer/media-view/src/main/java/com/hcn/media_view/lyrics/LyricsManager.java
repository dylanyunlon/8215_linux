package com.hcn.media_view.lyrics;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import com.hcn.media_view.uitls.Utils;

import org.w3c.dom.Text;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 歌词管理器
 *
 * @author 86158
 */
public class LyricsManager {
    private static final String TAG = "Lyrics";

    private static final String US_ASCII = "US-ASCII";
    private static final String UTF_8 = "UTF-8";
    private static final String GB2312 = "GB2312";
    private static final String BIG5 = "BIG5";
    private static final String GBK = "GBK";
    private static final String GB18030 = "GB18030";
    private static final String UTF_16BE = "UTF-16BE";
    private static final String UTF_16LE = "UTF-16LE";
    private static final String UTF_16 = "UTF-16";
    private static final String UNICODE = "UNICODE";

    /**
     * 查询歌词信息返回结果
     * @see #lyricsInfoInMemory(String) 接口；
     */
    public static final int NO_LYRICS_FILE = -1;
    public static final int LYRICS_NOT_IN_MEMORY = 0;
    public static final int LYRICS_IN_MEMORY = 1;

    // 唯一实例
    private static LyricsManager sInstance = null;

    public static LyricsManager instance() {
        if (sInstance == null) {
            synchronized (LyricsManager.class) {
                if (sInstance == null) {
                    sInstance = new LyricsManager();
                }
            }
        }
        return sInstance;
    }

    /** 歌曲名称和路径 */
    protected String mCurSongName = "";
    protected String mCurSongPath = "";

    /**
     * 保存歌词信息
     * <pre>
     *   存储到内存，为了避免歌词重复解析耗时；
     *   一首歌歌词平均信息量小于 2K，2K * 1000 == 2M;
     *   我们这里可以考虑最多存储 1200 首歌的歌词信息，也就 2M 信息不到：
     *   存储 1200 首歌曲歌词信息到内存，满足 99.99% 的用户；
     * </pre>
     */
    private static ConcurrentHashMap<String, List<LyricsRow>> mLyricsMap = null;
    private static final int LYRICS_MAP_MAX_LIMIT = 1200;

    private LyricsManager() {
        super();

        // 静态的变量，只需要一份；
        if (mLyricsMap == null) {
            mLyricsMap = new ConcurrentHashMap<>();
        }
    }

    /**
     * 获取指定路径的歌词文件路径
     * <p> 返回目标文件同级目录下同名且后缀为 .lrc 的歌词文件路径；
     *
     * @param path 文件路径
     * @return 如果文件不存在，返回 null；
     */
    public String getLyricsFilePath(String path) {
        if (TextUtils.isEmpty(path)) {
            return null;
        }

        String pathNameEx;
        int index = path.lastIndexOf(".");
        if (index > 0) {
            pathNameEx = path.substring(0, index);
            String filePath = pathNameEx + ".lrc";
            File fileLrc = new File(filePath);
            if (fileLrc.exists()) {
                return filePath;
            }
        }

        return null;
    }

    /**
     * 目标路径歌词信息是否在内存中
     * <pre>
     *    -1: 不在内存中，且歌词文件不存在；
     *     1: 在内存中，可以不解析，直接读取；
     *     0: 不在内存中，歌词文件也存在，需要解析；
     * </pre>
     *
     * @param path 目标文件路径
     * @return -1、1、0
     */
    public int lyricsInfoInMemory(String path) {
        if (TextUtils.isEmpty(path)) {
            return -1;
        }

        String pathNameEx;
        int index = path.lastIndexOf(".");
        if (index > 0) {
            pathNameEx = path.substring(0, index);
            String filePath = pathNameEx + ".lrc";
            File fileLrc = new File(filePath);
            if (fileLrc.exists()) {
                // 是否已经在内存中
                return mLyricsMap.containsKey(filePath) ? 1 : 0;
            }
        }

        return -1;
    }

    /**
     * 从内存中获取指定路径的歌词信息
     * <p> 配合接口 {@link #lyricsInfoInMemory(String)} 一起使用;
     * @return 歌词信息列表/null
     */
    public List<LyricsRow> getLrcFromMemory(@NonNull String path) {
        String pathNameEx;
        int index = path.lastIndexOf(".");
        if (index > 0) {
            pathNameEx = path.substring(0, index);
            String filePath = pathNameEx + ".lrc";
            File fileLrc = new File(filePath);
            if (fileLrc.exists()) {
                // 是否已经在内存中
                if (mLyricsMap.containsKey(filePath)) {
                    return mLyricsMap.get(filePath);
                }
            }
        }

        return null;
    }

    /**
     * 从歌曲路径获取歌词信息列表
     * <pre>
     *    如果歌词文件很大（恶意），会导致阻塞超时；
     *    会替换当前使用 LyricsManager 管理的歌词播放信息；
     * </pre>
     *
     * @param path 路径
     * @return 歌词列表
     */
    public List<LyricsRow> getLrcFromSong(String path) {
        if (path == null) {
            return null;
        }

        int index = path.lastIndexOf("/");
        if (index > 0) {
            mCurSongName = path.substring(index + 1, path.lastIndexOf("."));
            mCurSongPath = path.substring(0, index + 1);
        }

        return scanLrc(null);
    }

    /**
     * 从歌曲路径获取歌词信息列表
     * <pre>
     *    如果歌词文件很大（恶意），会导致阻塞超时；
     *    不影响当前使用 LyricsManager 管理的歌词播放信息；
     * <pre>
     *
     * @param path 路径
     * @return 歌词列表
     */
    public List<LyricsRow> readLrcFromSong(String path) {
        if (path == null) {
            return null;
        }

        int index = path.lastIndexOf("/");
        int indexPot = path.lastIndexOf(".");
        if (index > 0 && indexPot > 0) {
            String lrcFilePath = path.substring(0, indexPot) + ".lrc";
            return scanLrc(lrcFilePath);
        }

        return null;
    }

    public List<LyricsRow> buildLrcRows(String str) {
        if (TextUtils.isEmpty(str)) {
            return null;
        }

        BufferedReader br = new BufferedReader(new StringReader(str));
        List<LyricsRow> lrcRows = new ArrayList<>();
        String lrcLine;

        try {
            while ((lrcLine = br.readLine()) != null) {
                List<LyricsRow> rows = LyricsRow.createRows(lrcLine.trim());
                if (rows != null && rows.size() > 0) {
                    lrcRows.addAll(rows);
                }
            }

            br.close();
            Collections.sort(lrcRows);

            if (lrcRows.isEmpty()) {
                return null;
            }

            for (int i = 0; i < lrcRows.size() - 1; i++) {
                lrcRows.get(i).setTotalTime(
                        lrcRows.get(i + 1).getTime() - lrcRows.get(i).getTime());
            }

            lrcRows.get(lrcRows.size() - 1).setTotalTime(5000);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        } finally {
            try {
                br.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return lrcRows;
    }

    /** 创建缓存读取器 **/
    private BufferedReader createBufferedReader(
            BufferedInputStream bis, String charsetSimple) throws UnsupportedEncodingException {
        BufferedReader reader;
        switch (charsetSimple) {
            case US_ASCII:
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.US_ASCII));
                break;
            case UTF_8:
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_8));
                break;
            case GB2312:
                reader = new BufferedReader(new InputStreamReader(bis, GB2312));
                break;
            case BIG5:
                reader = new BufferedReader(new InputStreamReader(bis, BIG5));
                break;
            case GB18030:
                reader = new BufferedReader(new InputStreamReader(bis, GB18030));
                break;
            case UTF_16BE:
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_16BE));
                break;
            case UTF_16LE:
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_16LE));
                break;
            case UTF_16:
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_16));
                break;
            case GBK:
                reader = new BufferedReader(new InputStreamReader(bis, GBK));
                break;
            case UNICODE:
            default:
                reader = new BufferedReader(new InputStreamReader(bis, UNICODE));
                break;
        }

        return reader;
    }

    public List<LyricsRow> readLrcFile(String filepath) {
        List<LyricsRow> rows = null;
        File file = new File(filepath);
        FileInputStream fis = null;
        BufferedInputStream bis = null;
        BufferedReader reader = null;

        try {
            fis = new FileInputStream(file);
            bis = new BufferedInputStream(fis);

            // 获取文本编码格式
            String charsetSimple = Utils.parseTextFileCharset(file);
            if ("unknown".equals(charsetSimple)) {
                charsetSimple = Utils.getFileCharsetSimple(file);
            }

            // 打印解析出来的文本编码格式
            if (Utils.isDebugVersion()) {
                Log.v(TAG, "readLrcFile: " + charsetSimple);
            }

            // 根据文本编码格式创建读取器
            reader = createBufferedReader(bis, charsetSimple);

            // 字符编码有效
            if (!Objects.isNull(reader)) {
                String line;
                StringBuilder sb = new StringBuilder();
                try {
                    line = reader.readLine();
                    while (line != null) {
                        sb.append(line).append("\n");
                        line = reader.readLine();
                    }

                    // 构建歌词信息
                    rows = buildLrcRows(sb.toString());

                    // 打印歌词信息
                    if (Utils.isDebugVersion()) {
                        System.out.println(sb);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    if (reader != null) {
                        try {
                            reader.close();
                            reader = null;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (reader != null) {
                try {
                    reader.close();
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
            if (bis != null) {
                try {
                    bis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return rows;
    }

    private List<LyricsRow> scanLrc(String lrcFilePath) {
        String filePath = lrcFilePath;
        if (TextUtils.isEmpty(lrcFilePath)) {
            filePath = mCurSongPath + mCurSongName + ".lrc";
        }

        File fileLrc = new File(filePath);
        if (fileLrc.exists()) {
            // 是否已经在内存中
            if (mLyricsMap.containsKey(filePath)) {
                return mLyricsMap.get(filePath);
            }

            // 不在内存中，则解析歌词文件
            List<LyricsRow> lyricsRowList = readLrcFile(fileLrc.getPath());

            // 最多存储 LYRICS_MAP_MAX_LIMIT 个目标
            if (lyricsRowList != null
                    && mLyricsMap.size() < LYRICS_MAP_MAX_LIMIT) {
                // 把解析结果添加到内存中
                mLyricsMap.put(filePath, lyricsRowList);
            }

            return lyricsRowList;
        }

        return null;
    }
}
