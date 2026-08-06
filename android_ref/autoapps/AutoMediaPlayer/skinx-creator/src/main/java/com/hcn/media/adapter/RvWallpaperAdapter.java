package com.hcn.media.adapter;

import static androidx.recyclerview.widget.RecyclerView.NO_POSITION;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.hcn.auto_compat.app.Wallpaper;
import com.hcn.common.utils.HImageUtils;
import com.hcn.media.adapter.base.IRvDecoration;
import com.hcn.media.adapter.base.IRvListener;
import com.hcn.media_common.utils.MiscUtils;
import com.hcn.skinx.R3;
import com.hcn.skinx.extend.SkinExBaseRvAdapter;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;

/**
 * Wallpaper List RecyclerView Adapter
 * <p> 用来显示并管理壁纸列表序列；
 *
 * @author 65821
 */
public class RvWallpaperAdapter extends SkinExBaseRvAdapter {

    /**
     * 壁纸信息列表
     * <p> 当前列表适配器的数据源；
     */
    private List<Wallpaper.Info> mWallpaperList;

    /**
     * 列表相关事件监听
     * <p> e.g. 列表选项点击、自定义事件等；
     */
    private final IRvListener mRvListener;

    /**
     * 适配器构造函数
     *
     * @param context 页面上下文
     * @param list 列表数据
     * @param listener 事件监听
     */
    public RvWallpaperAdapter(@NonNull Context context,
                              List<Wallpaper.Info> list,
                              @NonNull IRvListener listener) {
        super(context, FileType.NONE);

        mRvListener = listener;
        mWallpaperList = list;
    }

    /**
     * 更新当前列表数据
     * <p> 注意这里是单纯的引用，无数据拷贝过程（拷贝耗时）；
     *
     * @param list 列表数据
     */
    public void updateList(@NonNull List<Wallpaper.Info> list) {
        mWallpaperList = list;
    }

    @Override
    public int getLayoutRes(int itemViewType) {
        if (IRvDecoration.RV_WALLPAPER_ITEM_TYPE == itemViewType) {
            return R3.layout.layout_wallpaper_rv_item;
        }

        return super.getLayoutRes(itemViewType);
    }

    @Override
    public String getLayoutResName(int itemViewType) {
        if (IRvDecoration.RV_WALLPAPER_ITEM_TYPE == itemViewType) {
            return "layout_wallpaper_rv_item";
        }

        return super.getLayoutResName(itemViewType);
    }

    @Override
    public int getItemViewType(int position) {
        return IRvDecoration.RV_WALLPAPER_ITEM_TYPE;
    }

    /**
     * 获取指定索引的 Item 数据
     *
     * @param position 在 RecyclerView 的位置索引
     * @return {@link Wallpaper.Info}
     */
    private Wallpaper.Info getItemInfo(int position) {
        // 无数据返回空
        if (Objects.isNull(mWallpaperList)) {
            return null;
        }

        // 检查索引是否合法
        if (position < 0 || position >= mWallpaperList.size()) {
            return null;
        }

        return mWallpaperList.get(position);
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        RecyclerView.ViewHolder viewHolder = null;

        switch (viewType) {
            case IRvDecoration.RV_WALLPAPER_ITEM_TYPE:
                View convertView = inflateItemView4Name(viewType, parent, false);
                viewHolder = new WallpaperViewHolder(this, convertView);
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
            case IRvDecoration.RV_WALLPAPER_ITEM_TYPE:
                onBindWallpaperViewHolder((WallpaperViewHolder) holder, position);
                break;
            case IRvDecoration.TEST_RV_ITEM_LIST_TYPE:
            default:
                break;
        }
    }

    /**
     * RecyclerView 选项视图持有者与列表绑定
     * <pre>
     *    不同风格装饰的 ItemView 分开绑定处理;
     *    当然我们这里就一种风格装饰，但是架子得搭建好，方便后续扩展；
     * </pre>
     *
     * @param holder 列表 Item 持有者
     * @param position 在 RecyclerView 列表中的位置
     */
    public void onBindWallpaperViewHolder(@NonNull WallpaperViewHolder holder, int position) {
        holder.setTag(position);
        Wallpaper.Info info = getItemInfo(position);

        // 壁纸图片
        if (info != null) {
            String path = info.thumbnailPath;
            if (!TextUtils.isEmpty(path)) {
                holder.updateWallpaperItem(path);
            }
        }
    }

    @Override
    public boolean isEmpty() {
        return ((null == mWallpaperList) || mWallpaperList.isEmpty());
    }

    @Override
    public int getItemCount() {
        if (Objects.isNull(mWallpaperList)) {
            return 0;
        }

        return mWallpaperList.size();
    }

    @SuppressLint("NonConstantResourceId")
    protected static class WallpaperViewHolder extends ViewHolderEx {
        protected Reference<RvWallpaperAdapter> mOwnerRef;

        public View layoutWallpaperItem;

        public ImageView ivThumbnail;

        public WallpaperViewHolder(RvWallpaperAdapter adapter, View itemView) {
            super(itemView, NO_POSITION);
            mOwnerRef = new WeakReference<>(adapter);

            // 壁纸 Item 布局视图
            layoutWallpaperItem = itemView.findViewById(R3.id.wallpaper_rv_item_layout);
            if (layoutWallpaperItem != null) {
                layoutWallpaperItem.setOnClickListener(v -> {
                    RvWallpaperAdapter adp = mOwnerRef.get();
                    if (Objects.isNull(adp)) {
                        return;
                    }

                    Object tag =  getTag();
                    if (!(tag instanceof Integer)) {
                        return;
                    }

                    int position = (int) tag;
                    Wallpaper.Info info = adp.getItemInfo(position);
                    adp.mRvListener.onItemClick(info, position);
                });
            }

            // 壁纸 Item 内容视图（不接受点击事件）
            ivThumbnail = itemView.findViewById(R3.id.ivThumbnail);
            if (ivThumbnail != null) {
                ivThumbnail.setFocusable(false);
                ivThumbnail.setFocusableInTouchMode(false);
            }
        }

        /**
         * 更新选项显示
         * @param path 壁纸缩略图路径
         */
        public void updateWallpaperItem(@NonNull String path) {
            if (Objects.isNull(ivThumbnail)) {
                return;
            }

            Bitmap bmp = HImageUtils.getBitmap(path);
            if (!Objects.isNull(bmp)) {
                ivThumbnail.setImageBitmap(bmp);
            }
        }
    }
}
