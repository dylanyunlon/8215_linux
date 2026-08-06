package com.hcn.media_cache.file;

import java.io.File;
import java.io.IOException;

/**
 * Declares how {@link FileCache} will use disc space.
 *
 * @author Alexey Danilov (danikula@gmail.com).
 */
public interface DiskUsage {

    /**
     * 创建文件或者更新文件最后的修改时间
     *
     * @param file
     * @throws IOException
     */
    void touch(File file) throws IOException;
}
