package com.hcn.media.adapter.simple;

import static androidx.recyclerview.widget.RecyclerView.NO_POSITION;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.Drawable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.AutoMediaPlayer.R;
import com.hcn.common.misc.LogUtils;
import com.hcn.common.utils.HUtilsEx;
import com.hcn.media.R3;
import com.hcn.media.music.common.simple.ISimpleList;
import com.hcn.media_base.constant.IMusicState;
import com.hcn.media_common.cache.BitmapCache;
import com.hcn.media_common.thread.HPublicExecutor;
import com.hcn.media_common.thread.HTaskRunnable;
import com.hcn.media_common.utils.MediaID3Util;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media_data.debug.DebugUiData;
import com.hcn.media_theme.StyleX;
import com.hcn.mediaservice.data.MusicInfo;
import com.hcn.skinx.extend.SkinExBaseRvAdapter;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Callable;

/**
 * Simple List RecyclerView Adapter
 * <p> 当前播放承储列表、收藏列表、视频数据页可以使用该列表；
 *
 * @author 65821
 */
public class SimpleRvAdapter extends SkinExBaseRvAdapter {
    public static final String TAG = SimpleRvAdapter.class.getSimpleName();

    /**
     * 音乐信息列表
     * <p> 当前列表适配器的数据源；
     */
    private List<MusicInfo> mMusicList;

    /**
     * 当前正在播放的选项数据
     * <p> 记录指定 Item 的数据对象和其在列表中的位置信息；
     */
    private final PlayObject mCurrentPlayInfo = new PlayObject();

    /**
     * 列表相关事件监听
     * <p> e.g. 列表选项点击、长按、移除等等；
     */
    private final ISimpleRvListener mSimpleListener;

    /**
     * 当前关联的列表类型
     * @see ISimpleList
     */
    private @ISimpleList int mBindListType;

    /**
     * 列表布局风格约束
     * <@seee> {@link R.integer#simple_list_item_style_type}
     */
    private final int mListItemStyleType;

    public int getListItemStyleType() {
        return mListItemStyleType;
    }

    /**
     * 适配器构造函数
     *
     * @param context 当前上下文
     * @param list 数据列表
     * @param fileType 列表文件类型
     * @param listener 列表事件监听器
     */
    public SimpleRvAdapter(@NonNull Context context,
                           List<MusicInfo> list,
                           ISimpleRvListener listener,
                           @ISimpleList int listType,
                           @FileType int fileType) {
        super(context, fileType);

        mMusicList = list;
        mSimpleListener = listener;
        mBindListType = listType;

        // 读取 simple 列表选项风格类型
        mListItemStyleType = xInteger("simple_list_item_style_type");
        Log.d(TAG, "mListItemStyleType = " + mListItemStyleType);
    }

    /**
     * 是目标列表类型
     *
     * @param listType {@link ISimpleList }
     * @return 是/否
     */
    private boolean isListType(@ISimpleList int listType) {
        return mBindListType == listType;
    }

    /**
     * 设置当前列表类型
     *
     * @param listType {@link ISimpleList }
     */
    public void setListType(@ISimpleList int listType) {
        mBindListType = listType;
    }

    /**
     * 列表数据是空的
     * <p> 没有引用数据、或者数据源被移除；
     *
     * @return 是/否
     */
    @Override
    public boolean isEmpty() {
        return ((null == mMusicList) || mMusicList.isEmpty());
    }

    /**
     * 更新当前列表数据
     * <p> 注意这里是单纯的引用，无数据拷贝过程（拷贝耗时）；
     *
     * @param list 列表数据
     */
    public void updateDataList(@NonNull List<MusicInfo> list) {
        mMusicList = list;

        if (mSimpleListener != null) {
            mSimpleListener.onRvAdapterEvent("list-size", list.size(), null);
        }
    }

