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
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>

#include <idc_common.h>

#include <idc_dev.h>
#include "idc_priv.h"
#include "rawstring.h"

static int g_dev_fd = -1;
static pthread_t g_read_events_tid;

#define DEFAULT_PARCEL_CAPACITY    (4096)

// This macro should never be used at runtime, as a too large value
// of s could cause an integer overflow. Instead, you should always
// use the wrapper function pad_size()
#define PAD_SIZE_UNSAFE(s) (((s)+3)&~3)

static size_t pad_size(size_t s) {
    return PAD_SIZE_UNSAFE(s);
}

IDCMessage::IDCMessage()
{
    m_msg.what = 0;
    m_msg.ret = 0;
    m_msg.when = 0;
    m_pData = new RawString();
}

IDCMessage::IDCMessage(int message) : IDCMessage()
{
    m_msg.what = message;
}

IDCMessage::IDCMessage(const IDCMessage& other)
{
    m_msg = other.m_msg;

    if (m_pData && other.m_pData) {
        *m_pData = *other.m_pData;
    }
}

IDCMessage::~IDCMessage()
{
    SAFE_DELETE(m_pData);
}

IDCMessage& IDCMessage::operator=(int message)
{
    m_msg.what = message;
    m_msg.ret = 0;
    m_msg.when = 0;
    if (m_pData) {
        m_pData->clear();
    }

    return *this;
}

IDCMessage& IDCMessage::operator=(const IDCMessage& message)
{
    m_msg = message.m_msg;
    if (m_pData && message.m_pData) {
        *m_pData = *message.m_pData;
    }

    return *this;
}

bool IDCMessage::operator==(const IDCMessage& message) const
{
    return (m_msg.what == message.m_msg.what);
}

IDCMessage& IDCMessage::putExtra(const std::string &key, char value)
{
    unsigned int type = EXTRA_CHAR;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&value, sizeof(char));

    return *this;
}

/*use bool as char*/
IDCMessage& IDCMessage::putExtra(const std::string &key, bool value)
{
    unsigned int type = EXTRA_CHAR;
    unsigned int size = 0;
    char charValue = (char)value;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&charValue, sizeof(char));

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, int value)
{
    unsigned int type = EXTRA_INT;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&value, sizeof(int));

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, double value)
{
    unsigned int type = EXTRA_DOUBLE;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&value, sizeof(double));

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, float value)
{
    unsigned int type = EXTRA_FLOAT;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&value, sizeof(float));

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, unsigned long long value)
{
    unsigned int type = EXTRA_UNSIGNED_LONG_LONG;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(&value, sizeof(unsigned long long));

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, const std::string &value)
{
    unsigned int type = EXTRA_STRING;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value len*/
    size = value.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(value.c_str(), size);

    return *this;
}

IDCMessage& IDCMessage::putExtra(const std::string &key, const unsigned char *value, unsigned int length)
{
    unsigned int type = EXTRA_CHAR_ARRAY;
    unsigned int size = 0;

    if (!m_pData) {
        return *this;
    }
    /*key len*/
    size = key.size();
    m_pData->addTail(&size, sizeof(unsigned int));
    /*key*/
    m_pData->addTail(key.c_str(), key.size());
    /*type*/
    m_pData->addTail(&type, sizeof(unsigned int));
    /*value len*/
    m_pData->addTail(&length, sizeof(unsigned int));
    /*value*/
    m_pData->addTail(value, length);

    return *this;
}

