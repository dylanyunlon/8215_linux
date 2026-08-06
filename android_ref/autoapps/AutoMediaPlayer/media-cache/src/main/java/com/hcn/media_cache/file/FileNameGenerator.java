package com.hcn.media_cache.file;

/**
 * Generator for files to be used for caching.
 *
 * @author Alexey Danilov (danikula@gmail.com).
 */
public interface FileNameGenerator {
    /**
     * 用来缓存文件的缓存文件名字生成器
     *
     * @param url
     * @return
     */
    String generate(String url);
}
