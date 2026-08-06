package com.hcn.auto.utils;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Build;
import android.os.StatFs;
import android.text.TextUtils;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import javax.net.ssl.HttpsURLConnection;

public class HFileUtils {
    private static final String LINE_SEP = System.getProperty("line.separator");

    private HFileUtils() {
        throw new UnsupportedOperationException("u can't instantiate me...");
    }

    public static File getFileByPath(String filePath) {
        return TextUtils.isEmpty(filePath) ? null : new File(filePath);
    }

    public static boolean isFileExists(File file) {
        if (file == null) {
            return false;
        } else {
            return file.exists() ? true : isFileExists(file.getAbsolutePath());
        }
    }

    public static boolean isFileExists(String filePath) {
        File file = getFileByPath(filePath);
        if (file == null) {
            return false;
        } else {
            return file.exists() ? true : isFileExistsApi29(filePath);
        }
    }

    private static boolean isFileExistsApi29(String filePath) {
        if (Build.VERSION.SDK_INT >= 29) {
            try {
                Uri uri = Uri.parse(filePath);
                ContentResolver cr = HUtilsEx.getApp().getContentResolver();
                AssetFileDescriptor afd = cr.openAssetFileDescriptor(uri, "r");
                if (afd == null) {
                    return false;
                } else {
                    try {
                        afd.close();
                    } catch (IOException var5) {
                    }

                    return true;
                }
            } catch (FileNotFoundException var6) {
                return false;
            }
        } else {
            return false;
        }
    }

    public static boolean rename(String filePath, String newName) {
        return rename(getFileByPath(filePath), newName);
    }

    public static boolean rename(File file, String newName) {
        if (file == null) {
            return false;
        } else if (!file.exists()) {
            return false;
        } else if (TextUtils.isEmpty(newName)) {
            return false;
        } else if (newName.equals(file.getName())) {
            return true;
        } else {
            File newFile = new File(file.getParent() + File.separator + newName);
            return !newFile.exists() && file.renameTo(newFile);
        }
    }

    public static boolean isDir(String dirPath) {
        return isDir(getFileByPath(dirPath));
    }

    public static boolean isDir(File file) {
        return file != null && file.exists() && file.isDirectory();
    }

    public static boolean isFile(String filePath) {
        return isFile(getFileByPath(filePath));
    }

    public static boolean isFile(File file) {
        return file != null && file.exists() && file.isFile();
    }

    public static boolean createOrExistsDir(String dirPath) {
        return createOrExistsDir(getFileByPath(dirPath));
    }

    public static boolean createOrExistsDir(File file) {
        boolean var10000;
        label25: {
            if (file != null) {
                if (file.exists()) {
                    if (file.isDirectory()) {
                        break label25;
                    }
                } else if (file.mkdirs()) {
                    break label25;
                }
            }

            var10000 = false;
            return var10000;
        }

        var10000 = true;
        return var10000;
    }

    public static boolean createOrExistsFile(String filePath) {
        return createOrExistsFile(getFileByPath(filePath));
    }

    public static boolean createOrExistsFile(File file) {
        if (file == null) {
            return false;
        } else if (file.exists()) {
            return file.isFile();
        } else if (!createOrExistsDir(file.getParentFile())) {
            return false;
        } else {
            try {
                return file.createNewFile();
            } catch (IOException var2) {
                var2.printStackTrace();
                return false;
            }
        }
    }

    public static boolean createFileByDeleteOldFile(String filePath) {
        return createFileByDeleteOldFile(getFileByPath(filePath));
    }

    public static boolean createFileByDeleteOldFile(File file) {
        if (file == null) {
            return false;
        } else if (file.exists() && !file.delete()) {
            return false;
        } else if (!createOrExistsDir(file.getParentFile())) {
            return false;
        } else {
            try {
                return file.createNewFile();
            } catch (IOException var2) {
                var2.printStackTrace();
                return false;
            }
        }
    }

    public static boolean copy(String srcPath, String destPath) {
        return copy((File)getFileByPath(srcPath), (File)getFileByPath(destPath), (OnReplaceListener)null);
    }

    public static boolean copy(String srcPath, String destPath, OnReplaceListener listener) {
        return copy(getFileByPath(srcPath), getFileByPath(destPath), listener);
    }

