/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.hcn.bluetooth.api;

import android.media.session.PlaybackState;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.SystemClock;

/**
 * 对PlaybackState信息封装
 */
public final class MusicPlayState implements Parcelable {
    private static final String TAG = "MusicPlayState";

    public final static int STATE_NONE = PlaybackState.STATE_NONE;
    public final static int STATE_STOPPED = PlaybackState.STATE_STOPPED;
    public final static int STATE_PAUSED = PlaybackState.STATE_PAUSED;
    public final static int STATE_PLAYING = PlaybackState.STATE_PLAYING;
    public final static int STATE_FAST_FORWARDING = PlaybackState.STATE_FAST_FORWARDING;
    public final static int STATE_REWINDING = PlaybackState.STATE_REWINDING;
    public final static int STATE_BUFFERING = PlaybackState.STATE_BUFFERING;
    public final static int STATE_ERROR = PlaybackState.STATE_ERROR;
    public final static int STATE_CONNECTING = PlaybackState.STATE_CONNECTING;
    public final static int STATE_SKIPPING_TO_PREVIOUS = PlaybackState.STATE_SKIPPING_TO_PREVIOUS;
    public final static int STATE_SKIPPING_TO_NEXT = PlaybackState.STATE_SKIPPING_TO_NEXT;
    public final static int STATE_SKIPPING_TO_QUEUE_ITEM =
            PlaybackState.STATE_SKIPPING_TO_QUEUE_ITEM;

    private int mState;
    private long mDuration;
    private long mPosition;
    private long mBufferedPosition;
    private float mSpeed;
    private long mUpdateTime;

    public MusicPlayState() {
        mState = STATE_STOPPED;
        mPosition = 0;
        mDuration = 0;
        mSpeed = 1.0f;
        mUpdateTime = SystemClock.elapsedRealtime();
        mBufferedPosition = 0;
    }

    private MusicPlayState(Parcel in) {
        mState = in.readInt();
        mPosition = in.readLong();
        mDuration = in.readLong();
        mSpeed = in.readFloat();
        mUpdateTime = in.readLong();
        mBufferedPosition = in.readLong();
    }

    @Override
    public String toString() {
        StringBuilder bob = new StringBuilder("MusicPlayState {");
        bob.append("state=").append(mState);
        bob.append(", position=").append(mPosition);
        bob.append(", duration=").append(mDuration);
        bob.append(", buffered position=").append(mBufferedPosition);
        bob.append(", speed=").append(mSpeed);
        bob.append(", updated=").append(mUpdateTime);
        bob.append("}");
        return bob.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mState);
        dest.writeLong(mPosition);
        dest.writeLong(mDuration);
        dest.writeFloat(mSpeed);
        dest.writeLong(mUpdateTime);
        dest.writeLong(mBufferedPosition);
    }

    public void readFromParcel(Parcel in) {
        mState = in.readInt();
        mPosition = in.readLong();
        mDuration = in.readLong();
        mSpeed = in.readFloat();
        mUpdateTime = in.readLong();
        mBufferedPosition = in.readLong();
    }

    /**
     * Get the current state of playback. One of the following:
     * <ul>
     * <li> {@link MusicPlayState#STATE_NONE}</li>
     * <li> {@link MusicPlayState#STATE_STOPPED}</li>
     * <li> {@link MusicPlayState#STATE_PLAYING}</li>
     * <li> {@link MusicPlayState#STATE_PAUSED}</li>
     * <li> {@link MusicPlayState#STATE_FAST_FORWARDING}</li>
     * <li> {@link MusicPlayState#STATE_REWINDING}</li>
     * <li> {@link MusicPlayState#STATE_BUFFERING}</li>
     * <li> {@link MusicPlayState#STATE_ERROR}</li>
     * <li> {@link MusicPlayState#STATE_CONNECTING}</li>
     * <li> {@link MusicPlayState#STATE_SKIPPING_TO_PREVIOUS}</li>
     * <li> {@link MusicPlayState#STATE_SKIPPING_TO_NEXT}</li>
     * <li> {@link MusicPlayState#STATE_SKIPPING_TO_QUEUE_ITEM}</li>
     * </ul>
     */
    public int getState() {
        return mState;
    }

    public void setState(int state) {
        mState = state;
    }

    /**
     * Get the current playback position in ms.
     */
    public long getPosition() {
        return mPosition;
    }

    public void setPosition(long pos) {
        mPosition = pos;
    }

    public long getDuration() {
        return mDuration;
    }

    public void setDuration(long duration) {
        mDuration = duration;
    }

    /**
     * Get the current buffered position in ms. This is the farthest playback point that can be
     * reached from the current position using only buffered content.
     */
    public long getBufferedPosition() {
        return mBufferedPosition;
    }

    public void setBufferedPosition(long bufferedPos) {
        mBufferedPosition = bufferedPos;
    }

    /**
     * Get the current playback speed as a multiple of normal playback. This should be negative when
     * rewinding. A value of 1 means normal playback and 0 means paused.
     *
     * @return The current speed of playback.
     */
    public float getPlaybackSpeed() {
        return mSpeed;
    }

    public void setPlaybackSpeed(float speed) {
        mSpeed = speed;
    }

    public void reset() {
        mState = STATE_STOPPED;
        mPosition = 0;
        mDuration = 0;
        mSpeed = 1.0f;
        mUpdateTime = SystemClock.elapsedRealtime();
        mBufferedPosition = 0;
    }

    /**
     * Get the elapsed real time at which position was last updated. If the position has never been
     * set this will return 0;
     *
     * @return The last time the position was updated.
     */
    public long getLastPositionUpdateTime() {
        return mUpdateTime;
    }

    public void setLastPositionUpdateTime(long updateTime) {
        mUpdateTime = updateTime;
    }

    public static final Creator<MusicPlayState> CREATOR =
            new Creator<MusicPlayState>() {
                @Override
                public MusicPlayState createFromParcel(Parcel in) {
                    return new MusicPlayState(in);
                }

                @Override
                public MusicPlayState[] newArray(int size) {
                    return new MusicPlayState[size];
                }
            };
}
