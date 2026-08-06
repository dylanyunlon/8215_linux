#ifndef CLUSTERAPPSOUNDPLAYER_H
#define CLUSTERAPPSOUNDPLAYER_H

#include <QMutex>
#include "clusterthreadproc.h"
class ClusterAppSoundPlayer
{
public:
    enum E_RingPlayState {
        E_SOUND_IDLE = 0,
        E_SOUND_PLAYING,
        E_SOUND_STOPING,
    };

    ClusterAppSoundPlayer(const QString &soundFile);
    ~ClusterAppSoundPlayer();

    bool play();
    bool stop();
    static void setVolume(int value);

protected:
    void run();

private:
    void init(const QString& soundFile);
    void setPlayState(int playState);
    int getPlayState();
    int alsaPcmInit();
    static int alsaMixerKcontrol(const char *name, int cmd, int *value);
    static int alsaMixerKcontrolSet(const char *name, int value);

private:
    struct WAV_HEADER
    {
        char rld[4];
        int rLen;
        char wld[4];
        char fld[4];
        int fLen;
        short wFormatTag;
        short wChannels;
        int nSamplesPersec;
        int nAvgBitsPerSample;
        short wBlockAlign;
        short wBitsPerSample;
        char dld[4];
        int wSampleLength;
    } m_header;

    int m_playState;
    QMutex m_mutex;
    char *m_data;
    ThreadProc *m_playThread = nullptr;
};

#endif // CLUSTERAPPSOUNDPLAYER_H