    public static boolean copy(File src, File dest) {
        return copy((File)src, (File)dest, (OnReplaceListener)null);
    }

    public static boolean copy(File src, File dest, OnReplaceListener listener) {
        if (src == null) {
            return false;
        } else {
            return src.isDirectory() ? copyDir(src, dest, listener) : copyFile(src, dest, listener);
        }
    }

    private static boolean copyDir(File srcDir, File destDir, OnReplaceListener listener) {
        return copyOrMoveDir(srcDir, destDir, listener, false);
    }

    private static boolean copyFile(File srcFile, File destFile, OnReplaceListener listener) {
        return copyOrMoveFile(srcFile, destFile, listener, false);
    }

    public static boolean move(String srcPath, String destPath) {
        return move((File)getFileByPath(srcPath), (File)getFileByPath(destPath), (OnReplaceListener)null);
    }

    public static boolean move(String srcPath, String destPath, OnReplaceListener listener) {
        return move(getFileByPath(srcPath), getFileByPath(destPath), listener);
    }

    public static boolean move(File src, File dest) {
        return move((File)src, (File)dest, (OnReplaceListener)null);
    }

    public static boolean move(File src, File dest, OnReplaceListener listener) {
        if (src == null) {
            return false;
        } else {
            return src.isDirectory() ? moveDir(src, dest, listener) : moveFile(src, dest, listener);
        }
    }

    public static boolean moveDir(File srcDir, File destDir, OnReplaceListener listener) {
        return copyOrMoveDir(srcDir, destDir, listener, true);
    }

    public static boolean moveFile(File srcFile, File destFile, OnReplaceListener listener) {
        return copyOrMoveFile(srcFile, destFile, listener, true);
    }

