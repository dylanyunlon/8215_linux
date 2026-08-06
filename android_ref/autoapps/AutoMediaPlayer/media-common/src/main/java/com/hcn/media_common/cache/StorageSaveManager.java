package com.hcn.media_common.cache;

import com.hcn.mediaservice.data.MusicInfo;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.List;

/**
 * @author 86158
 */
public class StorageSaveManager {

    private static final String STORAGE_PATH = "/.music";
    public static final String STORAGE_TAG_FILE = STORAGE_PATH + "/tag.txt";
    private static final String STORAGE_FAVORITE_FILE = STORAGE_PATH + "/favorite.txt";

    private String mStoragePath = "";

    public StorageSaveManager() {
        // TODO Auto-generated constructor stub
    }

    public void setStoragePath(String filePath) {
        mStoragePath = filePath;
    }

    public void onReadFavoriteInfoEvent(List<String> filePathList) {
        filePathList.clear();

        if (isExist(mStoragePath + STORAGE_FAVORITE_FILE)) {
            return;
        }

        String filePath = mStoragePath + STORAGE_FAVORITE_FILE;
        File file = new File(filePath);

        BufferedInputStream bis = null;
        BufferedReader reader = null;

        try (FileInputStream fis = new FileInputStream(file)) {
            bis = new BufferedInputStream(fis);
            bis.mark(4);
            byte[] first3bytes = new byte[3];
            int ignored = bis.read(first3bytes);
            bis.reset();
            if (first3bytes[0] == (byte) 0xEF && first3bytes[1] == (byte) 0xBB
                    && first3bytes[2] == (byte) 0xBF) {
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_8));
            } else if (first3bytes[0] == (byte) 0xFF && first3bytes[1] == (byte) 0xFE) {
                reader = new BufferedReader(new InputStreamReader(bis, "unicode"));
            } else if (first3bytes[0] == (byte) 0xFE && first3bytes[1] == (byte) 0xFF) {
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_16BE));
            } else if (first3bytes[0] == (byte) 0xFF && first3bytes[1] == (byte) 0xFF) {
                reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_16LE));
            } else {
                if (first3bytes[0] == (byte) 0x62 && first3bytes[1] == (byte) 0x61
                        && first3bytes[2] == (byte) 0x69) {
                    reader = new BufferedReader(new InputStreamReader(bis, StandardCharsets.UTF_8));
                } else {
                    reader = new BufferedReader(new InputStreamReader(bis, "GBK"));
                }
            }
            String line;
            try {
                while ((line = reader.readLine()) != null) {
                    filePathList.add(line);
                }
            } catch (IOException ignored1) {
            }

            System.out.println("success");
        } catch (IOException e) {
            // e.printStackTrace();
        }
    }

    public void onWriteFavoriteInfoEvent(List<MusicInfo> infoList) {
        checkoutStorage();

        try {
            String filePath = mStoragePath + STORAGE_FAVORITE_FILE;
            FileOutputStream fileOutputStream = new FileOutputStream(filePath);
            for (MusicInfo info : infoList) {
                byte[] buffer = info.mFilePath.getBytes();
                fileOutputStream.write(buffer, 0, buffer.length);
            }
            fileOutputStream.flush();
            fileOutputStream.close();
            System.out.println("success");
        } catch (IOException e) {
            // e.printStackTrace();
        }
    }

    private void checkoutStorage() {
        String filePath = mStoragePath + STORAGE_PATH;
        if (isExist(filePath)) {
            File file = new File(filePath);
            if (!file.exists()) {
                boolean ignored = file.mkdirs();
            }
        }
    }

    private boolean isExist(String filePath) {
        File file = new File(filePath);
        return !file.exists();
    }
}
