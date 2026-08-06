package com.hcn.media.vm.base;

import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import androidx.lifecycle.LifecycleOwner;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.Observer;

import java.util.ArrayList;
import java.util.List;

/**
 * ViewModel 命令观察封装
 * @author 65821
 */
public class VmCommand<T> {
    private static final String TAG = VmCommand.class.getSimpleName();

    // 可观察的实时命令数据包
    private final List<Action<T>> mListHAction = new ArrayList<>();
    private final MutableLiveData<List<Action<T>>> mDelegate = new MutableLiveData<>();

    /**
     * 命令数据包
     *
     * @param <E>
     */
    public interface Action<E> {
        /**
         * 执行函数
         * @param t 事件
         */
        void exec(E t);
    }

    /**
     * 事件执行者<由命令观察者实现>
     *
     * @param <R>
     */
    public interface ActionExecutor<R> {
        /**
         * 执行函数
         * @param action
         */
        void exec(Action<R> action);
    }

    /**
     * 触发事件
     *
     * @param action 事件包装
     */
    @MainThread
    public void execute(@NonNull Action<T> action) {
        mListHAction.add(action);

        // setValue 有生命周期的限制: [onResume，onPause]
        // 并且每次生命周期变化都会试着将值传递给目标监听者。
        // 在 onStart 之前调用它，只会有最后一次会触发回调。
        mDelegate.setValue(mListHAction);
        // 有效生命周期内指令的状态是瞬时的，因此在执行完成之后需要清除状态
        mDelegate.setValue(null);
    }

    /**
     * 添加事件处理观察者
     *
     * @param owner 生命周期所有者
     * @param executor 观察事件改变（观察者的对象）
     */
    public void observe(LifecycleOwner owner, @NonNull ActionExecutor<T> executor) {
        // 观察命令数据包改变
        mDelegate.observe(owner, new Observer<List<Action<T>>>() {
            /**
             * 把命令下发给观察者<带事件处理器>
             * @param actions 已改变的命令数据包
             */
            @Override
            public void onChanged(List<Action<T>> actions) {
                if (actions == null) {
                    return;
                }

                for (Action<T> action : actions) {
                    executor.exec(action);
                }

                if (!actions.isEmpty()) {
                    mListHAction.clear();
                }
            }
        });
    }
}