    /**
     * 获取当前 Item 视图类型
     * <pre>
     *    像音视频列表相对简单，一般只有一个视图类型；
     *    当前也可以按不同的专辑或者艺术家去分类分区间显示，这种情况就会存在多个视图类型；
     * </pre>
     *
     * @param position 在 RecyclerView 的位置索引
     * @return 返回值与使用时设置的值需保持一致
     */
    @Override
    public int getItemViewType(int position) {
        return IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE;
    }

    /**
     * 获取指定索引的 Item 数据
     *
     * @param position 在 RecyclerView 的位置索引
     * @return {@link MusicInfo}
     */
    private MusicInfo getItemInfo(int position) {
        // 无数据返回空
        if (Objects.isNull(mMusicList)) {
            return null;
        }

        // 检查索引是否合法
        if (position < 0 || position >= mMusicList.size()) {
            return null;
        }

        return mMusicList.get(position);
    }

    /**
     * 更新播放信息
     * <pre>
     *    接口对外开放；
     *    注意播放索引不一定是准确的，但是其更新效率高；
     * </pre>
     *
     * @param info 歌曲信息
     * @param notifyUpdate 通知更新显示效果
     */
    public void updatePlayInfo(MusicInfo info, boolean notifyUpdate) {
        // 数据不相等才需要更新
        if (mCurrentPlayInfo.compareTo(info)) {
            if (DebugUiData.MUSIC_DEBUG_V) {
                LogUtils.vTag(TAG, "updatePlayInfo: The playInfo is unchanged!");
            }

            // 校验当前播放信息是否在列表中
            if (mCurrentPlayInfo.position != NO_POSITION) {
                MusicInfo playInfo = getItemInfo(mCurrentPlayInfo.position);
                if (!mCurrentPlayInfo.compareTo(playInfo)) {
                    mCurrentPlayInfo.position = NO_POSITION;
                }
            }
            return;
        }

        mCurrentPlayInfo.info = info;
        mCurrentPlayInfo.position = NO_POSITION;

        // 更新显示信息，这个接口可以避免闪烁
        if (!isEmpty() && notifyUpdate) {
            notifyItemRangeChanged(0, getItemCount());
        }
    }

    /**
     * 更新播放索引
     * <pre>
     *    接口不对外开放；
     *    注意播放索引不一定是准确的，但是其更新效率高；
     * </pre>
     *
     * @param info 歌曲信息
     * @param position 索引位置
     */
    @SuppressLint("NotifyDataSetChanged")
    private void updatePlayIndex(MusicInfo info, int position) {
        // 安全性检查
        if (position < 0
                || position >= getItemCount()) {
            return;
        }

        // 数据不相等才需要更新
        if (mCurrentPlayInfo.compareTo(info, position)) {
            return;
        }

        mCurrentPlayInfo.info = info;
        mCurrentPlayInfo.position = position;

        // 更新显示信息，这个接口可以避免闪烁
        notifyItemRangeChanged(0, getItemCount());
    }

    @Override
    public int getLayoutRes(int itemViewType) {
        if (itemViewType == IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE) {
            return R.layout.layout_simple_list_item;
        }

        return super.getLayoutRes(itemViewType);
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        RecyclerView.ViewHolder viewHolder = null;

        switch (viewType) {
            case IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE:
                View convertView = inflateItemView(viewType, parent, false);
                viewHolder = new SimpleViewHolder(this, convertView);
                break;
            case IRvDecoration.TEST_RV_ITEM_LIST_TYPE:
            default:
                break;
        }

        assert viewHolder != null;
        return viewHolder;
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        super.onBindViewHolder(holder, position);

        int viewType = getItemViewType(position);
        switch (viewType) {
            case IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE:
                onBindSimpleViewHolder((SimpleViewHolder) holder, position);
                break;
            case IRvDecoration.TEST_RV_ITEM_LIST_TYPE:
            default:
                break;
        }
    }

