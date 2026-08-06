/**
 * 新增定制类
 * @Tag: HCN2022_T
 */
#ifndef A_THREAD_H_

#define A_THREAD_H_

#include <sys/epoll.h>
#include <sys/inotify.h>

#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>

#include <functional>
#include <algorithm>

namespace android {

class AThread : public Thread {
	// Hint for number of file descriptors to be associated with the epoll instance.
	static const int EPOLL_SIZE_HINT = 8;

	// Maximum number of file descriptors for which to retrieve poll events each iteration.
	static const int EPOLL_MAX_EVENTS = 16;

public:

	typedef std::function<void()> callback;

	AThread();

	int  postMessageDelayed(const sp<AMessage> &msg, int timeout = 0);
	int32_t postFunctionDelayed(const callback func, int32_t timeout = 0);
	bool hasMessage(const uint32_t what);
	int  removeMessage(const uint32_t what);
	int  removeMessage(void *msg);
	int  addFd(struct epoll_event *const event);

protected:
	virtual ~AThread();
	virtual bool onMessage(const sp<AMessage> &msg) = 0;
	virtual bool onPolling(struct epoll_event *eventItem) = 0;

private:

	class CallbackWrap : public RefBase {
		public:
		CallbackWrap(callback c) : cb(c) { }
		const callback cb;
	};

	bool         threadLoop();
	int32_t      insertMessage(const sp<AMessage> &msg);
	int          getEarliestTimeout();
	sp<AMessage> getEarliestMessage();
	List<sp<AMessage>> mMessageList;
	Mutex        mLock;

	int          mEpollfd;
	int          mWakeupPipe[2];
	bool         mRequestExit;

