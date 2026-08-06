package com.hcn.media.impl;

import android.content.Context;

import com.hcn.media.base.IMediaAgent;
import com.hcn.media.utils.UtilsEx;

import java.util.Objects;

/**
 * 实例化代理工具类
 * @author 65821
 */
public class Instrumentation {
    private static MediaAgent sAgent = null;

    /**
     * 构建媒体代理对象
     *
     * @param context 上下文参数
     * @return {@link IMediaAgent}
     */
    public static IMediaAgent buildAgent(Context context) {
        if (Objects.isNull(sAgent)) {
            Context ctx = context;
            if (context == null) {
                ctx = UtilsEx.getApplication();
            }

            sAgent = new MediaAgent(Objects.requireNonNull(ctx));
        }

        return sAgent;
    }
}
