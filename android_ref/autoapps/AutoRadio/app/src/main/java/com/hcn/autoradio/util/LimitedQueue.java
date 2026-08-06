package com.hcn.autoradio.util;

import java.util.LinkedList;

/**
 * 限定长度的队列
 *
 * @param <E>
 */
public class LimitedQueue<E> extends LinkedList<E> {
    private int limit;

    public LimitedQueue(int limit) {
        this.limit = limit;
    }

    @Override
    public boolean add(E o) {
        super.add(o);
        while (size() > limit) {
            super.remove();
        }
        return true;
    }
}