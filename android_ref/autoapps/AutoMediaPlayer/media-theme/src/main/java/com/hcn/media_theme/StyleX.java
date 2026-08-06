package com.hcn.media_theme;

/**
 * 布局风格差异配置
 * <pre>
 *    用来区分主题相关的布局文件风格差异，旨在替换 {@link ThemeEx} 类；
 *    针对不同客户对布局元素的要求和风格可能不同，为了代码执行效率和兼容性而设计；
 *    e.g. 不同客户对列表中的元素可能存在差异，有的要专辑封面、有的不要等；
 * </pre>
 *
 * @author 65821
 */
public interface StyleX {

    /**
     * 列表元素风格类型
     * <pre>
     *    有些客户要列表序号、有些不要；
     *    有些客户要歌曲封面、有些不要；
     *    ...
     * </pre>
     */
    interface ListItemType {
        /** 默认类型 */
        int None = 0;

        /**
         * 带元素序号、歌手名字、和专辑封面的风格
         * <p> 由 mcc400-mnc109 最早导入，所以赋值 109，方便阅读；
         */
        int SimpleType01 = 109;

        /**
         * 带元素序号、背景按压效果的风格
         * <p> 由 mcc400-mnc109 最早导入，所以赋值 109，方便阅读；
         */
        int MusicSongType01 = 109;

        /**
         * 带元素序号、歌手名字、背景按压效果的风格
         * <p> 由 mcc400-mnc109 最早导入，所以赋值 109，方便阅读；
         */
        int MusicFolderType01 = 109;

        /**
         * 带元素序号、背景按压效果的风格(视频)
         * <p> 由 mcc400-mnc109 最早导入，所以赋值 109，方便阅读；
         */
        int VideoListType01 = 109;
    }
}
