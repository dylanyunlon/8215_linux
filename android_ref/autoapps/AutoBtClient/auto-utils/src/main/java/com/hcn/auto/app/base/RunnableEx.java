package com.hcn.auto.app.base;

public abstract class RunnableEx implements Runnable {
    private Object mObject;

    public RunnableEx() {
    }

    public Object getObject() {
        return this.mObject;
    }

    public void setObject(Object object) {
        this.mObject = object;
    }

    public void run() {
    }

    public void callback(Object obj) {
    }
}