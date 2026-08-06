package com.hcn.eq.listener;

public interface IEQModeListener {
    void onUser(int m60hz, int m230hz, int m910hz, int m3600hz, int m14khz);

    void onNews();

    void onJazz();

    void onCity();

    void onPop();

    void onElectronic();

    void onClassic();

    void onMovie();

    void onRock();

    void onTechno();

    void onDefault();
}
