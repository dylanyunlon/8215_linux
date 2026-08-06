#ifndef ICLUSTER_H
#define ICLUSTER_H
#include <string>

/**
 * @brief The IClusterCallBack class
 * @note The interface class used for callback notification to the upper app
 */
class IClusterCallBack
{
public:
    virtual ~IClusterCallBack() = default;

    /**
     * @brief onMusicStateChanged
     * @param status
     * @return none
     * @note When the IVI music playing status changes, this function is called back
     */
    virtual void onMusicStateChanged(int status) = 0;

    /**
     * @brief onMusicNameChanged
     * @param musicName
     * @return none
     * @note When the name of the song played by IVI changes, this function is called back
     */
    virtual void onMusicNameChanged(const std::string &musicName) = 0;

    /**
     * @brief onCallStatusChanged
     * @param status
     * @return none
     * @note When the IVI call status changes, this function is called back
     */
    virtual void onCallStatusChanged(int status) = 0;

    /**
     * @brief onCallNumberChanged
     * @param number
     * @return none
     * @note When the IVI call number changes, this function is called back
     */
    virtual void onCallNumberChanged(const std::string &number) = 0;

    /**
     * @brief onCallPersonChanged
     * @param number
     * @return none
     * @note When the name of the contact person in the IVI call changes, this function is called back
     */
    virtual void onCallPersonChanged(const std::string &number) = 0;

    /**
     * @brief onCallTimeChanged
     * @param time
     * @return none
     * @note When the IVI call time changes, this function is called back
     */
    virtual void onCallTimeChanged(const std::string &time) = 0;

    /**
     * @brief onCallPixmapChanged
     * @param data
     * @param length
     * @return none
     * @note When the contact picture of the IVI call changes, this function is called back
     */
    virtual void onCallPixmapChanged(const char unsigned *data, unsigned int length) = 0;

    /**
     * @brief onCurrentAlbumPixmapChanged
     * @param data
     * @param length
     * @return none
     * @note When the album picture of the IVI call changes, this function is called back
     */
    virtual void onCurrentAlbumPixmapChanged(const char unsigned *data, unsigned int length) = 0;

    /**
     * @brief onUpdateStateChanged
     * @param state
     * @return none
     * @note When the cluster upgrade status changes, this function is called back
     */
    virtual void onUpdateStateChanged(int state) = 0;

    /**
     * @brief onUpdateProgressChanged
     * @param progress
     * @return none
     * @note When the cluster upgrade progress changes, this function is called back
     */
    virtual void onUpdateProgressChanged(int progress) = 0;

    /**
     * @brief onDisconnect
     * @return none
     * @note When IVI disconnects from the cluster, this function is called back
     */
    virtual void onDisconnect() = 0;
};

/**
 * @brief The ICluster class
 * @note Interface classes provided to the upper app
 */
class ICluster
{
public:
    static ICluster *getInstance();
    virtual ~ICluster() = default;

    /**
     * @brief registerCallBack
     * @param callback
     * @return none
     * @note Register callback notification interface
     */
    virtual void registerCallBack(IClusterCallBack *callback) = 0;

    /**
     * @brief mediaPlayPause
     * @return none
     * @note Control IVI media playback pause
     */
    virtual void mediaPlayPause() = 0;

    /**
     * @brief mediaPre
     * @return none
     * @note Control IVI media to switch to the previous song
     */
    virtual void mediaPre() = 0;

    /**
     * @brief mediaNext
     * @return none
     * @note Control IVI media to switch to the next song
     */
    virtual void mediaNext() = 0;

    /**
     * @brief call
     * @return none
     * @note Control IVI to dial recent calls or connect incoming calls
     */
    virtual void call() = 0;

    /**
     * @brief handup
     * @return none
     * @note Control IVI to hang up
     */
    virtual void handup() = 0;

    /**
     * @brief startService
     * @return none
     * @note Start monitoring IVI communication service
     */
    virtual void startService() = 0;

    /**
     * @brief startUpdateService
     * @param lastVersion
     * @return lastState: recent upgrade status
     * @note Start the upgrade service
     */
    virtual int startUpdateService(std::string &lastVersion) = 0;

    /**
     * @brief startIVIProjection
     * @param x: display position x coordinate
     * @param y: display position y coordinate
     * @param w: display width
     * @param h: display height
     * @return none
     * @note Start IVI projection process
     */
    virtual void startIVIProjection(int x, int y, int w, int h) = 0;

    /**
     * @brief stopIVIProjection
     * @return none
     * @note Stop IVI projection process
     */
    virtual void stopIVIProjection() = 0;
};

#endif // ICLUSTER_H
