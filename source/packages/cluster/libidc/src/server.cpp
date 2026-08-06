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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>

#include <idc_client.h>
#include <idc_dev.h>
#include <idc_server.h>
#include "clog.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <thread>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "idc_priv.h"

#define SOCKET_PATH "/tmp/cluster_unix_socket"

#define IVI_CHANNEL_NAME "cluster.transport"
#define RECEIVE_ALBUM_IMAGE 0x9
#define ARG1 "arg1"
#define ARG2 "arg2"
const static char *TAG = "server";

#define ANCIL_FD_BUFFER(n) \
    struct { \
      struct cmsghdr h; \
      int fd[n]; \
    }
using idc_utils::CLog;

int currentState;
int ancil_recv_fds_with_buffer(int sock, int *fds, unsigned n_fds, void *buffer)
{
  struct msghdr msghdr;
  char nothing;
  struct iovec nothing_ptr;
  struct cmsghdr *cmsg;
  unsigned i;
  UTILS_LOGI(TAG, "ancil_recv_fds_with_buffer");

  nothing_ptr.iov_base = &nothing;
  nothing_ptr.iov_len = 1;
  msghdr.msg_name = NULL;
  msghdr.msg_namelen = 0;
  msghdr.msg_iov = &nothing_ptr;
  msghdr.msg_iovlen = 1;
  msghdr.msg_flags = 0;
  msghdr.msg_control = buffer;
  msghdr.msg_controllen = sizeof(struct cmsghdr) + sizeof(int) * n_fds;
  cmsg = CMSG_FIRSTHDR(&msghdr);
  cmsg->cmsg_len = msghdr.msg_controllen;
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  for(i = 0; i < n_fds; i++)
    ((int *)CMSG_DATA(cmsg))[i] = -1;

  if(recvmsg(sock, &msghdr, 0) <= 0){
      return(-1);
  }
  for(i = 0; i < n_fds; i++)
    fds[i] = ((int *)CMSG_DATA(cmsg))[i];
  n_fds = (cmsg->cmsg_len - sizeof(struct cmsghdr)) / sizeof(int);
  return(n_fds);
}

void listenerClientEvent(){
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        UTILS_LOGE(TAG,"Error creating socket");
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        UTILS_LOGE(TAG,"Error binding address");
        close(serverSocket);
    }

    if (listen(serverSocket, 5) == -1) {
        UTILS_LOGE(TAG,"Error listening for connections");
        close(serverSocket);
    }

    UTILS_LOGI(TAG,"Server waiting for connections..." );

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == -1) {
            UTILS_LOGE(TAG,"Error accepting connection");
            continue;
        }

        UTILS_LOGI(TAG, "Connection accepted from clientSocket=%d ",clientSocket);
        std::thread run([=]() {
            char buffer[4096];
            int bytesRead;
            while ((bytesRead = recv(clientSocket, buffer, 4096, 0)) > 0) {
                idc_event_t idc_event;
                IDCMessage idc_msg;
                IDCMessage idc_reply_msg(0x100);
                int data_sz = static_cast<int>(sizeof(buffer));
                idc_msg.setData((uint8_t*)buffer, data_sz);
                idc_event.id = IDC_EVENT_MESSAGE;
                idc_event.param1 =  (0xFAFAFAFA & 0xFFFFFFFF);
                idc_event.param2 = (uint64_t)&idc_msg;
                IDCMonitorImpl::get()->notify(1, &idc_event, &idc_reply_msg);
            }
            if (bytesRead == 0 || bytesRead == -1) {
                IDCMessage msg;
                IDCMessage idc_reply_msg(0x100);
                idc_event_t  event;
                event.id = IDC_EVENT_DISCONNECTED;
                event.param1 = 0xFAFAFAFA;
                event.param2 = (uint64_t)&msg;
                IDCMonitorImpl::get()->notify(1, &event, &idc_reply_msg);
                UTILS_LOGI(TAG, "Client disconnected bytesRead = %d",bytesRead);
            }
            close(clientSocket);
        });
        run.detach();
    }
    close(serverSocket);
}


