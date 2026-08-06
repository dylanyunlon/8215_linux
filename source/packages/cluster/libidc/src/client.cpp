/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>

#include <idc_client.h>
#include <idc_dev.h>

#include "idc_priv.h"
#include <mutex>
#include <linux/ion.h>
#include <sys/socket.h>
#include "clog.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <vector>

#define SOCKET_PATH "/tmp/cluster_unix_socket"

#define ANCIL_FD_BUFFER(n) \
    struct { \
      struct cmsghdr h; \
      int fd[n]; \
    }
using idc_utils::CLog;
#define TAG "[idc client]"

class InterdomainChannelImpl : public IInterdomainChannel {
public:
    InterdomainChannelImpl(void) {
        m_isInited = false;
        m_channel = -1;
    }

    ~InterdomainChannelImpl(void) {
    }

    bool init(void);
    void setName(const char *name);
    void getName(char *name);
    void addListener(void);
    void removeListener(void);

    bool connect(const char *domain_name);
    void disconnect(void);

    bool postEvent(idc_event_t *event, IIDCEventCallback *cb, void *extra);
    bool postBuffer(idc_buffer_t *buf, IIDCEventCallback *cb, void *extra);
    bool postGfxBuffer(idc_gfx_buffer_t *buf, IIDCEventCallback *cb, void *extra);

    void destroy(void);

    int allocateAndFillIonBuffer(const void *data, size_t size, int *ionFd);
    int createClusterConnection();
    int ancil_send_fds_with_buffer(int sock, const int *fds, unsigned n_fds);


private:
    bool m_isInited;
    int  m_channel;
    std::mutex postEventMutex;
    


};


static InterdomainChannelImpl g_defaultChannel;
static int clientSocket=0;


IInterdomainChannel *IInterdomainChannel::get(void) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(8080);

    if (::connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error connecting to server");
        close(clientSocket);
    }

#if 0
    if (!g_defaultChannel.init()) {
        return (NULL);
    }
#endif
    return (&g_defaultChannel);
}

IInterdomainChannel *IInterdomainChannel::create(void) {
    InterdomainChannelImpl *idc_channel = new InterdomainChannelImpl();

    if (!idc_channel) {
        return (NULL);
    }
    if (!idc_channel->init()) {
        delete idc_channel;

        return (NULL);
    }

    return (idc_channel);
}

bool InterdomainChannelImpl::init(void) {
    if (m_isInited) {
        return (true);
    }
    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return (false);
    }

    struct idc_allocation_data data;

    sprintf(data.name, "idc.tx.channel-%d", getpid());
    data.is_tx = true;
    int ret = 0;
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_NEW_CHANNEL -> 11111111\n");
        return (false);
    }
    printf("[idc] InterdomainChannelImpl::init -> channel: %d\n", data.channel);
    m_channel = data.channel;
    m_isInited = true;

    return (true);
}

void InterdomainChannelImpl::setName(const char *name) {
    if (m_channel < 0) {
        return;
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return;
    }

    struct idc_name_data data;

    strcpy(data.name, name);
    data.channel = m_channel;
    int ret = 0;
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_SET_NAME\n");
    }
}

void InterdomainChannelImpl::getName(char *name) {
    if (m_channel < 0) {
        return;
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return;
    }

    struct idc_name_data data;

    data.channel = m_channel;
    int ret = 0;
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_GET_NAME\n");
    } else {
        strcpy(name, data.name);
    }
}

void InterdomainChannelImpl::addListener(void) {
}

void InterdomainChannelImpl::removeListener(void) {
}

bool InterdomainChannelImpl::connect(const char *domain_name) {
    if (m_channel < 0) {
        return (false);
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return (false);
    }

    struct idc_connection_data connection;

    strcpy(connection.domain_name, domain_name);
    connection.channel = m_channel;
    int ret = 0;
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_CONNECT\n");
        return (false);
    }

    return (true);
}

void InterdomainChannelImpl::disconnect(void) {
    if (m_channel < 0) {
        IDC_LOG("[idc] %s -> disconnect invalid idc channel!\n", __func__);
        return;
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return;
    }

    struct idc_disconnection_data disconnection;

    disconnection.channel = m_channel;
    int ret = 0;
    if (ret < 0) {
        IDC_LOG("[idc] %s -> failed to ioctl IDC_IOC_DISCONNECT\n", __func__);
    }
}

