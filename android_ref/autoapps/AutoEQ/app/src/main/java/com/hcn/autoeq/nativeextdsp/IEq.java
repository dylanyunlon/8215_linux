package com.hcn.autoeq.nativeextdsp;

public interface IEq {
    default int setEqBand(int[] _data) {
        return -1;
    }

    default int setEqBands(int[] _data) {
        return -1;
    }

    default int setEqBalance(int[] _data) {
        return -1;
    }

    default int setEqAttSpeaker(int[] _data) {
        return -1;
    }

    default int setEqHpfLpf(int[] _data) {
        return -1;
    }

    default int setEqDbb(int[] _data) {
        return -1;
    }

    default int setEqSurround(int[] _data) {
        return -1;
    }

    default int setEqLoudness(int[] _data) {
        return -1;
    }

    default int setEqSpeakerDelay(int[] _data) {
        return -1;
    }

    default int setEqBass(int[] _data) {
        return -1;
    }
}
