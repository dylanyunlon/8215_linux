package com.hcn.media_common.debug;

/**
 * 调试配置广播
 * @author 86158
 */
public interface IBroadcast {

    /**
     * HMedia 配置广播: 用来通知进程应用程序配置改变
     * EXTRA_INFO = intent.getStringExtra(IBroadcast.HMEDIA_CONFIG_EXTRA_KEY);
     *
     * 例如, 关闭当前进程内存监控打印:
     *     setprop persist.sys.mm.mem.m.cfg 0
     *     am broadcast -a com.hcn.media.action.Config --es Config.extra.key mem.monitor
     */
    String HMEDIA_CONFIG_ACTION = "com.hcn.media.action.Config";
    String HMEDIA_CONFIG_EXTRA_KEY = "Config.extra.key";
    String HMEDIA_CONFIG_EXTRA_INFO_MEM = "mem.monitor";
}
