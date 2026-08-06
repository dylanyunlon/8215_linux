package com.hcn.auto.utils;

import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.List;

public class HFileIOUtils {
    private static int sBufferSize = 524288;

    private HFileIOUtils() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    public static boolean writeFileFromIS(String filePath, InputStream is) {
        return writeFileFromIS((File) HFileUtils.getFileByPath(filePath), is, false, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromIS(String filePath, InputStream is, boolean append) {
        return writeFileFromIS((File)HFileUtils.getFileByPath(filePath), is, append, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromIS(File file, InputStream is) {
        return writeFileFromIS((File)file, is, false, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromIS(File file, InputStream is, boolean append) {
        return writeFileFromIS((File)file, is, append, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromIS(String filePath, InputStream is, OnProgressUpdateListener listener) {
        return writeFileFromIS(HFileUtils.getFileByPath(filePath), is, false, listener);
    }

    public static boolean writeFileFromIS(String filePath, InputStream is, boolean append, OnProgressUpdateListener listener) {
        return writeFileFromIS(HFileUtils.getFileByPath(filePath), is, append, listener);
    }

    public static boolean writeFileFromIS(File file, InputStream is, OnProgressUpdateListener listener) {
        return writeFileFromIS(file, is, false, listener);
    }

    public static boolean writeFileFromIS(File file, InputStream is, boolean append, OnProgressUpdateListener listener) {
        if (is != null && HFileUtils.createOrExistsFile(file)) {
            OutputStream os = null;

            boolean var6;
            try {
                os = new BufferedOutputStream(new FileOutputStream(file, append), sBufferSize);
                if (listener == null) {
                    byte[] data = new byte[sBufferSize];

                    int len;
                    while((len = is.read(data)) != -1) {
                        os.write(data, 0, len);
                    }
                } else {
                    double totalSize = (double)is.available();
                    int curSize = 0;
                    listener.onProgressUpdate(0.0);
                    byte[] data = new byte[sBufferSize];

                    int len;
                    while((len = is.read(data)) != -1) {
                        os.write(data, 0, len);
                        curSize += len;
                        listener.onProgressUpdate((double)curSize / totalSize);
                    }
                }

                boolean var25 = true;
                return var25;
            } catch (IOException var22) {
                var22.printStackTrace();
                var6 = false;
            } finally {
                try {
                    is.close();
                } catch (IOException var21) {
                    var21.printStackTrace();
                }

                try {
                    if (os != null) {
                        os.close();
                    }
                } catch (IOException var20) {
                    var20.printStackTrace();
                }

            }

            return var6;
        } else {
            Log.e("FileIOUtils", "create file <" + file + "> failed.");
            return false;
        }
    }

    public static boolean writeFileFromBytesByStream(String filePath, byte[] bytes) {
        return writeFileFromBytesByStream((File)HFileUtils.getFileByPath(filePath), bytes, false, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromBytesByStream(String filePath, byte[] bytes, boolean append) {
        return writeFileFromBytesByStream((File)HFileUtils.getFileByPath(filePath), bytes, append, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromBytesByStream(File file, byte[] bytes) {
        return writeFileFromBytesByStream((File)file, bytes, false, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromBytesByStream(File file, byte[] bytes, boolean append) {
        return writeFileFromBytesByStream((File)file, bytes, append, (OnProgressUpdateListener)null);
    }

    public static boolean writeFileFromBytesByStream(String filePath, byte[] bytes, OnProgressUpdateListener listener) {
        return writeFileFromBytesByStream(HFileUtils.getFileByPath(filePath), bytes, false, listener);
    }

    public static boolean writeFileFromBytesByStream(String filePath, byte[] bytes, boolean append, OnProgressUpdateListener listener) {
        return writeFileFromBytesByStream(HFileUtils.getFileByPath(filePath), bytes, append, listener);
    }

    public static boolean writeFileFromBytesByStream(File file, byte[] bytes, OnProgressUpdateListener listener) {
        return writeFileFromBytesByStream(file, bytes, false, listener);
    }

    public static boolean writeFileFromBytesByStream(File file, byte[] bytes, boolean append, OnProgressUpdateListener listener) {
        return bytes == null ? false : writeFileFromIS((File)file, new ByteArrayInputStream(bytes), append, listener);
    }

    public static boolean writeFileFromBytesByChannel(String filePath, byte[] bytes, boolean isForce) {
        return writeFileFromBytesByChannel(HFileUtils.getFileByPath(filePath), bytes, false, isForce);
    }

    public static boolean writeFileFromBytesByChannel(String filePath, byte[] bytes, boolean append, boolean isForce) {
        return writeFileFromBytesByChannel(HFileUtils.getFileByPath(filePath), bytes, append, isForce);
    }

    public static boolean writeFileFromBytesByChannel(File file, byte[] bytes, boolean isForce) {
        return writeFileFromBytesByChannel(file, bytes, false, isForce);
    }

    public static boolean writeFileFromBytesByChannel(File file, byte[] bytes, boolean append, boolean isForce) {
        if (bytes == null) {
            Log.e("FileIOUtils", "bytes is null.");
            return false;
        } else if (!HFileUtils.createOrExistsFile(file)) {
            Log.e("FileIOUtils", "create file <" + file + "> failed.");
            return false;
        } else {
            FileChannel fc = null;

            boolean var5;
            try {
                fc = (new FileOutputStream(file, append)).getChannel();
                if (fc == null) {
                    Log.e("FileIOUtils", "fc is null.");
                    var5 = false;
                    return var5;
                }

                fc.position(fc.size());
                fc.write(ByteBuffer.wrap(bytes));
                if (isForce) {
                    fc.force(true);
                }

                var5 = true;
            } catch (IOException var17) {
                var17.printStackTrace();
                boolean var6 = false;
                return var6;
            } finally {
                try {
                    if (fc != null) {
                        fc.close();
                    }
                } catch (IOException var16) {
                    var16.printStackTrace();
                }

            }

            return var5;
        }
    }

    public static boolean writeFileFromBytesByMap(String filePath, byte[] bytes, boolean isForce) {
        return writeFileFromBytesByMap(filePath, bytes, false, isForce);
    }

    public static boolean writeFileFromBytesByMap(String filePath, byte[] bytes, boolean append, boolean isForce) {
        return writeFileFromBytesByMap(HFileUtils.getFileByPath(filePath), bytes, append, isForce);
    }

    public static boolean writeFileFromBytesByMap(File file, byte[] bytes, boolean isForce) {
        return writeFileFromBytesByMap(file, bytes, false, isForce);
    }

    public static boolean writeFileFromBytesByMap(File file, byte[] bytes, boolean append, boolean isForce) {
        if (bytes != null && HFileUtils.createOrExistsFile(file)) {
            FileChannel fc = null;

            boolean var6;
            try {
                fc = (new FileOutputStream(file, append)).getChannel();
                if (fc != null) {
                    MappedByteBuffer mbb = fc.map(FileChannel.MapMode.READ_WRITE, fc.size(), (long)bytes.length);
                    mbb.put(bytes);
                    if (isForce) {
                        mbb.force();
                    }

                    var6 = true;
                    return var6;
                }

                Log.e("FileIOUtils", "fc is null.");
                boolean var5 = false;
                return var5;
            } catch (IOException var17) {
                var17.printStackTrace();
                var6 = false;
            } finally {
                try {
                    if (fc != null) {
                        fc.close();
                    }
                } catch (IOException var16) {
                    var16.printStackTrace();
                }

            }

            return var6;
        } else {
            Log.e("FileIOUtils", "create file <" + file + "> failed.");
            return false;
        }
    }

    public static boolean writeFileFromString(String filePath, String content) {
        return writeFileFromString(HFileUtils.getFileByPath(filePath), content, false);
    }

    public static boolean writeFileFromString(String filePath, String content, boolean append) {
        return writeFileFromString(HFileUtils.getFileByPath(filePath), content, append);
    }

    public static boolean writeFileFromString(File file, String content) {
        return writeFileFromString(file, content, false);
    }

    public static boolean writeFileFromString(File file, String content, boolean append) {
        if (file != null && content != null) {
            if (!HFileUtils.createOrExistsFile(file)) {
                Log.e("FileIOUtils", "create file <" + file + "> failed.");
                return false;
            } else {
                BufferedWriter bw = null;

                boolean var5;
                try {
                    bw = new BufferedWriter(new FileWriter(file, append));
                    bw.write(content);
                    boolean var4 = true;
                    return var4;
                } catch (IOException var15) {
                    var15.printStackTrace();
                    var5 = false;
                } finally {
                    try {
                        if (bw != null) {
                            bw.close();
                        }
                    } catch (IOException var14) {
                        var14.printStackTrace();
                    }

                }

                return var5;
            }
        } else {
            return false;
        }
    }

    public static List<String> readFile2List(String filePath) {
        return readFile2List((File)HFileUtils.getFileByPath(filePath), (String)null);
    }

    public static List<String> readFile2List(String filePath, String charsetName) {
        return readFile2List(HFileUtils.getFileByPath(filePath), charsetName);
    }

    public static List<String> readFile2List(File file) {
        return readFile2List((File)file, 0, Integer.MAX_VALUE, (String)null);
    }

    public static List<String> readFile2List(File file, String charsetName) {
        return readFile2List((File)file, 0, Integer.MAX_VALUE, charsetName);
    }

    public static List<String> readFile2List(String filePath, int st, int end) {
        return readFile2List((File)HFileUtils.getFileByPath(filePath), st, end, (String)null);
    }

    public static List<String> readFile2List(String filePath, int st, int end, String charsetName) {
        return readFile2List(HFileUtils.getFileByPath(filePath), st, end, charsetName);
    }

    public static List<String> readFile2List(File file, int st, int end) {
        return readFile2List((File)file, st, end, (String)null);
    }

    public static List<String> readFile2List(File file, int st, int end, String charsetName) {
        if (!HFileUtils.isFileExists(file)) {
            return null;
        } else if (st > end) {
            return null;
        } else {
            BufferedReader reader = null;

            Object var6;
            try {
                int curLine = 1;
                List<String> list = new ArrayList();
                if (TextUtils.isEmpty(charsetName)) {
                    reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
                } else {
                    reader = new BufferedReader(new InputStreamReader(new FileInputStream(file), charsetName));
                }

                String line;
                for(; (line = reader.readLine()) != null && curLine <= end; ++curLine) {
                    if (st <= curLine && curLine <= end) {
                        list.add(line);
                    }
                }

                ArrayList var8 = (ArrayList) list;
                return var8;
            } catch (IOException var18) {
                var18.printStackTrace();
                var6 = null;
            } finally {
                try {
                    if (reader != null) {
                        reader.close();
                    }
                } catch (IOException var17) {
                    var17.printStackTrace();
                }

            }

            return (List)var6;
        }
    }

    public static String readFile2String(String filePath) {
        return readFile2String((File)HFileUtils.getFileByPath(filePath), (String)null);
    }

    public static String readFile2String(String filePath, String charsetName) {
        return readFile2String(HFileUtils.getFileByPath(filePath), charsetName);
    }

    public static String readFile2String(File file) {
        return readFile2String((File)file, (String)null);
    }

    public static String readFile2String(File file, String charsetName) {
        byte[] bytes = readFile2BytesByStream(file);
        if (bytes == null) {
            return null;
        } else if (TextUtils.isEmpty(charsetName)) {
            return new String(bytes);
        } else {
            try {
                return new String(bytes, charsetName);
            } catch (UnsupportedEncodingException var4) {
                var4.printStackTrace();
                return "";
            }
        }
    }

    public static byte[] readFile2BytesByStream(String filePath) {
        return readFile2BytesByStream((File)HFileUtils.getFileByPath(filePath), (OnProgressUpdateListener)null);
    }

    public static byte[] readFile2BytesByStream(File file) {
        return readFile2BytesByStream((File)file, (OnProgressUpdateListener)null);
    }

    public static byte[] readFile2BytesByStream(String filePath, OnProgressUpdateListener listener) {
        return readFile2BytesByStream(HFileUtils.getFileByPath(filePath), listener);
    }

    public static byte[] readFile2BytesByStream(File file, OnProgressUpdateListener listener) {
        if (!HFileUtils.isFileExists(file)) {
            return null;
        } else {
            try {
                ByteArrayOutputStream os = null;
                InputStream is = new BufferedInputStream(new FileInputStream(file), sBufferSize);

                Object var5;
                try {
                    os = new ByteArrayOutputStream();
                    byte[] b = new byte[sBufferSize];
                    int len;
                    if (listener == null) {
                        while((len = is.read(b, 0, sBufferSize)) != -1) {
                            os.write(b, 0, len);
                        }
                    } else {
                        double totalSize = (double)is.available();
                        int curSize = 0;
                        listener.onProgressUpdate(0.0);

                        while((len = is.read(b, 0, sBufferSize)) != -1) {
                            os.write(b, 0, len);
                            curSize += len;
                            listener.onProgressUpdate((double)curSize / totalSize);
                        }
                    }

                    byte[] var26 = os.toByteArray();
                    return var26;
                } catch (IOException var22) {
                    var22.printStackTrace();
                    var5 = null;
                } finally {
                    try {
                        is.close();
                    } catch (IOException var21) {
                        var21.printStackTrace();
                    }

                    try {
                        if (os != null) {
                            os.close();
                        }
                    } catch (IOException var20) {
                        var20.printStackTrace();
                    }

                }

                return (byte[])var5;
            } catch (FileNotFoundException var24) {
                var24.printStackTrace();
                return null;
            }
        }
    }

    public static byte[] readFile2BytesByChannel(String filePath) {
        return readFile2BytesByChannel(HFileUtils.getFileByPath(filePath));
    }

    public static byte[] readFile2BytesByChannel(File file) {
        if (!HFileUtils.isFileExists(file)) {
            return null;
        } else {
            FileChannel fc = null;

            try {
                fc = (new RandomAccessFile(file, "r")).getChannel();
                if (fc == null) {
                    Log.e("FileIOUtils", "fc is null.");
                    byte[] var16 = new byte[0];
                    return var16;
                } else {
                    ByteBuffer byteBuffer = ByteBuffer.allocate((int)fc.size());

                    while(fc.read(byteBuffer) > 0) {
                    }

                    byte[] var17 = byteBuffer.array();
                    return var17;
                }
            } catch (IOException var14) {
                var14.printStackTrace();
                Object var3 = null;
                return (byte[])var3;
            } finally {
                try {
                    if (fc != null) {
                        fc.close();
                    }
                } catch (IOException var13) {
                    var13.printStackTrace();
                }

            }
        }
    }

    public static void setBufferSize(int bufferSize) {
        sBufferSize = bufferSize;
    }

    public interface OnProgressUpdateListener {
        void onProgressUpdate(double var1);
    }
}