    private static boolean copyOrMoveDir(File srcDir, File destDir, OnReplaceListener listener, boolean isMove) {
        if (srcDir != null && destDir != null) {
            String srcPath = srcDir.getPath() + File.separator;
            String destPath = destDir.getPath() + File.separator;
            if (destPath.contains(srcPath)) {
                return false;
            } else if (srcDir.exists() && srcDir.isDirectory()) {
                if (!createOrExistsDir(destDir)) {
                    return false;
                } else {
                    File[] files = srcDir.listFiles();
                    if (files != null && files.length > 0) {
                        File[] var7 = files;
                        int var8 = files.length;

                        for(int var9 = 0; var9 < var8; ++var9) {
                            File file = var7[var9];
                            File oneDestFile = new File(destPath + file.getName());
                            if (file.isFile()) {
                                if (!copyOrMoveFile(file, oneDestFile, listener, isMove)) {
                                    return false;
                                }
                            } else if (file.isDirectory() && !copyOrMoveDir(file, oneDestFile, listener, isMove)) {
                                return false;
                            }
                        }
                    }

                    return !isMove || deleteDir(srcDir);
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    private static boolean copyOrMoveFile(File srcFile, File destFile, OnReplaceListener listener, boolean isMove) {
        if (srcFile != null && destFile != null) {
            if (srcFile.equals(destFile)) {
                return false;
            } else if (srcFile.exists() && srcFile.isFile()) {
                if (destFile.exists()) {
                    if (listener != null && !listener.onReplace(srcFile, destFile)) {
                        return true;
                    }

                    if (!destFile.delete()) {
                        return false;
                    }
                }

                if (!createOrExistsDir(destFile.getParentFile())) {
                    return false;
                } else {
                    try {
                        return HFileIOUtils.writeFileFromIS(destFile.getAbsolutePath(), new FileInputStream(srcFile)) && (!isMove || deleteFile(srcFile));
                    } catch (FileNotFoundException var5) {
                        var5.printStackTrace();
                        return false;
                    }
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    public static boolean delete(String filePath) {
        return delete(getFileByPath(filePath));
    }

    public static boolean delete(File file) {
        if (file == null) {
            return false;
        } else {
            return file.isDirectory() ? deleteDir(file) : deleteFile(file);
        }
    }

    private static boolean deleteDir(File dir) {
        if (dir == null) {
            return false;
        } else if (!dir.exists()) {
            return true;
        } else if (!dir.isDirectory()) {
            return false;
        } else {
            File[] files = dir.listFiles();
            if (files != null && files.length > 0) {
                File[] var2 = files;
                int var3 = files.length;

                for(int var4 = 0; var4 < var3; ++var4) {
                    File file = var2[var4];
                    if (file.isFile()) {
                        if (!file.delete()) {
                            return false;
                        }
                    } else if (file.isDirectory() && !deleteDir(file)) {
                        return false;
                    }
                }
            }

            return dir.delete();
        }
    }

    private static boolean deleteFile(File file) {
        return file != null && (!file.exists() || file.isFile() && file.delete());
    }

    public static boolean deleteAllInDir(String dirPath) {
        return deleteAllInDir(getFileByPath(dirPath));
    }

    public static boolean deleteAllInDir(File dir) {
        return deleteFilesInDirWithFilter(dir, new FileFilter() {
            public boolean accept(File pathname) {
                return true;
            }
        });
    }

    public static boolean deleteFilesInDir(String dirPath) {
        return deleteFilesInDir(getFileByPath(dirPath));
    }

    public static boolean deleteFilesInDir(File dir) {
        return deleteFilesInDirWithFilter(dir, new FileFilter() {
            public boolean accept(File pathname) {
                return pathname.isFile();
            }
        });
    }

    public static boolean deleteFilesInDirWithFilter(String dirPath, FileFilter filter) {
        return deleteFilesInDirWithFilter(getFileByPath(dirPath), filter);
    }

    public static boolean deleteFilesInDirWithFilter(File dir, FileFilter filter) {
        if (dir != null && filter != null) {
            if (!dir.exists()) {
                return true;
            } else if (!dir.isDirectory()) {
                return false;
            } else {
                File[] files = dir.listFiles();
                if (files != null && files.length != 0) {
                    File[] var3 = files;
                    int var4 = files.length;

                    for(int var5 = 0; var5 < var4; ++var5) {
                        File file = var3[var5];
                        if (filter.accept(file)) {
                            if (file.isFile()) {
                                if (!file.delete()) {
                                    return false;
                                }
                            } else if (file.isDirectory() && !deleteDir(file)) {
                                return false;
                            }
                        }
                    }
                }

                return true;
            }
        } else {
            return false;
        }
    }

    public static List<File> listFilesInDir(String dirPath) {
        return listFilesInDir((String)dirPath, (Comparator)null);
    }

    public static List<File> listFilesInDir(File dir) {
        return listFilesInDir((File)dir, (Comparator)null);
    }

    public static List<File> listFilesInDir(String dirPath, Comparator<File> comparator) {
        return listFilesInDir(getFileByPath(dirPath), false, comparator);
    }

    public static List<File> listFilesInDir(File dir, Comparator<File> comparator) {
        return listFilesInDir(dir, false, comparator);
    }

    public static List<File> listFilesInDir(String dirPath, boolean isRecursive) {
        return listFilesInDir(getFileByPath(dirPath), isRecursive);
    }

    public static List<File> listFilesInDir(File dir, boolean isRecursive) {
        return listFilesInDir((File)dir, isRecursive, (Comparator)null);
    }

    public static List<File> listFilesInDir(String dirPath, boolean isRecursive, Comparator<File> comparator) {
        return listFilesInDir(getFileByPath(dirPath), isRecursive, comparator);
    }

    public static List<File> listFilesInDir(File dir, boolean isRecursive, Comparator<File> comparator) {
        return listFilesInDirWithFilter(dir, new FileFilter() {
            public boolean accept(File pathname) {
                return true;
            }
        }, isRecursive, comparator);
    }

    public static List<File> listFilesInDirWithFilter(String dirPath, FileFilter filter) {
        return listFilesInDirWithFilter(getFileByPath(dirPath), filter);
    }

    public static List<File> listFilesInDirWithFilter(File dir, FileFilter filter) {
        return listFilesInDirWithFilter((File)dir, filter, false, (Comparator)null);
    }

    public static List<File> listFilesInDirWithFilter(String dirPath, FileFilter filter, Comparator<File> comparator) {
        return listFilesInDirWithFilter(getFileByPath(dirPath), filter, comparator);
    }

    public static List<File> listFilesInDirWithFilter(File dir, FileFilter filter, Comparator<File> comparator) {
        return listFilesInDirWithFilter(dir, filter, false, comparator);
    }

    public static List<File> listFilesInDirWithFilter(String dirPath, FileFilter filter, boolean isRecursive) {
        return listFilesInDirWithFilter(getFileByPath(dirPath), filter, isRecursive);
    }

    public static List<File> listFilesInDirWithFilter(File dir, FileFilter filter, boolean isRecursive) {
        return listFilesInDirWithFilter((File)dir, filter, isRecursive, (Comparator)null);
    }

    public static List<File> listFilesInDirWithFilter(String dirPath, FileFilter filter, boolean isRecursive, Comparator<File> comparator) {
        return listFilesInDirWithFilter(getFileByPath(dirPath), filter, isRecursive, comparator);
    }

    public static List<File> listFilesInDirWithFilter(File dir, FileFilter filter, boolean isRecursive, Comparator<File> comparator) {
        List<File> files = listFilesInDirWithFilterInner(dir, filter, isRecursive);
        if (comparator != null) {
            Collections.sort(files, comparator);
        }

        return files;
    }

    private static List<File> listFilesInDirWithFilterInner(File dir, FileFilter filter, boolean isRecursive) {
        List<File> list = new ArrayList();
        if (!isDir(dir)) {
            return list;
        } else {
            File[] files = dir.listFiles();
            if (files != null && files.length > 0) {
                File[] var5 = files;
                int var6 = files.length;

                for(int var7 = 0; var7 < var6; ++var7) {
                    File file = var5[var7];
                    if (filter.accept(file)) {
                        list.add(file);
                    }

                    if (isRecursive && file.isDirectory()) {
                        list.addAll(listFilesInDirWithFilterInner(file, filter, true));
                    }
                }
            }

            return list;
        }
    }

    public static long getFileLastModified(String filePath) {
        return getFileLastModified(getFileByPath(filePath));
    }

    public static long getFileLastModified(File file) {
        return file == null ? -1L : file.lastModified();
    }

    public static String getFileCharsetSimple(String filePath) {
        return getFileCharsetSimple(getFileByPath(filePath));
    }

    public static String getFileCharsetSimple(File file) {
        if (file == null) {
            return "";
        } else if (isUtf8(file)) {
            return "UTF-8";
        } else {
            int p = 0;
            InputStream is = null;

            try {
                is = new BufferedInputStream(new FileInputStream(file));
                p = (is.read() << 8) + is.read();
            } catch (IOException var12) {
                var12.printStackTrace();
            } finally {
                try {
                    if (is != null) {
                        is.close();
                    }
                } catch (IOException var11) {
                    var11.printStackTrace();
                }

            }

            switch (p) {
                case 65279:
                    return "UTF-16BE";
                case 65534:
                    return "Unicode";
                default:
                    return "GBK";
            }
        }
    }

    public static boolean isUtf8(String filePath) {
        return isUtf8(getFileByPath(filePath));
    }

    public static boolean isUtf8(File file) {
        if (file == null) {
            return false;
        } else {
            InputStream is = null;

            boolean var5;
            try {
                byte[] bytes = new byte[24];
                is = new BufferedInputStream(new FileInputStream(file));
                int read = is.read(bytes);
                if (read == -1) {
                    boolean var18 = false;
                    return var18;
                }

                byte[] readArr = new byte[read];
                System.arraycopy(bytes, 0, readArr, 0, read);
                var5 = isUtf8(readArr) == 100;
            } catch (IOException var16) {
                var16.printStackTrace();
                return false;
            } finally {
                try {
                    if (is != null) {
                        is.close();
                    }
                } catch (IOException var15) {
                    var15.printStackTrace();
                }

            }

            return var5;
        }
    }

    private static int isUtf8(byte[] raw) {
        int utf8 = 0;
        int ascii = 0;
        if (raw.length > 3 && raw[0] == -17 && raw[1] == -69 && raw[2] == -65) {
            return 100;
        } else {
            int len = raw.length;
            int child = 0;
            int i = 0;

            while(i < len) {
                if ((raw[i] & -1) == -1 || (raw[i] & -2) == -2) {
                    return 0;
                }

                if (child == 0) {
                    if ((raw[i] & 127) == raw[i] && raw[i] != 0) {
                        ++ascii;
                    } else if ((raw[i] & -64) == -64) {
                        for(int bit = 0; bit < 8 && ((byte)(128 >> bit) & raw[i]) == (byte)(128 >> bit); child = bit++) {
                        }

                        ++utf8;
                    }

                    ++i;
                } else {
                    child = raw.length - i > child ? child : raw.length - i;
                    boolean currentNotUtf8 = false;

                    for(int children = 0; children < child; ++children) {
                        if ((raw[i + children] & -128) != -128) {
                            if ((raw[i + children] & 127) == raw[i + children] && raw[i] != 0) {
                                ++ascii;
                            }

                            currentNotUtf8 = true;
                        }
                    }

                    if (currentNotUtf8) {
                        --utf8;
                        ++i;
                    } else {
                        utf8 += child;
                        i += child;
                    }

                    child = 0;
                }
            }

            if (ascii == len) {
                return 100;
            } else {
                return (int)(100.0F * ((float)(utf8 + ascii) / (float)len));
            }
        }
    }

    public static int getFileLines(String filePath) {
        return getFileLines(getFileByPath(filePath));
    }

    public static int getFileLines(File file) {
        int count = 1;
        InputStream is = null;

        try {
            is = new BufferedInputStream(new FileInputStream(file));
            byte[] buffer = new byte[1024];
            int readChars;
            int i;
            if (LINE_SEP.endsWith("\n")) {
                while((readChars = is.read(buffer, 0, 1024)) != -1) {
                    for(i = 0; i < readChars; ++i) {
                        if (buffer[i] == 10) {
                            ++count;
                        }
                    }
                }
            } else {
                while((readChars = is.read(buffer, 0, 1024)) != -1) {
                    for(i = 0; i < readChars; ++i) {
                        if (buffer[i] == 13) {
                            ++count;
                        }
                    }
                }
            }
        } catch (IOException var14) {
            var14.printStackTrace();
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException var13) {
                var13.printStackTrace();
            }

        }

        return count;
    }

    public static String getSize(String filePath) {
        return getSize(getFileByPath(filePath));
    }

    public static String getSize(File file) {
        if (file == null) {
            return "";
        } else {
            return file.isDirectory() ? getDirSize(file) : getFileSize(file);
        }
    }

    private static String getDirSize(File dir) {
        long len = getDirLength(dir);
        return len == -1L ? "" : HConvertUtils.byte2FitMemorySize(len);
    }

    private static String getFileSize(File file) {
        long len = getFileLength(file);
        return len == -1L ? "" : HConvertUtils.byte2FitMemorySize(len);
    }

    public static long getLength(String filePath) {
        return getLength(getFileByPath(filePath));
    }

    public static long getLength(File file) {
        if (file == null) {
            return 0L;
        } else {
            return file.isDirectory() ? getDirLength(file) : getFileLength(file);
        }
    }

    private static long getDirLength(File dir) {
        if (!isDir(dir)) {
            return 0L;
        } else {
            long len = 0L;
            File[] files = dir.listFiles();
            if (files != null && files.length > 0) {
                File[] var4 = files;
                int var5 = files.length;

                for(int var6 = 0; var6 < var5; ++var6) {
                    File file = var4[var6];
                    if (file.isDirectory()) {
                        len += getDirLength(file);
                    } else {
                        len += file.length();
                    }
                }
            }

            return len;
        }
    }

    public static long getFileLength(String filePath) {
        boolean isURL = filePath.matches("[a-zA-z]+://[^\\s]*");
        if (isURL) {
            try {
                HttpsURLConnection conn = (HttpsURLConnection)(new URL(filePath)).openConnection();
                conn.setRequestProperty("Accept-Encoding", "identity");
                conn.connect();
                if (conn.getResponseCode() == 200) {
                    return (long)conn.getContentLength();
                }

                return -1L;
            } catch (IOException var3) {
                var3.printStackTrace();
            }
        }

        return getFileLength(getFileByPath(filePath));
    }

    private static long getFileLength(File file) {
        return !isFile(file) ? -1L : file.length();
    }

    public static String getFileMD5ToString(String filePath) {
        File file = TextUtils.isEmpty(filePath) ? null : new File(filePath);
        return getFileMD5ToString(file);
    }

    public static String getFileMD5ToString(File file) {
        return HConvertUtils.bytes2HexString(getFileMD5(file));
    }

    public static byte[] getFileMD5(String filePath) {
        return getFileMD5(getFileByPath(filePath));
    }

    public static byte[] getFileMD5(File file) {
        if (file == null) {
            return null;
        } else {
            DigestInputStream dis = null;

            try {
                FileInputStream fis = new FileInputStream(file);
                MessageDigest md = MessageDigest.getInstance("MD5");
                dis = new DigestInputStream(fis, md);
                byte[] buffer = new byte[262144];

                while(dis.read(buffer) > 0) {
                }

                md = dis.getMessageDigest();
                byte[] var5 = md.digest();
                return var5;
            } catch (IOException | NoSuchAlgorithmException var15) {
                var15.printStackTrace();
            } finally {
                try {
                    if (dis != null) {
                        dis.close();
                    }
                } catch (IOException var14) {
                    var14.printStackTrace();
                }

            }

            return null;
        }
    }

    public static String getDirName(File file) {
        return file == null ? "" : getDirName(file.getAbsolutePath());
    }

    public static String getDirName(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return "";
        } else {
            int lastSep = filePath.lastIndexOf(File.separator);
            return lastSep == -1 ? "" : filePath.substring(0, lastSep + 1);
        }
    }

    public static String getFileName(File file) {
        return file == null ? "" : getFileName(file.getAbsolutePath());
    }

    public static String getFileName(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return "";
        } else {
            int lastSep = filePath.lastIndexOf(File.separator);
            return lastSep == -1 ? filePath : filePath.substring(lastSep + 1);
        }
    }

    public static String getFileNameNoExtension(File file) {
        return file == null ? "" : getFileNameNoExtension(file.getPath());
    }

    public static String getFileNameNoExtension(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return "";
        } else {
            int lastPoi = filePath.lastIndexOf(46);
            int lastSep = filePath.lastIndexOf(File.separator);
            if (lastSep == -1) {
                return lastPoi == -1 ? filePath : filePath.substring(0, lastPoi);
            } else {
                return lastPoi != -1 && lastSep <= lastPoi ? filePath.substring(lastSep + 1, lastPoi) : filePath.substring(lastSep + 1);
            }
        }
    }

    public static String getFileExtension(File file) {
        return file == null ? "" : getFileExtension(file.getPath());
    }

    public static String getFileExtension(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return "";
        } else {
            int lastPoi = filePath.lastIndexOf(46);
            int lastSep = filePath.lastIndexOf(File.separator);
            return lastPoi != -1 && lastSep < lastPoi ? filePath.substring(lastPoi + 1) : "";
        }
    }

    public static void notifySystemToScan(String filePath) {
        notifySystemToScan(getFileByPath(filePath));
    }

    public static void notifySystemToScan(File file) {
        if (file != null && file.exists()) {
            Intent intent = new Intent("android.intent.action.MEDIA_SCANNER_SCAN_FILE");
            intent.setData(Uri.parse("file://" + file.getAbsolutePath()));
            HUtilsEx.getApp().sendBroadcast(intent);
        }
    }

    @SuppressLint({"ObsoleteSdkInt"})
    public static long getFsTotalSize(String anyPathInFs) {
        if (TextUtils.isEmpty(anyPathInFs)) {
            return 0L;
        } else {
            StatFs statFs = new StatFs(anyPathInFs);
            long blockSize;
            long totalSize;
            if (Build.VERSION.SDK_INT >= 18) {
                blockSize = statFs.getBlockSizeLong();
                totalSize = statFs.getBlockCountLong();
            } else {
                blockSize = (long)statFs.getBlockSize();
                totalSize = (long)statFs.getBlockCount();
            }

            return blockSize * totalSize;
        }
    }

    public static long getFsAvailableSize(String anyPathInFs) {
        if (TextUtils.isEmpty(anyPathInFs)) {
            return 0L;
        } else {
            StatFs statFs = new StatFs(anyPathInFs);
            long blockSize;
            long availableSize;
            if (Build.VERSION.SDK_INT >= 18) {
                blockSize = statFs.getBlockSizeLong();
                availableSize = statFs.getAvailableBlocksLong();
            } else {
                blockSize = (long)statFs.getBlockSize();
                availableSize = (long)statFs.getAvailableBlocks();
            }

            return blockSize * availableSize;
        }
    }

    public interface OnReplaceListener {
        boolean onReplace(File var1, File var2);
    }
}
