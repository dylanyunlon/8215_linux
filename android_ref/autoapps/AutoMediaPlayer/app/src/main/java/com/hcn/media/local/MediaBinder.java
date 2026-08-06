package com.hcn.media.local;

import android.os.RemoteException;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.hcn.media_common.debug.LogUtil;
import com.hcn.media_base.IMediaEvent;
import com.hcn.media_base.constant.IConstant;
import com.hcn.media_data.AppGlobalData;
import com.hcn.media.IMediaPlayerService;
import com.hcn.mediaservice.data.MusicInfo;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.List;
import java.util.Objects;

/**
 * 本地服务 Binder 类
 * <pre>
 *    现阶段主要是本地应用程序 Bind 使用；
 *    其他特定需求要扩展，可以使用它作为基类；
 * </pre>
 *
 * @author 65821
 */
public class MediaBinder extends IMediaPlayerService.Stub {
    private static final String TAG = MediaBinder.class.getSimpleName();
    private Reference<LocalService> mOwnerRef;

    public MediaBinder(LocalService service) {
        mOwnerRef = new WeakReference<>(service);
    }

    @Override
    public boolean isRemoteConnected() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isRemoteConnected();
    }

    @Override
    public void doShouldPlayEvent() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.doShouldPlayEvent();
    }

    @Override
    public boolean getSDState() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isMounted(IConstant.PATH_SD);
    }

    @Override
    public boolean getSD2State() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isMounted(IConstant.PATH_SD2);
    }

    @Override
    public boolean getUSBState() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isMounted(IConstant.PATH_USB);
    }

    @Override
    public boolean targetStorageMounted(String path) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.targetStorageMounted(path);
    }

    @Override
    public boolean isCanWatchVideo() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isCanWatchVideo();
    }

    @Override
    public boolean isCanPlayVideo() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.isCanPlayVideo();
    }

    @Override
    public boolean existsHighPriorityEvent() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.existsHighPriorityEvent();
    }

    @Override
    public void requestSwitchMediaType() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.trySwitchMediaTypeTask();
    }

    @Override
    public void onRequestAudioFocus() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.onRequestAudioFocus();
    }

    @Override
    public void requestSeekToTime(int nTime) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.trySeekToTime(nTime);
    }

    @Override
    public void requestPlayDataSource(MusicInfo info) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.requestPlayDataSource(info);
    }

    @Override
    public void requestPlayMusiclist(int listType,
                                     List<MusicInfo> list,
                                     int position)
            throws RemoteException {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        // 这里的做法有些粗暴，待改善
        if (AppGlobalData.getInstance().mIsMediaPlayerLocked) {
            // [BUG] 如果还在锁定状态, 有概率出现 ANR 问题。
            // 锁定状态: 简单的说就是 MediaPlayer 还在 prepareAsync 中。
            LogUtil.d(TAG, " -- onChangeMusicList: mIsMediaPlayerLocked!");
            return;
        }

        service.tryUpdateMusicPlaylist(listType, position, list, true);
    }

    @Override
    public void requestPlayVideolist(int listType,
                                     List<MusicInfo> list,
                                     int position) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.tryUpdateVideoPlaylist(position, list, true);
    }

    @Override
    public boolean requestUpdateMusicPlaylist(int listType, List<MusicInfo> list) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.tryUpdateMusicPlaylistEx(listType, list);
    }

    @Override
    public void requestPlayControl(int nCommand) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.onPlayControl(nCommand);
    }

    @Override
    public void switchMusicRepeatMode() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.switchMusicRepeatMode();
    }

    @Override
    public void switchVideoRepeatMode() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.switchVideoRepeatMode();
    }

    @Override
    public void requestScanTargetPath(String filePath) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.requestScanStorageDevice(
                service.getStorageDevice(filePath));
    }

    @Override
    public int readMediaTime(int type, String path) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return 0;
        }

        return service.readMediaTime(type, path);
    }

    @Override
    public void writeMediaTime(int type, String path, int nTime, int reason) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.writeMediaTime(type, path, nTime, reason);
    }

    @Override
    public void dispatchMusicEvent(int eventId, String strParam, int nParam) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        // 处理播放文件不存在
        if (eventId == IMediaEvent.EVENT_ERROR_FILE_NOT_EXIST) {
            service.onMediaEvent(eventId, strParam, nParam);
            return;
        }

        service.onMediaEvent(eventId, null, null);
    }

    @Override
    public void doShouldPauseEvent(boolean stop, int reason) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.doShouldPauseEvent(stop, reason);
    }

    @Override
    public void requestExitApp(int reason) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.requestExitApplication(reason);
    }

    @Override
    public boolean inAccON() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return false;
        }

        return service.inAccON();
    }

    @Override
    public void registerMediaButton() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.registerMediaButtonEvent();
    }

    @Override
    public int readVideoScaleType() {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return 0;
        }

        return service.readVideoScaleType();
    }

    @Override
    public void writeVideoScaleType(int type) {
        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.writeVideoScaleType(type);
    }

    @Override
    protected void dump(@NonNull FileDescriptor fd,
                        @NonNull PrintWriter fout,
                        @Nullable String[] args) {
        super.dump(fd, fout, args);

        LocalService service = mOwnerRef.get();
        if (Objects.isNull(service)) {
            return;
        }

        service.dump(fd, fout, args);
    }
}

