package com.hcn.media.video;

import android.os.Bundle;
import android.view.View;
import android.widget.LinearLayout;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.media.extend.PageExtend;
import com.hcn.skinx.R3;
import com.hcn.skinx.base.ISkinExFragment;

import java.util.Objects;

/**
 * 视频信息页扩展
 * <pre>
 *    1、类名不可以修改，这个是插件的规则；
 *    2、主要用来对 VideoInfoFragment 页面操作扩展；
 *    3、最终由 VideoInfoFragment 实例化并调用；
 * </pre>
 *
 * @author 65821
 */
public class VideoInfoPageExtend extends PageExtend {

    private static final String TAG = VideoInfoPageExtend.class.getSimpleName();

    /** 视频播放菜单元素 **/
    interface BTN_ID {
        int PREV = 0;
        int PLAY_PAUSE = 1;
        int NEXT = 2;
        int REPEAT_MODE = 3;
        int LIST = 4;
        int EQ = 5;
        int SCALE_MODE = 6;
        int SIZE = 7;
    }

    /** 视频播放菜单元素 **/
    private final View[] mButt;

    /**
     * 视频页面扩展构造函数
     * @param fragment 页面所有者
     */
    public VideoInfoPageExtend(ISkinExFragment fragment) {
        super(fragment);
        mButt = new View[BTN_ID.SIZE];
    }

    @Override
    public void onCreate(@Nullable Bundle bundle) {
        // TODO: 扩展预留
        ISkinExFragment f = mFragmentRef.get();
        f.requestExecuteMethod("test-case");
    }

    /**
     * 页面视图创建完成
     * <pre>
     *    所有子视图都在此初始化；
     *    由 Fragment#onViewCreated 触发调用；
     * </pre>
     *
     * @param view 页面视图
     * @param bundle 实例存储状态
     */
    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle bundle) {
        // 初始化 View 元素
        mButt[BTN_ID.PREV] = view.findViewById(R3.id.btnPrev);
        mButt[BTN_ID.PLAY_PAUSE] = view.findViewById(R3.id.btnPlay);
        mButt[BTN_ID.NEXT] = view.findViewById(R3.id.btnNext);
        mButt[BTN_ID.REPEAT_MODE] = view.findViewById(R3.id.btnRepeatMode);
        mButt[BTN_ID.LIST] = view.findViewById(R3.id.btnList);
        mButt[BTN_ID.EQ] = view.findViewById(R3.id.btnEQ);
        mButt[BTN_ID.SCALE_MODE] = view.findViewById(R3.id.btnVideoScale);
    }

    /**
     * 扩展类外部调用入口
     *
     * @param method 方法类型
     * @param args 参数集
     * @return 方法调用结果
     * @throws IllegalArgumentException 参数异常
     */
    @Override
    public String tryCallMethod(String method, Object... args) throws IllegalArgumentException {
        switch (method) {
            case "adjustBottomMenuLayout":
                return adjustBottomMenuLayout(args);
            case "none":
            default:
                break;
        }

        return super.tryCallMethod(method, args);
    }

    @Override
    public void onDestroy() {
        // TODO: 扩展预留
    }

    /**
     * 调整底部菜单布局
     * <p> 视频窗口在 "freeform" 模式时菜单显示不一样；
     *
     * @param args 参数集
     * @return 当前函数执行结果
     */
    private String adjustBottomMenuLayout(Object... args) {
        // 参数个数检查
        if (Objects.isNull(args)
                || args.length != 1) {
            throw new IllegalArgumentException("adjustBottomMenuLayout.");
        }

        // 参数类型检查
        boolean freeformMode;
        if (args[0] instanceof Boolean) {
            freeformMode = (boolean) args[0];
        } else {
            throw new IllegalArgumentException("adjustBottomMenuLayout.");
        }

        // 页面有效性检查
        ISkinExFragment f = mFragmentRef.get();
        if (Objects.isNull(f)) {
            return "false";
        }

        // EQ 菜单显示按钮（小窗口不显示）
        if (mButt[BTN_ID.EQ] != null) {
            mButt[BTN_ID.EQ].setVisibility(freeformMode ? View.GONE : View.VISIBLE);
        }

        // 列表菜单显示按钮（小窗口不显示）
        if (mButt[BTN_ID.LIST] != null) {
            mButt[BTN_ID.LIST].setVisibility(freeformMode ? View.GONE : View.VISIBLE);
        }

        // 视频底部播放菜单布局
        LinearLayout bottomMenuLayout = (LinearLayout) f.xFindViewByName("bottom_menu_layout");
        if (!Objects.isNull(bottomMenuLayout)) {
            if (mButt[BTN_ID.SCALE_MODE] != null) {
                bottomMenuLayout.removeView(mButt[BTN_ID.SCALE_MODE]);
                if (freeformMode) {
                    if (mButt[BTN_ID.NEXT] != null) {
                        int index = bottomMenuLayout.indexOfChild(mButt[BTN_ID.NEXT]);
                        bottomMenuLayout.addView(mButt[BTN_ID.SCALE_MODE], index + 1);
                    }
                } else {
                    if (mButt[BTN_ID.REPEAT_MODE] != null) {
                        int index = bottomMenuLayout.indexOfChild(mButt[BTN_ID.REPEAT_MODE]);
                        bottomMenuLayout.addView(mButt[BTN_ID.SCALE_MODE], index + 1);
                    }
                }
            }
        }

        return "true";
    }
}
