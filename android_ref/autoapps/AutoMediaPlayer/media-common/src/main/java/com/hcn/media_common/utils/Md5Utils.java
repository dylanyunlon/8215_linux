package com.hcn.media_common.utils;

import java.math.BigInteger;
import java.security.MessageDigest;

/**
 * MD5 工具
 * <p> 计算给定字符串的 MD5 值；
 *
 * @author 86158
 */
public class Md5Utils {

    /**
     * 计算指定字符串的 MD5 值；
     *
     * @param plainText 输入的字符串对象
     * @return md5 值字符串表达形式；
     */
    public static String md5(String plainText) {
        byte[] secretBytes = null;
        try {
            secretBytes = MessageDigest.getInstance("md5").digest(plainText.getBytes());
        } catch (Exception e) {
            throw new RuntimeException("");
        }

        StringBuilder zero = new StringBuilder("0");
        String md5code = new BigInteger(1, secretBytes).toString(16);
        for (int i = 0; i < 32 - md5code.length(); i++) {
            md5code = zero.append(md5code).toString();
        }
        return md5code;
    }
}