    /**
     * SimpleList 选项视图持有者与列表绑定
     * <pre>
     *    不同风格装饰的 ItemView 分开绑定处理;
     *    当然我们这里就一种风格装饰，但是架子得搭建好，方便后续扩展；
     * </pre>
     *
     * @param holder 列表 Item 持有者
     * @param position 在 RecyclerView 列表中的位置
     */
    public void onBindSimpleViewHolder(@NonNull SimpleViewHolder holder, int position) {
        holder.setTag(position);
        MusicInfo info = getItemInfo(position);

        // 更新显示 Title 信息
        holder.setSimpleTitle(info != null? info.mFileName: "null");

        switch (mListItemStyleType) {
            case StyleX.ListItemType.SimpleType01:
                String unknown = xString(R3.string.text_unknown);

                if (info != null) {
                    // 检查是否已经解析 ID3 信息
                    if (info.mID3Type == MusicInfo.ID3_TYPE_NONE) {
                        holder.setSimpleArtist(unknown);

                        // 需要解析 ID3 信息
                        switch (mBindListType) {
                            case ISimpleList.PAGE_FAVORITE:
                                // 需要重新解析 ID3 信息
                                requestAnalysisID3Task(info, holder, position);
                                break;
                            case ISimpleList.PAGE_PLAYLIST:
                            case ISimpleList.PAGE_NONE:
                            default:
                                break;
                        }
                    } else {
                        holder.setSimpleArtist(
                                TextUtils.isEmpty(info.mArtist) ? unknown : info.mArtist);
                    }
                } else {
                    holder.setSimpleArtist("null");
                }

                holder.setSimpleNum(MiscUtils.formatNum(position, mMusicList.size()));
                holder.setSimpleIcon(
                        info != null ? info.mFilePath : "null",
                        xDrawableId2(R.drawable.simple_icon_music_n));
                break;
            case StyleX.ListItemType.None:
            default:
                break;
        }

        // 更新当前播放选择高亮状态
        if (mCurrentPlayInfo.position != NO_POSITION) {
            holder.setSelected(position == mCurrentPlayInfo.position);
        } else {
            holder.setSelected(mCurrentPlayInfo.compareTo(info));
        }
    }

    /**
     * 请求解析 ID3 任务
     * <pre>
     *    因为数据库比较旧的缘故，所以没有存储收藏信息的 ID3;
     *    这里只是对列表 ID3 更新的一个补充，本身它是有性能消耗的；
     *    后续更新数据库表以后，这个函数的执行频率就会大大降低；
     * </pre>
     *
     * @param info 媒体信息
     * @param holder 列表元素的 holder 对象
     * @param position 元素在列表中的位置
     */
    private void requestAnalysisID3Task(MusicInfo info,
                                        SimpleViewHolder holder,
                                        int position) {
        if (DebugUiData.MUSIC_DEBUG_I) {
            LogUtils.iTag(TAG, "requestAnalysisID3Task: " + position);
        }

        // 注意：这个接口的回调结果事件运行在主线程
        HPublicExecutor.instance().submitTask(
                new AnalysisID3Task(info),
                new HTaskRunnable.OnCompletionListener() {
                    // 记忆状态用来匹配（holder）
                    final int posTag = position;
                    final Reference<SimpleViewHolder>
                            holderRef = new WeakReference<>(holder);

                    @Override
                    public void onCompletion(Object o) {
                        // 视图有效性检查
                        SimpleViewHolder viewHolder = holderRef.get();
                        if (Objects.isNull(viewHolder)) {
                            LogUtils.iTag(TAG, "AnalysisID3Task: return 01");
                           return;
                        }

                        // 关联参数类型检查
                        Object tag = viewHolder.getTag();
                        if (!(tag instanceof Integer)
                                || !(o instanceof MusicInfo)) {
                            LogUtils.iTag(TAG, "AnalysisID3Task: return 02");
                            return;
                        }

                        // 还是之前的视图吗
                        int pos = (int) tag;
                        if (pos != posTag) {
                            LogUtils.iTag(TAG, "AnalysisID3Task: return 03");
                            return;
                        }

                        MusicInfo source = (MusicInfo) o;
                        MusicInfo target = getItemInfo(pos);
                        if (Objects.isNull(info) ||
                                !HUtilsEx.reverseEquals(
                                        target.mFileName, source.mFileName)) {
                            LogUtils.iTag(TAG, "AnalysisID3Task: return 04");
                            return;
                        }

                        // 同步 ID3 信息到内存列表对象
                        target.mID3Type = source.mID3Type;
                        target.mTitle = source.mTitle;
                        target.mArtist = source.mArtist;
                        target.mAlbum = source.mAlbum;
                        target.mTotalTime = source.mTotalTime;

                        // 更新显示信息到 UI 列表 Item
                        viewHolder.setSimpleArtist(source.mArtist);
                    }
                });
    }