void listenerClientImage(){
    //listener to client read image
        bool shouldExit = false;
        int server_fd = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
        if (server_fd == -1) {
            UTILS_LOGE(TAG,"Socket creation failed");
            return ;
        }

        struct sockaddr_un address;
        address.sun_family = AF_LOCAL;
        strcpy(address.sun_path, SOCKET_PATH);
    
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
            UTILS_LOGE(TAG,"Bind failed");
            close(server_fd);
            return ;
        }

        if (listen(server_fd, 1) == -1) {
            UTILS_LOGE(TAG,"Listen failed");
            close(server_fd);
            return ;
        }

        while (!shouldExit) {
             int client_fd = accept(server_fd, nullptr, nullptr);
             if (client_fd == -1) {
                 //UTILS_LOGE(TAG, "Accept failed");
                 continue;
             }else {
                 UTILS_LOGI(TAG, "accept success , client_fd =%d ",client_fd);
             }
                while(1) {
                 int received_fd;
                 ANCIL_FD_BUFFER(1) buffer;
                 int ret = ancil_recv_fds_with_buffer(client_fd, &received_fd, 1 , &buffer);

                 if(ret <= 0){
                     UTILS_LOGE(TAG, "recv_fds failed =%d ",ret);
                     close(client_fd);
                     shouldExit =true;
                     break;
                 }
                 int file_size = lseek(received_fd,0 ,SEEK_END);
                 if(file_size > 0) {
                     unsigned char* data = reinterpret_cast<unsigned char*>(mmap(NULL, file_size, PROT_READ, MAP_SHARED, received_fd, 0));
                     UTILS_LOGI(TAG, "recvAlbumPicThread, file_size:%d", file_size);
                     IDCMessage idc_reply_msg(0x100);
                     idc_event_t idc_event;
    
                     IDCMessage msg(0x100);
                     msg.m_msg.what = RECEIVE_ALBUM_IMAGE;
                     msg.putExtra(ARG1, std::string("1"));
                     msg.putExtra(ARG2, data,file_size);
    
                     idc_event.id = IDC_EVENT_MESSAGE;
                     idc_event.param1 = 0xFAFAFAFA;
                     idc_event.param2 = (uint64_t)&msg;
    
                     IDCMonitorImpl::get()->notify(1, &idc_event, &idc_reply_msg);
    
                     if(data == MAP_FAILED){
                         UTILS_LOGE(TAG, "mmap failed!\n");
                         close(received_fd);
                         close(client_fd);
                         continue;
                     }
                     munmap(data, file_size);
                 }
                 close(received_fd);
             }
         }
         close(server_fd);
         unlink(SOCKET_PATH);
         listenerClientImage();
}

class IDCProxyImpl : public IInterdomainChannelProxy {
public:
    IDCProxyImpl(int rx_channel);
    ~IDCProxyImpl(void);

    void addListener(IIDCProxyListener *listener);
    void removeListener(IIDCProxyListener *listener);
    void getName(char *name);
    bool getEvent(idc_event_t *event);
    bool getBuffer(idc_buffer_t *buf);
    void release(void);

private:
    IIDCProxyListener *m_listeners[MAX_IDC_LISTENER_NUM];
    int m_listener_count;
    int m_channel;

    IDCProxyImpl *m_prev;
    IDCProxyImpl *m_next;

    friend class IDCMonitorImpl;
};

static IDCMonitorImpl g_idc_monitor;
static IDCProxyImpl *g_idc_channels = NULL;

IInterdomainChannelProxy *IInterdomainChannelProxy::get(const char *name) {
    int idc_dev_fd = get_idc_device();
#if 0
    if (idc_dev_fd < 0) {
        idc_dev_fd = init_idc_device();
        if (idc_dev_fd < 0) {
            return (NULL);
        }
    }
#endif

if(strcmp(name,IVI_CHANNEL_NAME) == 0){
    UTILS_LOGI(TAG, "recvAlbumPicThread");
    std::thread recvAlbumPicThread([&]() {
        listenerClientImage();

    });
     recvAlbumPicThread.detach();
    std::thread recvEventThread([&]() {
        listenerClientEvent();

    });
     recvEventThread.detach();
     /**
    std::thread registerCommonApiThread([&]() {
            //listener common api
        std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();
        std::shared_ptr<atclibidc::ATCIDCMessagerServiceImp> myService = std::make_shared<atclibidc::ATCIDCMessagerServiceImp>();
        if (runtime == NULL) {
            UTILS_LOGE(TAG, "CommonAPI::Runtime::get fail\r\n");
            return (NULL);
        }

        if (myService == NULL) {
            UTILS_LOGE(TAG, "std::make_shared<DeviceManagerStubImpl> fail!\r\n");
            return (NULL);
        }

        bool successfullyRegistered = runtime->registerService("local", "atclibsidc", myService);
        while (!successfullyRegistered) {
            UTILS_LOGE(TAG, "Register Service failed, trying again in 100 milliseconds...\r\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            successfullyRegistered = runtime->registerService("local", "atclibsidc", myService);
        }
    });
    registerCommonApiThread.detach();
    **/
}
    if (!name) {
        return (NULL);
    }

    struct idc_allocation_data data;

    strcpy(data.name, name);
    data.is_tx = false;
    
    int ret= 0;
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_NEW_CHANNEL\n");
        return (NULL);
    }
    printf("[idc] IInterdomainChannelProxy::get -> channel: %d\n", data.channel);
    
    IDCProxyImpl *pChannelProxy = new IDCProxyImpl(1);
    if (!pChannelProxy) {
        return (NULL);
    }

    return (pChannelProxy);
}