char IDCMessage::getCharExtra(const std::string &key, char defaultValue) const
{
    char value = 0;
    int valuelen = sizeof(char);

    int ret = findValue(EXTRA_CHAR, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

int IDCMessage::getIntExtra(const std::string &key, int defaultValue) const
{
    int value = 0;
    int valuelen = sizeof(int);

    int ret = findValue(EXTRA_INT, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

double IDCMessage::getDoubleExtra(const std::string &key, double defaultValue) const
{
    double value = 0;
    int valuelen = sizeof(double);

    int ret = findValue(EXTRA_DOUBLE, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

float IDCMessage::getFloatExtra(const std::string &key, float defaultValue) const
{
    float value = 0;
    int valuelen = sizeof(float);

    int ret = findValue(EXTRA_FLOAT, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

unsigned long long IDCMessage::getUnsignedLongLongExtra(const std::string &key, unsigned long long defaultValue) const
{
    unsigned long long value = 0;
    int valuelen = sizeof(unsigned long long);
    int ret = findValue(EXTRA_UNSIGNED_LONG_LONG, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

bool IDCMessage::getBoolExtra(const std::string &key, bool defaultValue) const
{
    char value = 0;
    int valuelen = sizeof(int);
    int ret = findValue(EXTRA_CHAR, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return (bool)value;
    }
}

std::string IDCMessage::getStringExtra(const std::string &key, const std::string defaultValue) const
{
    std::string value = "";
    int valuelen = 0;
    int ret = findValue(EXTRA_STRING, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

const unsigned char* IDCMessage::getArrayExtra(const std::string &key, unsigned int *arraylength, const unsigned char *defaultValue) const
{
    unsigned char *value = NULL;
    int valuelen = 0;

    if (NULL == arraylength) {
        return NULL;
    }
    int ret = findValue(EXTRA_CHAR_ARRAY, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        *arraylength = valuelen;
        return value;
    }
}

int IDCMessage::size() const
{
    if (!m_pData) {
        return 0;
    }
    return sizeof(m_msg) + m_pData->getStrLen();
}

int IDCMessage::dataSize() const
{
    if (!m_pData) {
        return sizeof(m_msg);
    } else {
        return (sizeof(m_msg) + m_pData->getStrLen());
    }
}

int IDCMessage::readData(uint8_t *data, int size)
{
    int data_sz = 0;

    if (size < (int)sizeof(m_msg)) {
        return 0;
    }
    memcpy(data, &m_msg, sizeof(m_msg));
    data_sz += sizeof(m_msg);
    if (m_pData) {
        if (size >= m_pData->getStrLen() + (int)sizeof(m_msg)) {
	    memcpy(data + sizeof(m_msg), m_pData->getStr(), m_pData->getStrLen());
	    data_sz += m_pData->getStrLen();
        } else {
	    memcpy(data + sizeof(m_msg), m_pData->getStr(), size - sizeof(m_msg));
	    data_sz += (size - sizeof(m_msg));
        }
    }

    return (data_sz);
}

int IDCMessage::setData(uint8_t *data, int size)
{
    idc_msg_header_t *msg = NULL;

    if (!data) {
        return (-1);
    }
    if (size < (int)sizeof(m_msg)) {
        return (-1);
    }
    msg = (idc_msg_header_t *)data;
    m_msg = *msg;
    if (m_pData) {
        m_pData->setStr(data + sizeof(m_msg), size - sizeof(m_msg));
    }

    return 0;
}

int IDCMessage::findValue(int type, const std::string &key,  void *value, int *valueLen) const
{
    unsigned int keylen = 0;
    int tmpType;

    if (!m_pData) {
        return (-1);
    }
    unsigned int remain = m_pData->getStrLen();
    std::string tmpKey;

    const unsigned char *data = m_pData->getStr();
    while (data && remain > 0) {
        /*key len*/
        keylen = getInt(data);
        data += sizeof(unsigned int);
        remain -= sizeof(unsigned int);

	/*key*/
        tmpKey.assign((const char*)data, keylen);
        data += keylen;
        remain -= keylen;

        /*type*/
        tmpType = getInt(data);
        data += sizeof(unsigned int);
        remain -= sizeof(unsigned int);
        //IDC_LOG("findValue, type:%d, tmpType:%d, key:%s, tmpKey:%s\n", type, tmpType, key.c_str(), tmpKey.c_str());
        if ((tmpType == type) && (0 == tmpKey.compare(key))) {
            /*finded, get value*/
            switch (tmpType) {
            case EXTRA_CHAR: {
                *(char*)value = getChar(data);
            }
            break;

            case EXTRA_INT: {
                *(int*)value = getInt(data);
            }
            break;

            case EXTRA_DOUBLE: {
                *(double*)value = getDouble(data);
            }
            break;

            case EXTRA_FLOAT: {
                *(float*)value = getFloat(data);
            }
            break;

            case EXTRA_UNSIGNED_LONG_LONG: {
                *(unsigned long long*)value = getLongLong(data);
            }
            break;

            case EXTRA_STRING: {
                unsigned int valueStringLen;

                valueStringLen = getInt(data);
                data += sizeof(unsigned int);
                remain -= sizeof(unsigned int);

                ((std::string*)value)->assign((const char*)data, valueStringLen);
            }
            break;

            case EXTRA_CHAR_ARRAY: {
                unsigned int valueArrayLen;

                valueArrayLen = getInt(data);
                data += sizeof(unsigned int);
                remain -= sizeof(unsigned int);
                *valueLen = (int)valueArrayLen;
                *(unsigned char**)value = (unsigned char*)data;
            }
            break;
            }

            return 0;

        } else {
            /*not finded, go on*/
            switch (tmpType) {
            case EXTRA_CHAR: {
                data += sizeof(char);
                remain -= sizeof(char);
                break;
            }

            case EXTRA_INT: {
                data += sizeof(int);
                remain -= sizeof(int);
                break;
            }

            case EXTRA_DOUBLE: {
                data += sizeof(double);
                remain -= sizeof(double);
                break;
            }

            case EXTRA_FLOAT: {
                data += sizeof(float);
                remain -= sizeof(float);
                break;
            }

            case EXTRA_UNSIGNED_LONG_LONG: {
                data += sizeof(unsigned long long);
                remain -= sizeof(unsigned long long);
                break;
            }

            case EXTRA_STRING: {
                unsigned int valueStringLen;

                valueStringLen = getInt(data);
                data += sizeof(unsigned int);
                remain -= sizeof(unsigned int);
                data += valueStringLen;
                remain -= valueStringLen;
                break;
            }

            case EXTRA_CHAR_ARRAY: {
                unsigned int valueArrayLen;

                valueArrayLen = getInt(data);
                data += sizeof(unsigned int);
                remain -= sizeof(unsigned int);
                data += valueArrayLen;
                remain -= valueArrayLen;
                break;
            }
	    }
        }
    }

    return -1;
}

char IDCMessage::getChar(const unsigned char *p) const
{
    char value = 0;

    memcpy(&value, p, sizeof(char));

    return value;
}

unsigned int IDCMessage::getInt(const unsigned char *p) const
{
    unsigned int value = 0;

    memcpy(&value, p, sizeof(unsigned int));

    return value;
}

double IDCMessage::getDouble(const unsigned char *p) const
{
    double value = 0.0;

    memcpy(&value, p, sizeof(double));

    return value;
}

float IDCMessage::getFloat(const unsigned char *p) const
{
    float value = 0.0;

    memcpy(&value, p, sizeof(float));

    return value;
}

unsigned long long IDCMessage::getLongLong(const unsigned char *p) const
{
    unsigned long long value = 0;

    memcpy(&value, p, sizeof(unsigned long long));

    return value;
}


IDCParcel::IDCParcel(void) {
    mDataPos = 0;
    mDataSize = 0;
    mDataCapacity = DEFAULT_PARCEL_CAPACITY;
    mData = (uint8_t *)malloc(mDataCapacity);
}

IDCParcel::~IDCParcel(void) {
}

int IDCParcel::write(const void *data, size_t len)
{
    memcpy(mData + mDataPos, data, len);
    mDataPos += len;
    mDataSize = mDataPos;

    return 0;
}

int IDCParcel::read(void *data, size_t len) const
{
    memcpy(data, mData + mDataPos, len);
    mDataPos += len;

    return (0);
}

const uint8_t *IDCParcel::data() const
{
    return mData;
}

size_t IDCParcel::dataSize() const
{
    return (mDataSize > mDataPos ? mDataSize : mDataPos);
}

int IDCParcel::setData(const uint8_t *buffer, size_t len)
{
    if (len >= mDataCapacity) {
        return (-1);
    }
    memcpy(const_cast<uint8_t *>(data()), buffer, len);
    mDataPos = 0;
    mDataSize = len;

    return (0);
}

int IDCParcel::readInt8(int8_t *pArg) const
{
    if ((mDataPos  + sizeof(int8_t)) > mDataSize) {
        return (-1);
    }
    if (pArg) {
        read(pArg, sizeof(*pArg));
    } else {
        mDataPos += sizeof(int8_t);
    }

    return 0;
}

int IDCParcel::readUint8(uint8_t *pArg) const
{
    if ((mDataPos  + sizeof(uint8_t)) > mDataSize) {
        return (-1);
    }
    if (pArg) {
        read(pArg, sizeof(*pArg));
    } else {
        mDataPos += sizeof(uint8_t);
    }

    return 0;
}

int IDCParcel::readInt16(int16_t *pArg) const
{
    if ((mDataPos  + sizeof(int16_t)) > mDataSize) {
        return (-1);
    }
    if (pArg) {
        read(pArg, sizeof(*pArg));
    } else {
        mDataPos += sizeof(int16_t);
    }

    return 0;
}

int IDCParcel::readUint16(uint16_t *pArg) const
{
    if ((mDataPos  + sizeof(uint16_t)) > mDataSize) {
        return (-1);
    }
    if (pArg) {
        read(pArg, sizeof(*pArg));;
    } else {
        mDataPos += sizeof(uint16_t);
    }

    return 0;
}

int IDCParcel::readInt32(int32_t *pArg) const
{
    if ((mDataPos  + sizeof(int32_t)) > mDataSize) {
        return (-1);
    }
    if (pArg) {
        read(pArg, sizeof(*pArg));;
    } else {
        mDataPos += sizeof(int32_t);
    }

    return 0;
}

const char *IDCParcel::readCString() const
{
    if (mDataPos < mDataSize) {
        const size_t avail = mDataSize-mDataPos;
        const char *str = reinterpret_cast<const char *>(mData+mDataPos);
        // is the string's trailing NUL within the parcel's valid bounds?
        const char *eos = reinterpret_cast<const char *>(memchr(str, 0, avail));
        if (eos) {
            const size_t len = eos - str;
            mDataPos += pad_size(len+1);
            return str;
        }
    }
    return nullptr;
}

int IDCParcel::writeInt8(int8_t val)
{
    if ((mDataPos  + sizeof(int8_t)) >= mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeUint8(uint8_t val)
{
    if ((mDataPos  + sizeof(uint8_t)) > mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeInt16(int16_t val)
{
    if ((mDataPos  + sizeof(int16_t)) > mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeUint16(uint16_t val)
{
    if ((mDataPos  + sizeof(uint16_t)) > mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeInt32(int32_t val)
{
    if ((mDataPos  + sizeof(int32_t)) > mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeUint32(uint32_t val)
{
    if ((mDataPos  + sizeof(uint32_t)) > mDataCapacity) {
        return (-1);
    }
    write(&val, sizeof(val));

    return 0;
}

int IDCParcel::writeCString(const char *str)
{
    if (!str) {
        return (-1);
    }
    int len = strlen(str);
    if ((mDataPos + pad_size(len + 1)) > mDataCapacity) {
        return (-1);
    }
    strcpy((char *)(mData + mDataPos), str);
    mData[mDataPos + len] = 0;
    mDataPos += pad_size(len + 1);
    mDataSize = mDataPos;

    return 0;
}

int IDCParcel::readParcelable(IDCParcelable *parcelable) const {
    int32_t have_parcelable = 0;
    int status = readInt32(&have_parcelable);
    if (status != 0) {
        return status;
    }
    if (!have_parcelable) {
        return (-1);
    }
    return parcelable->readFromParcel(this);
}

int IDCParcel::writeParcelable(const IDCParcelable &parcelable) {
    int status = writeInt32(1);  // parcelable is not null.
    if (status != 0) {
        return status;
    }
    return parcelable.writeToParcel(this);
}

#define MAX_IDC_EVENTS_SIZE      (8*4096)

static void *_idc_read_events(void *arg) {
    (void)arg;

    struct idc_events_data events;
    uint8_t  data[MAX_IDC_EVENTS_SIZE];


    printf("[idc] %s -> enter ++++++++\n", __func__);
    events.timeout = 1000;
    events.capacity = MAX_IDC_EVENTS_SIZE;
    events.data = data;
#if 0
    while (1) {
        events.size = 0;
        int ret = ioctl(g_dev_fd, IDC_IOC_READ_EVENTS, &events);
        if (ret < 0) {
            printf("[idc] %s -> Failed to ioctl IDC_IOC_READ_EVENTS\n", __func__);
        } else {
            uint32_t i = 0;

            while (i < events.size) {
                struct idc_event_header *event;
                idc_event_t idc_event;

                event = (struct idc_event_header *)(data + i);
#if 0
                printf("[idc] %s -> event id: %u, domain id: %d, channel id: %d, data size: %u, event size: %u, i: %u\n",
                       __func__, event->id, event->domain,  event->channel, event->data_sz, event->event_sz, i);
#endif
                if ((IDC_KM_EVENT_CONNECTED == event->id) ||
                        (IDC_KM_EVENT_DISCONNECTED == event->id)) {
                    const char *domain_name;

                    if (IDC_DOMAIN_KM_CLUSTER == event->domain) {
                        domain_name = IDC_KM_CLUSTER_DOMAIN_NAME;
                    } else if (IDC_DOMAIN_KM_ADAS == event->domain) {
                        domain_name = IDC_KM_ADAS_DOMAIN_NAME;
                    } else {
                        domain_name = IDC_KM_IVI_DOMAIN_NAME;
                    }

                    if (IDC_KM_EVENT_CONNECTED == event->id) {
                        idc_event.id = IDC_EVENT_CONNECTED;
                    } else {
                        idc_event.id = IDC_EVENT_DISCONNECTED;
                    }
                    idc_event.param1 = 0;
                    idc_event.param2 = 0;
                    IDCMonitorImpl::get()->notify(domain_name, (const char *)event->data, &idc_event);
                    if (event->channel >= 0) {
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, NULL);
                    }
                } else if (IDC_KM_EVENT_DMA_BUFFER == event->id) {
                    struct idc_dma_buf_data *data = (struct idc_dma_buf_data *)event->data;
                    printf("[idc] %s -> Receive a shared dma buffer fd: %d, size: %u\n",
                           __func__, data->data.fd, data->size);
                    idc_event.id = IDC_EVENT_DMA_BUFFER;
                    if (event->channel >= 0) {
                        idc_buffer_t  buf;

                        buf.fd = data->data.fd;
                        buf.size = data->size;
                        idc_event.param1 = (uint64_t)&buf;
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, NULL);
                        if (buf.fd >= 0) {
                            close(buf.fd);
                        }
                    }
                } else if (IDC_KM_EVENT_GFX_DMA_BUFFER == event->id) {
                    struct idc_gfx_buf_data *data = (struct idc_gfx_buf_data *)event->data;

#if 0
                    printf("[idc] %s -> Receive a shared gfx buffer fd: %d, size: %u\n",
                           __func__, data->data.fd, data->size);
#endif
                    idc_event.id = IDC_EVENT_GFX_DMA_BUFFER;
                    if (event->channel >= 0) {
                        idc_gfx_buffer_t  buf;

                        buf.width = data->meta.width;
                        buf.height = data->meta.height;
                        buf.stride = data->meta.stride;
                        buf.format = data->meta.format;
                        memcpy(buf.priv, data->meta.priv, sizeof(buf.priv));
                        buf.fd = data->data.fd;
                        idc_event.param1 = (uint64_t)&buf;
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, NULL);
                        if (buf.fd >= 0) {
                            close(buf.fd);
                        }
                    }
                } else if ((IDC_KM_EVENT_RAW_DATA == event->id) ||
                           (IDC_KM_EVENT_MESSAGE == event->id) ||
                           (IDC_KM_EVENT_PARCEL == event->id)) {
                    if (IDC_KM_EVENT_PARCEL == event->id) {
                        IDCParcel parcel;

                        idc_event.id = IDC_EVENT_PARCEL;
                        parcel.setData(event->data, event->data_sz);
                        idc_event.param1 = event->data_id;
                        idc_event.param2 = (uint64_t)&parcel;
                        printf("[idc] %s -> received parcel event, data size: %d\n", __func__,
                               (int)event->data_sz);
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, NULL);
                    } else if (IDC_KM_EVENT_MESSAGE == event->id) {
			idc_msg_header_t *msg = (idc_msg_header_t *)event->data;
                        IDCMessage idc_msg;
                        IDCMessage idc_reply_msg(msg->what);

                        IDC_LOG("[idc] %s -> received message event, what: %d\n",
                                __func__, msg->what);
			idc_msg.setData(event->data, event->data_sz);
                        idc_event.id = IDC_EVENT_MESSAGE;
                        idc_event.param1 = event->data_id;
                        idc_event.param2 = (uint64_t)&idc_msg;
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, &idc_reply_msg);
                        if (event->rsp_id > 0) {
                            struct idc_event_reply_data reply;
                            uint8_t *malloc_data = NULL;
                            int data_sz = 0;

                            reply.channel = event->channel;
                            reply.rsp_id = event->rsp_id;
                            malloc_data = (uint8_t *)malloc(idc_reply_msg.dataSize());
                            if (malloc_data) {
                                data_sz = idc_reply_msg.readData(malloc_data, idc_reply_msg.dataSize());
                            }
                            reply.data = malloc_data;
                            reply.data_sz = data_sz;
                            IDC_LOG("[idc] %s -> handle message reply, data_sz: %d\n", __func__, data_sz);
                            ret = ioctl(g_dev_fd, IDC_IOC_EVENT_REPLY, &reply);
                            if (ret < 0) {
                                IDC_LOG("[idc] %s -> reply event: %d failed\n",
                                        __func__, event->id);
                            }
                            if (malloc_data) {
                                free(malloc_data);
                                malloc_data = NULL;
                            }
                            event->rsp_id = 0;
                        }
                    } else {
                        idc_event.id = IDC_EVENT_RAW_DATA;
                        idc_event.param1 = (uint64_t)event->data;
                        idc_event.param2 = event->data_sz;
                        IDCMonitorImpl::get()->notify(event->channel, &idc_event, NULL);
                    }
                } else if (IDC_KM_EVENT_REPLY == event->id) {
#if 0
                    printf("[idc] %s -> event: %d, rsp id: %p\n",
                           __func__, (int)event->id, (void *)event->rsp_id);
#endif
                    if (event->rsp_id) {
                        idc_event_rsp_t *event_rsp = (idc_event_rsp_t *)event->rsp_id;
                        if (event_rsp->cb) {
                            if (IDC_EVENT_MESSAGE == event_rsp->event) {
                                idc_msg_header_t *msg = (idc_msg_header_t *)event->data;
                                IDCMessage reply_msg(msg->what);

                                reply_msg.setData(event->data, event->data_sz);
                                event_rsp->cb->onResult(event_rsp->extra, 0, &reply_msg);
                            } else {
                                event_rsp->cb->onResult(event_rsp->extra, 0, NULL);
                            }
                        }
                        free(event_rsp);
                    }
                }

                if ((IDC_KM_EVENT_REPLY != event->id) && (event->rsp_id > 0) && (event->channel >= 0)) {
                    struct idc_event_reply_data reply;

#if 0
                    printf("[idc] %s -> event: %d, rsp id: %p\n",
                           __func__, (int)event->id, (void *)event->rsp_id);
#endif
                    reply.channel = event->channel;
                    reply.rsp_id = event->rsp_id;
                    reply.data = NULL;
                    reply.data_sz = 0;
                    ret = ioctl(g_dev_fd, IDC_IOC_EVENT_REPLY, &reply);
                    if (ret < 0) {
                        printf("[idc] %s -> reply event: %d failed\n",
                               __func__, event->id);
                    }
                }
                i += event->event_sz;
            }
        }
    }

#endif

    return (NULL);
}

__attribute__ ((constructor)) static void _idc_init(void) {
#if 0

    g_dev_fd = open("/dev/idc", O_RDONLY | O_CLOEXEC);
    IDC_LOG("[libidc] %s -> g_dev_fd: %d\n", __func__, g_dev_fd);
    if (g_dev_fd >= 0) {
        int err;

        err = pthread_create(&g_read_events_tid, NULL, _idc_read_events, NULL);
        if (0 != err) {
            IDC_LOG("[idc] %s -> pthread_create idc read events thread  failed!\n", __func__);
        }
    }


    int fd = open("/data/idc.log", O_CREAT | O_APPEND | O_RDWR, O_CREAT);
    if (fd >= 0) {
        dup2(fd, 1);
	close(fd);
    }
#endif
}

int init_idc_device(void) {
    if (g_dev_fd >= 0) {
        return (g_dev_fd);
    }
    _idc_init();

    return (g_dev_fd);
}

int get_idc_device(void) {
    return (g_dev_fd);
}