    /**
     * 解析 ID3 任务
     * <pre>
     *    解析完成后需要通知列表更新；
     *    注意多线程操作，不用共用列表的数据；
     * </pre>
     */
    private static final class AnalysisID3Task implements Callable<MusicInfo> {
        private final MusicInfo mTargetInfo = new MusicInfo();

        public AnalysisID3Task(MusicInfo info) {
            // 不要直接引用对象（涉及多线程操作）
            mTargetInfo.mFilePath = info.mFilePath;
            mTargetInfo.mFileName = info.mFileName;
        }

        @Override
        public MusicInfo call() throws Exception {
            MediaID3Util.retrieveTargetID3Info(mTargetInfo);
            return mTargetInfo;
        }
    }

    @Override
    public int getItemCount() {
        if (Objects.isNull(mMusicList)) {
            return 0;
        }
        return mMusicList.size();
    }

    /**
     * 播放对象类
     * <p> 用来记录当前正在播放的数据选项；
     */
    private static final class PlayObject {
        public int position = NO_POSITION;
        public MusicInfo info;


        /**
         * 比较信息是否相等
         *
         * @param info 播放信息
         * @return 相等/不相等
         */
        public boolean compareTo(MusicInfo info) {
            if (Objects.isNull(info)) {
                return false;
            }

            return info.compareTo(this.info) == 0;
        }

        /**
         * 比较信息是否相等
         *
         * @param info 播放信息
         * @param position 位置信息
         * @return 相等/不相等
         */
        public boolean compareTo(MusicInfo info, int position) {
            if (Objects.isNull(info)) {
                return false;
            }

            return this.position == position
                    && info.compareTo(this.info) == 0;
        }
    }

    /**
     * Simple List Item ViewHolder
     */
    @SuppressLint("NonConstantResourceId")
    protected static class SimpleViewHolder extends ViewHolderEx {
        protected Reference<SimpleRvAdapter> mOwnerRef;

        protected TextView tvSimpleNum;
        protected ImageView ivSimpleIcon;
        protected TextView tvSimpleTitle;
        protected TextView tvSimpleArtist;
        protected ImageView ivPlayingIcon;
        protected AnimationDrawable animator;

        SimpleViewHolder(SimpleRvAdapter adapter, View itemView) {
            super(itemView, NO_POSITION);
            mOwnerRef = new WeakReference<>(adapter);

            // 获取 Item 中的内容对象
            int type = IRvDecoration.SIMPLE_RV_ITEM_LIST_TYPE;
            itemView.setOnClickListener(v -> {
                SimpleRvAdapter adp = mOwnerRef.get();
                if (Objects.isNull(adp)) {
                    return;
                }

                // 回调点击 Item 事件给监听者
                if (adp.mSimpleListener != null) {
                    Object tag =  getTag();
                    if (!(tag instanceof Integer)) {
                        return;
                    }

                    int position = (int) tag;
                    MusicInfo info = adp.getItemInfo(position);
                    adp.updatePlayIndex(info, position);
                    adp.mSimpleListener.onItemClick(info, position);
                }
            });

            ivSimpleIcon = itemView.findViewById(
                    adapter.xId(type, "ivSimpleIcon"));
            tvSimpleTitle = itemView.findViewById(
                    adapter.xId(type, "tvSimpleTitle"));
            ivPlayingIcon = itemView.findViewById(
                    adapter.xId(type, "ivPlayingIcon"));
            if (!Objects.isNull(ivPlayingIcon)) {
                animator = (AnimationDrawable) ivPlayingIcon.getBackground();
            }
            switch (adapter.getListItemStyleType()) {
                case StyleX.ListItemType.SimpleType01:
                    tvSimpleNum = itemView.findViewById(
                            adapter.xId(type, "tvSimpleNum"));
                    tvSimpleArtist = itemView.findViewById(
                            adapter.xId(type, "tvSimpleArtist"));
                    break;
                case StyleX.ListItemType.None:
                default:
                    break;
            }
        }

