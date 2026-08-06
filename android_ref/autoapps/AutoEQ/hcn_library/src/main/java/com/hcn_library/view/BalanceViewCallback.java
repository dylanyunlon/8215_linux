package com.hcn_library.view;

public interface BalanceViewCallback {
    void onMotionBegin();

    void onMotionChanged(int x, int y, boolean bUpdate);

    void onMotionFinished(int x, int y, boolean bUpdate);

    void onSaveData(int x, int y);
}
