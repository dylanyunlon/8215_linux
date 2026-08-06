package com.hcn.media_common.observer;

import io.reactivex.rxjava3.annotations.NonNull;
import io.reactivex.rxjava3.core.Observer;
import io.reactivex.rxjava3.disposables.Disposable;

/**
 * 观察者扩展接口类
 * @author 65821
 */
public abstract class ObserverEx<T> implements Observer<T> {
    Disposable disposable = null;

    @Override
    public void onSubscribe(@NonNull Disposable d) {
        disposable = d;
    }

    /**
     * 子类去实现
     * @param t 模板类参数
     */
    @Override
    public abstract void onNext(@NonNull T t);

    @Override
    public void onError(@NonNull Throwable e) {
        if (disposable != null) {
            disposable.dispose();
        }
    }

    @Override
    public void onComplete() {
        if (disposable != null) {
            disposable.dispose();
        }
    }
}