        /**
         * 设置此视图的旋转状态
         * <pre>
         *    视图通常在 AdapterView（如 ListView 或 RecyclerView）的上下文中选择；
         *    所选视图将显示高亮状态的配置效果;
         * </pre>
         *
         * @param selected 如果必须选择视图，则为 true，否则为 false；
         * @see View#setFocusableInTouchMode(boolean)
         * @see View#setFocusable(int)
         * @attr ref android.R.styleable#View_focusable
         */
        public void setSelected(boolean selected) {
            itemView.setSelected(selected);

            if (ivSimpleIcon != null) {
                ivSimpleIcon.setSelected(selected);
            }

            if (tvSimpleTitle != null) {
                tvSimpleTitle.setSelected(selected);
            }
            updateIconStatus(selected);
        }

        /**
         * 根据播放状态更新动画展示
         */
        private void updateIconStatus(Boolean selected) {
            if (ivPlayingIcon != null) {
                if (selected) {
                    ivPlayingIcon.setVisibility(View.VISIBLE);
                    if (AppGlobalData.getInstance()
                            .isPlayState(IMusicState.E_PLAY_STATE_PLAY)) {
                        if (!Objects.isNull(animator) && !animator.isRunning()) {
                            animator.start();
                        }
                    } else if (!Objects.isNull(animator)) {
                        animator.stop();
                    }
                } else {
                    ivPlayingIcon.setVisibility(View.INVISIBLE);
                    if (!Objects.isNull(animator) && animator.isRunning()) {
                        animator.stop();
                    }
                }
            }
        }

        /**
         * 设置歌曲信息图标
         * <p> 可以是专辑封面的缩略图，也可以是特定的图标；
         *
         * @param drawable 歌曲图标
         */
        public void setSimpleIcon(Drawable drawable) {
            if (ivSimpleIcon != null) {
                ivSimpleIcon.setImageDrawable(drawable);
            }
        }

        /**
         * 设置歌曲信息图标
         * <p> 可以是专辑封面的缩略图，也可以是特定的图标；
         *
         * @param mFilePath 歌曲图标
         * @param defaultImageResId 默认资源
         */
        public void setSimpleIcon(String mFilePath, int defaultImageResId) {
            if (ivSimpleIcon != null) {
                ivSimpleIcon.setTag(mFilePath);
                BitmapCache.getInstance().loadNativeImage(mFilePath,
                        ivSimpleIcon, defaultImageResId, true);
            }
        }

        /**
         * 设置歌曲名称
         * @param title 歌曲名称
         */
        public void setSimpleTitle(String title) {
            if (tvSimpleTitle != null) {
                tvSimpleTitle.setText(title);
            }
        }

        /**
         * 设置歌手名字
         * @param name 歌手名字
         */
        public void setSimpleArtist(String name) {
            if (tvSimpleArtist != null) {
                tvSimpleArtist.setText(name);
            }
        }

        /**
         * 设置歌曲序号
         * @param num 歌曲序号
         */
        public void setSimpleNum(String num) {
            if (tvSimpleNum != null) {
                tvSimpleNum.setText(num);
            }
        }
    }
}
