package com.hcn.media_common;

import android.os.Build;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * 消息对象封装
 * <p> 自定义消息对象包，用来传递对象数据；
 *
 * @author 86158
 */
public final class HMessage implements Parcelable {
    public int what;
    public int arg0;
    public int arg1;
    public Object obj0;

    public HMessage() {
        this(-1);
    }

    public HMessage(int what) {
        this(what, -1, -1);
    }

    public HMessage(int what, int arg0, int arg1) {
        this(what, arg0, arg1, null);
    }

    public HMessage(int what, int arg0, int arg1, Object obj0) {
        this.what = what;
        this.arg0 = arg0;
        this.arg1 = arg1;
        this.obj0 = obj0;
    }

    /** If set message is in use.
     * This flag is set when the message is enqueued and remains set while it
     * is delivered and afterwards when it is recycled.  The flag is only cleared
     * when a new message is created or obtained since that is the only time that
     * applications are allowed to modify the contents of the message.
     *
     * It is an error to attempt to enqueue or recycle a message that is already in use.
    /*package*/ static final int FLAG_IN_USE = 1 << 0;

    /** If set message is asynchronous */
    /*package*/ static final int FLAG_ASYNCHRONOUS = 1 << 1;

    /** Flags to clear in the copyFrom method */
    /*package*/ static final int FLAGS_TO_CLEAR_ON_COPY_FROM = FLAG_IN_USE;

    /*package*/ int flags;

    public boolean isAsynchronous() {
        return (flags & FLAG_ASYNCHRONOUS) != 0;
    }

    public void setAsynchronous(boolean async) {
        if (async) {
            flags |= FLAG_ASYNCHRONOUS;
        } else {
            flags &= ~FLAG_ASYNCHRONOUS;
        }
    }

    /*package*/ boolean isInUse() {
        return ((flags & FLAG_IN_USE) == FLAG_IN_USE);
    }

    /*package*/ void markInUse() {
        flags |= FLAG_IN_USE;
    }

    // [缓存消息存储链表]
    /*package*/ HMessage next;

    /** @hide */
    // [消息缓存, 提高性能]
    public static final Object sPoolSync = new Object();
    private static HMessage sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 20;
    private static boolean gCheckRecycle = true;

    public static HMessage obtain() {
        synchronized (sPoolSync) {
            if (sPool != null) {
                HMessage m = sPool;
                sPool = m.next;
                m.next = null;
                m.flags = 0; // clear in-use flag
                sPoolSize--;
                return m;
            }
        }

        return new HMessage();
    }

    public static HMessage obtain(int what) {
        return obtain(what, -1, -1, null);
    }

    public static HMessage obtain(int what, Object obj0) {
        return obtain(what, -1, -1, obj0);
    }

    public static HMessage obtain(int what, int arg0, int arg1) {
        return obtain(what, arg0, arg1, null);
    }

    public static HMessage obtain(int what, int arg0, Object obj0) {
        return obtain(what, arg0, -1, obj0);
    }

    public static HMessage obtain(int what, int arg0, int arg1, Object obj0) {
        HMessage m = obtain();
        m.what = what;
        m.arg0 = arg0;
        m.arg1 = arg1;
        m.obj0 = obj0;

        return m;
    }

    public static HMessage obtain(HMessage orig) {
        HMessage m = obtain();
        m.what = orig.what;
        m.arg0 = orig.arg0;
        m.arg1 = orig.arg1;
        m.obj0 = orig.obj0;

        return m;
    }

    public HMessage setWhat(int what) {
        this.what = what;
        return this;
    }

    public void copyFrom(HMessage o) {
        this.what = o.what;
        this.arg0 = o.arg0;
        this.arg1 = o.arg1;
        this.obj0 = o.obj0;

        this.flags = o.flags & ~FLAGS_TO_CLEAR_ON_COPY_FROM;
    }

    /** @hide */
    public static void updateCheckRecycle(int targetSdkVersion) {
        if (targetSdkVersion < Build.VERSION_CODES.LOLLIPOP) {
            gCheckRecycle = false;
        }
    }

    // [消息回收, 可缓存消息对象]
    public void recycle() {
        if (isInUse()) {
            if (gCheckRecycle) {
                throw new IllegalStateException("This message cannot be recycled because it "
                        + "is still in use.");
            }
            return;
        }

        recycleUnchecked();
    }

    void recycleUnchecked() {
        flags = FLAG_IN_USE;
        what = 0;
        arg0 = 0;
        arg1 = 0;
        obj0 = null;

        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                next = sPool;
                sPool = this;
                sPoolSize++;
            }
        }
    }

    public static final Parcelable.Creator<HMessage> CREATOR
            = new Parcelable.Creator<HMessage>() {
        public HMessage createFromParcel(Parcel source) {
            HMessage msg = HMessage.obtain();
            msg.readFromParcel(source);
            return msg;
        }

        public HMessage[] newArray(int size) {
            return new HMessage[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(what);
        dest.writeInt(arg0);
        dest.writeInt(arg1);

        if (obj0 != null) {
            try {
                Parcelable p = (Parcelable)obj0;
                dest.writeInt(1);
                dest.writeParcelable(p, flags);
            } catch (ClassCastException e) {
                throw new RuntimeException(
                        "Can't marshal non-Parcelable objects across processes.");
            }
        } else {
            dest.writeInt(0);
        }
    }

    private void readFromParcel(Parcel source) {
        what = source.readInt();
        arg0 = source.readInt();
        arg1 = source.readInt();

        if (source.readInt() != 0) {
            obj0 = source.readParcelable(getClass().getClassLoader());
        }
    }
}
