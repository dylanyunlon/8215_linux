package com.hcn.auto.concurrent.factory;

import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

public class NamedThreadFactory implements ThreadFactory {
    private final String namePrefix;
    private final AtomicInteger threadNumber = new AtomicInteger(1);
    private final SecurityManager securityManager = System.getSecurityManager();
    private final ThreadGroup group;

    public NamedThreadFactory(String namePrefix) {
        this.group = this.securityManager != null ? this.securityManager.getThreadGroup() : Thread.currentThread().getThreadGroup();
        this.namePrefix = namePrefix;
    }

    public Thread newThread(Runnable r) {
        Thread thread = new Thread(this.group, r, this.namePrefix + "-thread-" + this.threadNumber.getAndIncrement(), 0L);
        if (thread.isDaemon()) {
            thread.setDaemon(false);
        }

        if (thread.getPriority() != 5) {
            thread.setPriority(5);
        }

        return thread;
    }
}