int InterdomainChannelImpl::allocateAndFillIonBuffer(const void *data, size_t size, int *ionFd){
    int ionDevFd = open("/dev/ion", O_RDONLY | O_CLOEXEC);
    int ret;
    if (ionDevFd < 0) {
        UTILS_LOGE(TAG, "Failed to open Ion device");
        return -1;
    }

    struct ion_allocation_data ionAllocData = {
        .len = size,
        .align = 0,
        .heap_id_mask = ION_HEAP_TYPE_MM,
        .flags = 0,
    };

    if (ioctl(ionDevFd, ION_IOC_ALLOC, &ionAllocData) < 0) {
        UTILS_LOGE(TAG, "Ion allocation failed");
        close(ionDevFd);
        return -1;
    }
    ion_user_handle_t handle = ionAllocData.handle;

    struct ion_fd_data fd_data = {
        .handle = handle,
    };
    ret = ioctl(ionDevFd, ION_IOC_MAP, &fd_data);
    if (fd_data.fd < 0) {
        UTILS_LOGE(TAG, "Failed to get Ion buffer fd");
        close(ionDevFd);
        return -1;
    }

    void *ionBuffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,  fd_data.fd, 0);
    if (ionBuffer == MAP_FAILED) {
        UTILS_LOGE(TAG, "Failed to map Ion buffer");
        close(fd_data.fd);
        close(ionDevFd);
        return -1;
    }

    memcpy(ionBuffer, data, size);
    munmap(ionBuffer, size);
    close(ionDevFd);

    *ionFd =  fd_data.fd;
    return 0;
}