	int          mWritePipeCount;
	int          mReadPipeCount;
};


AThread::AThread()
{
	// create epoll.
	mEpollfd = epoll_create(EPOLL_SIZE_HINT);
	struct epoll_event eventItem;
	int result;

	pipe(mWakeupPipe);
	mRequestExit = false;
	mWritePipeCount = 0;
	mReadPipeCount = 0;

	memset(&eventItem, 0, sizeof(epoll_event));
	eventItem.events = EPOLLIN | EPOLLET;
	eventItem.data.fd = mWakeupPipe[0];
	result = epoll_ctl(mEpollfd, EPOLL_CTL_ADD, mWakeupPipe[0], &eventItem);
	if (result != 0) {
		ALOGW("epoll_ctl failed with an unexpected error, errno=%d", errno);
	}
}

AThread::~AThread()
{
	mRequestExit = true;
	mWritePipeCount++;
	write(mWakeupPipe[1], "A", 1);
	requestExitAndWait();

	close(mWakeupPipe[0]);
	close(mWakeupPipe[1]);
	close(mEpollfd);
}

int AThread::addFd(struct epoll_event *const event)
{
	int result;
	Mutex::Autolock autoLock(mLock);
	result = epoll_ctl(mEpollfd, EPOLL_CTL_ADD, event->data.fd, event);
	mWritePipeCount++;
	write(mWakeupPipe[1], "A", 1);
	return result;
}

bool AThread::hasMessage(const uint32_t what)
{
	Mutex::Autolock autoLock(mLock);

	for (List<sp<AMessage>>::iterator it = mMessageList.begin(); it != mMessageList.end(); ) {
		if (what == (*it)->what()) {
			return true;
		} else {
			it++;
		}
	}
	return false;
}

int AThread::removeMessage(const uint32_t what)
{
	int  erased = 0;
	Mutex::Autolock autoLock(mLock);

	for (List<sp<AMessage>>::iterator it = mMessageList.begin(); it != mMessageList.end(); ) {
		if (what == (*it)->what()) {
			List<sp<AMessage>>::iterator tx = mMessageList.erase(it);
			it = tx;
			erased++;
		} else {
			it++;
		}
	}
	return erased;
}

int AThread::removeMessage(void *msg)
{
	int  erased = 0;
	Mutex::Autolock autoLock(mLock);

	for (List<sp<AMessage>>::iterator it = mMessageList.begin(); it != mMessageList.end(); ) {
		if (msg == (*it).get()) {
			List<sp<AMessage>>::iterator tx = mMessageList.erase(it);
			it = tx;
			erased++;
		} else {
			it++;
		}
	}
	return erased;
}

int32_t AThread::postFunctionDelayed(const callback func, int32_t timeout)
{
	const int32_t now = systemTime() / 1000000L;  //in ms

	if (func == nullptr) {
		return -1;
	}

	timeout += now;
	sp<AMessage> msg = new AMessage(0, 0);
	sp<CallbackWrap> cb = new CallbackWrap(func);

	msg->setInt32("timeout", timeout);
	msg->setObject("callback", cb);

	return insertMessage(msg);
}

int AThread::postMessageDelayed(const sp<AMessage> &msg, int timeout)
{
	const int32_t now   = systemTime() / 1000000L;  //in ms

	timeout += now;
	msg->setInt32("timeout", timeout);

	return insertMessage(msg);
}

int32_t AThread::insertMessage(const sp<AMessage> &msg)
{
	bool inserted = false;
	int32_t timeout = 0;
	msg->findInt32("timeout", &timeout);

	Mutex::Autolock autoLock(mLock);

	for (List<sp<AMessage>>::iterator it = mMessageList.begin(); it != mMessageList.end(); ++it) {
		sp<AMessage> obj = *it;
		int32_t tmout  = -1;
		obj->findInt32("timeout", &tmout);
		if (tmout > timeout) {
			mMessageList.insert(it, msg);
			inserted = true;
			break;
		}
	}

	if (!inserted) {
		mMessageList.push_back(msg);
	}

	mWritePipeCount++;
	write(mWakeupPipe[1], "A", 1);

	return 0;
}

int AThread::getEarliestTimeout()
{
	int32_t now     = systemTime() / 1000000L;  //in ms
	int32_t timeout = -1;

	Mutex::Autolock autoLock(mLock);
	List<sp<AMessage>>::iterator it;
	if (!mMessageList.empty()) {
		it = mMessageList.begin();
		(*it)->findInt32("timeout", &timeout);
		timeout -= now;
		timeout = std::max(timeout, 1);
	}
	return timeout;
}

sp<AMessage> AThread::getEarliestMessage()
{
	Mutex::Autolock autoLock(mLock);

	if (!mMessageList.empty()) {
		sp<AMessage> msg = *mMessageList.begin();
		int32_t timeout;
		int32_t now     = systemTime() / 1000 / 1000;
		msg->findInt32("timeout", &timeout);
		if (timeout <= now) {
			mMessageList.erase(mMessageList.begin());
			return msg;
		}
	}

	return NULL;
}

bool AThread::threadLoop()
{
	while (!mRequestExit) {

		int timeout = getEarliestTimeout();
		printf("epoll wait timeout %d\n", timeout);
		struct epoll_event eventItems[EPOLL_MAX_EVENTS];
		int eventCount = epoll_wait(mEpollfd, eventItems, EPOLL_MAX_EVENTS, timeout);
		printf("    epoll exit... timeout %d\n", eventCount);

		// Check for poll error.
		if (eventCount < 0) {
			if (errno == EINTR) {
				continue;
			}
			ALOGW("Poll failed with an unexpected error, errno=%d", errno);
		}

		// Check for poll timeout.
		if (eventCount == 0) {
			sp<AMessage> msg = getEarliestMessage();
			if (!!msg.get()) {

				if (msg->what() == 0) {

					sp<RefBase> obj;

					msg->findObject("callback", &obj);
					sp<CallbackWrap> callb = static_cast<CallbackWrap *>(obj.get());

					(callb->cb)();

				} else {
					onMessage(msg);
				}
			}

			continue;
		}

		for (int i = 0; i < eventCount; i++) {
			int fd = eventItems[i].data.fd;
			if (fd == mWakeupPipe[0]) {
				int dump = 0;

				Mutex::Autolock autoLock(mLock);
				int delta = mWritePipeCount - mReadPipeCount;
				while (delta-- > 0) {
					mReadPipeCount++;
					read(mWakeupPipe[0], &dump, 1);
				}
			} else {
				onPolling(&eventItems[i]);
			}
		}
	}
	return false;
}

}  // namespace android

#endif  // A_MESSAGE_H_



