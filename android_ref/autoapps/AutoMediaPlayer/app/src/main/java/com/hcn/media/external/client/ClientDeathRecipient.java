package com.hcn.media.external.client;

import android.os.IBinder;

import androidx.annotation.NonNull;

import com.hcn.common.lang.RunnableEx;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.util.Objects;

/**
 * 客户端死亡机制接收者
 * <p> 监听客户端是否挂了；
 * @author 65821
 */
public class ClientDeathRecipient implements IBinder.DeathRecipient {
    private final Reference<ClientInfo> clientRef;
    private final RunnableEx binderDiedRunnable;

    /**
     * 客户端死亡监听构造函数
     *
     * @param clientInfo 需要绑定的客户端信息
     * @param runnable 死亡回调
     */
    public ClientDeathRecipient(@NonNull ClientInfo clientInfo, @NonNull RunnableEx runnable) {
        clientRef = new WeakReference<>(clientInfo);
        binderDiedRunnable = runnable;
    }

    @Override
    public void binderDied() {
        ClientInfo clientInfo = clientRef.get();
        if (Objects.isNull(clientInfo)) {
            return;
        }

        // 取消死亡监听 link 状态
        IBinder binder = clientInfo.getBinder();
        IBinder.DeathRecipient dr = clientInfo.getDeathRecipient();
        if (binder != null && dr != null) {
            binder.unlinkToDeath(dr, 0);
        }

        // 回调通知客户端对象已销毁
        binderDiedRunnable.callback(clientInfo.getName());
    }
}