IInterdomainChannelMonitor *IInterdomainChannelMonitor::get(void) {
    return (&g_idc_monitor);
}

IDCProxyImpl::IDCProxyImpl(int rx_channel) {
    m_channel = rx_channel;
    if (!g_idc_channels) {
        m_next = m_prev = this;
        g_idc_channels = this;
    } else {
        g_idc_channels->m_prev->m_next = this;
        m_prev = g_idc_channels->m_prev;
        m_next = g_idc_channels;
        g_idc_channels->m_prev = this;
    }
    m_listener_count = 0;
}

IDCProxyImpl::~IDCProxyImpl(void) {
}

void IDCProxyImpl::addListener(IIDCProxyListener *listener) {
    if (!listener) {
        return;
    }
    for (int i = 0; i < m_listener_count; i++) {
        if (m_listeners[i] == listener) {
            return;
        }
    }
    if (m_listener_count >= MAX_IDC_LISTENER_NUM) {
        return;
    }
    m_listeners[m_listener_count] = listener;
    m_listener_count++;
}

void IDCProxyImpl::removeListener(IIDCProxyListener *listener) {
    int idx = -1;

    if (!listener) {
        return;
    }
    for (int i = 0; i < m_listener_count; i++) {
        if (m_listeners[i] == listener) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        return;
    }
    for (int i = idx; i < m_listener_count-1; i++) {
        m_listeners[i] = m_listeners[i+1];
    }
    m_listener_count--;
}

void IDCProxyImpl::getName(char *name) {
    if (!name) {
        return;
    }
    if (m_channel < 0) {
        return;
    }

    int idc_dev_fd = get_idc_device();
    if (idc_dev_fd < 0) {
        return;
    }

    struct idc_name_data data;

    data.channel = m_channel;
    int ret = ioctl(idc_dev_fd, IDC_IOC_GET_NAME, &data);
    if (ret < 0) {
        printf("[idc] Failed to ioctl IDC_IOC_GET_NAME\n");
    } else {
        strcpy(name, data.name);
    }
}

bool IDCProxyImpl::getEvent(idc_event_t *event) {
    (void)event;

    return (true);
}

bool IDCProxyImpl::getBuffer(idc_buffer_t *buf) {
    (void)buf;
    return (true);
}

void IDCProxyImpl::release(void) {
}

IDCMonitorImpl *IDCMonitorImpl::get(void) {
    return (&g_idc_monitor);
}

void IDCMonitorImpl::addListener(IIDCMonitorListener *listener) {
    if (!listener) {
        return;
    }
    for (int i = 0; i < m_listener_count; i++) {
        if (m_listeners[i] == listener) {
            return;
        }
    }
    if (m_listener_count >= MAX_IDC_LISTENER_NUM) {
        return;
    }
    m_listeners[m_listener_count] = listener;
    m_listener_count++;
}

void IDCMonitorImpl::removeListener(IIDCMonitorListener *listener) {
    int idx = -1;

    if (!listener) {
        return;
    }
    for (int i = 0; i < m_listener_count; i++) {
        if (m_listeners[i] == listener) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        return;
    }
    for (int i = idx; i < m_listener_count-1; i++) {
        m_listeners[i] = m_listeners[i+1];
    }
    m_listener_count--;
}

void IDCMonitorImpl::notify(const char *domain, const char *channel,
                            idc_event_t *event) {
    for (int i = 0; i < m_listener_count; i++) {
        if (m_listeners[i]) {
            m_listeners[i]->onEvent(domain, channel, event);
        }
    }
}

void IDCMonitorImpl::notify(int32_t channel, idc_event_t *event, void *reply) {
    std::lock_guard<std::mutex> lock(notifyMutex);

    IDCProxyImpl *event_channel = NULL;
    IDCProxyImpl *tmp = g_idc_channels;

    while (tmp) {
        if (tmp->m_channel == channel) {
            event_channel = tmp;
            break;
        }
        tmp = tmp->m_next;
        if (tmp == g_idc_channels) {
            break;
        }
    }
    if (!event_channel) {
        return;
    }
    for (int i = 0; i < event_channel->m_listener_count; i++) {
        if (event_channel->m_listeners[i]) {
            event_channel->m_listeners[i]->onEvent(event_channel, event, reply);
        }
    }
}