int InterdomainChannelImpl::createClusterConnection(){
    // create Socket
    int client_fd = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
    if (client_fd == -1) {
        UTILS_LOGE(TAG,"Socket creation failed");
    } else{
        struct sockaddr_un server_address;
        server_address.sun_family = AF_LOCAL;
        strcpy(server_address.sun_path, SOCKET_PATH);

        if (::connect(client_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            UTILS_LOGE(TAG,"Connection failed");
            close(client_fd);
            return -1;
        }
        UTILS_LOGI(TAG,"cluster client connect success");
    }
    return client_fd;
}

int InterdomainChannelImpl::ancil_send_fds_with_buffer(int sock, const int *fds, unsigned n_fds)
{
    struct msghdr msghdr;
    char nothing = '!';
    struct iovec nothing_ptr;
    struct cmsghdr *cmsg;
    unsigned i;
    ANCIL_FD_BUFFER(1) buffer;

    nothing_ptr.iov_base = &nothing;
    nothing_ptr.iov_len = 1;
    msghdr.msg_name = NULL;
    msghdr.msg_namelen = 0;
    msghdr.msg_iov = &nothing_ptr;
    msghdr.msg_iovlen = 1;
    msghdr.msg_flags = 0;
    msghdr.msg_control = &buffer;
    msghdr.msg_controllen = sizeof(struct cmsghdr) + sizeof(int) * n_fds;
    cmsg = CMSG_FIRSTHDR(&msghdr);
    cmsg->cmsg_len = msghdr.msg_controllen;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    for(i = 0; i < n_fds; i++)
     ((int *)CMSG_DATA(cmsg))[i] = fds[i];
    return(sendmsg(sock, &msghdr, 0) >= 0 ? 0 : -1);
}


bool InterdomainChannelImpl::postEvent(idc_event_t *event, IIDCEventCallback *cb,
                                       void *extra) {
    std::lock_guard<std::mutex> lock(postEventMutex);
    idc_event_rsp_t *event_rsp = NULL;
    uint8_t *malloc_data = NULL;
/**
    if (m_channel < 0) {
        return (false);
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return (false);
    }
**/
    struct idc_event_data event_data;

    event_data.channel = 1;
    if (IDC_EVENT_RAW_DATA == event->id) {
        event_data.id = IDC_KM_EVENT_RAW_DATA;
        event_data.data = (uint8_t *)event->param1;
        event_data.data_id = ((event->param2 >> 32) & 0xFFFFFFFF);
        event_data.data_sz = (event->param2 & 0xFFFFFFFF);
    } else if (IDC_EVENT_MESSAGE == event->id) {
        IDCMessage *msg = (IDCMessage *)(event->param2);
        int data_sz = 0;

        event_data.id = IDC_KM_EVENT_MESSAGE;
        event_data.data_id = (event->param1 & 0xFFFFFFFF);

        malloc_data = (uint8_t *)malloc(msg->dataSize());
        if (!malloc_data) {
            return (false);
        }
        data_sz = msg->readData(malloc_data, msg->dataSize());
        event_data.data = malloc_data;
        event_data.data_sz = data_sz;
    } else if (IDC_EVENT_PARCEL == event->id) {
        IDCParcel *parcel = (IDCParcel *)(event->param2);

        event_data.id = IDC_KM_EVENT_PARCEL;
        event_data.data_id = (event->param1 & 0xFFFFFFFF);
        event_data.data = (uint8_t *)parcel->data();
        event_data.data_sz = parcel->dataSize();
    } else {
        printf("[idc] %s -> post a unsupprted event\n", __func__);
        return (false);
    }
    if (cb) {
        event_rsp = (idc_event_rsp_t *)malloc(sizeof(idc_event_rsp_t));
        if (!event_rsp) {
            if (malloc_data) {
                free(malloc_data);
            }
            return (false);
        }
        event_rsp->event = event->id;
        event_rsp->cb = cb;
        event_rsp->extra = extra;
    }
    event_data.rsp_id = (uint64_t)event_rsp;

    printf("[idc] %s -> event id: %u, rsp id: %p\n", __func__,
           event_data.id, (void *)event_data.rsp_id);
    std::vector<uint8_t> buffer;
   // int ret = ioctl(idc_dev_fd, IDC_IOC_POST_EVENT, &event_data);
    if(event_data.data_sz >0) {

        for(int i = 0;i<event_data.data_sz;i++){
            buffer.push_back(event_data.data[i]);
        }

        send(clientSocket, buffer.data(), event_data.data_sz, 0);

       // atclibidc::ATCIDCMessagerTransport::getInstance()->postEvent(event_data.id, event_data.data_id,buffer);
    }

    int ret = 0;
    if (malloc_data) {
        free(malloc_data);
        malloc_data = NULL;
    }
    if (ret < 0) {
        if (event_rsp) {
            free(event_rsp);
        }
        printf("[idc] %s -> failed to ioctl IDC_IOC_POST_EVENT\n", __func__);
        return (false);
    }

    return (true);
}

bool InterdomainChannelImpl::postBuffer(idc_buffer_t *buf, IIDCEventCallback *cb,
                                        void *extra) {
    idc_event_rsp_t *event_rsp = NULL;

    if (m_channel < 0) {
        return (false);
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return (false);
    }

    struct idc_buffer_data buf_data;

    buf_data.channel = m_channel;
    buf_data.fd = buf->fd;
    buf_data.size = buf->size;

    if (cb) {
        event_rsp = (idc_event_rsp_t *)malloc(sizeof(idc_event_rsp_t));
        if (!event_rsp) {
            return (false);
        }
        event_rsp->event = IDC_EVENT_DMA_BUFFER;
        event_rsp->cb = cb;
        event_rsp->extra = extra;
    }
    buf_data.rsp_id = (uint64_t)event_rsp;

    printf("[idc] %s ioctl -> rsp id: %p\n", __func__, (void *)buf_data.rsp_id);
   // int ret = ioctl(idc_dev_fd, IDC_IOC_POST_BUFFER, &buf_data);
    int ret = 0;
    if (ret < 0) {
        if (event_rsp) {
            free(event_rsp);
        }
        printf("[idc] %s -> failed to ioctl IDC_IOC_POST_BUFFER\n", __func__);
        return (false);
    } else {
        buf->fence = buf_data.fence;
    }

    return (true);
}

bool InterdomainChannelImpl::postGfxBuffer(idc_gfx_buffer_t *buf,
        IIDCEventCallback *cb, void *extra) {
    idc_event_rsp_t *event_rsp = NULL;

    if (m_channel < 0) {
        return (false);
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return (false);
    }

    struct idc_gfx_buffer_data buf_data;

    buf_data.channel = m_channel;
    buf_data.fd = buf->fd;
    buf_data.fence = -1;
    buf_data.meta.width = buf->width;
    buf_data.meta.height = buf->height;
    buf_data.meta.stride = buf->stride;
    buf_data.meta.format = buf->format;
    memcpy(buf_data.meta.priv, buf->priv, sizeof(buf->priv));
    if (cb) {
        event_rsp = (idc_event_rsp_t *)malloc(sizeof(idc_event_rsp_t));
        if (!event_rsp) {
            return (false);
        }
        event_rsp->event = IDC_EVENT_GFX_DMA_BUFFER;
        event_rsp->cb = cb;
        event_rsp->extra = extra;
    }
    buf_data.rsp_id = (uint64_t)event_rsp;

    //printf("[idc] %s ioctl -> rsp id: %p\n", __func__, (void *)buf_data.rsp_id);
  //  int ret = ioctl(idc_dev_fd, IDC_IOC_POST_GFX_BUFFER, &buf_data);
    int ret =  0;
    if (ret < 0) {
        if (event_rsp) {
            free(event_rsp);
        }
        printf("[idc] %s -> failed to ioctl IDC_IOC_POST_GFX_BUFFER\n", __func__);
        return (false);
    } else {
        buf->fence = buf_data.fence;
    }

    return (true);
}

void InterdomainChannelImpl::destroy(void) {
    if (m_channel < 0) {
        return;
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return;
    }

    struct idc_destroy_data destroy;

    destroy.channel = m_channel;
    int ret = 0;
    if (ret < 0) {
        printf("[idc] %s -> failed to ioctl IDC_IOC_DESTROY\n", __func__);
    }

    delete this;
}
