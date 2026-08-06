package com.hcn.media_dummy.utils;

import android.graphics.Bitmap;
import android.os.Environment;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.OutputStream;

/**
 * 简单的文件操作工具
 * @author 65821
 */
public class FileUtils {

    public static final String NAME = "FunVideo";
    public static final String NAME_TEST = "FunVideoTest";

    public static String getAppPath(String name) {
        StringBuilder sb;
        sb = new StringBuilder();
        sb.append(Environment.getExternalStoragePublicDirectory("Download").getAbsolutePath());
        sb.append(File.separator);
        sb.append(name);
        sb.append(File.separator);
        return sb.toString();
    }

    public static String getPath() {
        String path = getAppPath(NAME);
        File file = new File(path);
        if (!file.exists()) {
            boolean ignored = file.mkdirs();
        }
        return path;
    }

    public static String getTestPath() {
        String path = getAppPath(NAME_TEST);
        File file = new File(path);
        if (!file.exists()) {
            boolean ignored = file.mkdirs();
        }
        return path;
    }

    public static void deleteFiles(File root) {
        File[] files = root.listFiles();
        if (files != null) {
            for (File f : files) {
                if (!f.isDirectory() && f.exists()) {
                    try {
                        boolean ignored = f.delete();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    public static void saveBitmap(Bitmap bitmap, File file) {
        if (bitmap != null) {
            OutputStream outputStream;
            try {
                outputStream = new FileOutputStream(file);
                bitmap.compress(Bitmap.CompressFormat.JPEG, 100, outputStream);
                bitmap.recycle();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }
}
