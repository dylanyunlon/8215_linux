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

#ifndef _IDC_COMMON_H
#define _IDC_COMMON_H

#include <stdint.h>
#include <string>

#define IDC_UNUSED(x)               ((void *)x)

#define IDC_CLUSTER_DOMAIN_NAME     "cluster"
#define IDC_ADAS_DOMAIN_NAME        "adas"
#define IDC_IVI_DOMAIN_NAME         "ivi"

#define IDC_EVENT_CONNECTED         0
#define IDC_EVENT_DISCONNECTED      1
#define IDC_EVENT_RAW_DATA          2
#define IDC_EVENT_MESSAGE           3
#define IDC_EVENT_DMA_BUFFER        4
#define IDC_EVENT_GFX_DMA_BUFFER    5
#define IDC_EVENT_PARCEL            6

typedef struct idc_event {
    uint32_t id;
    uint64_t param1;
    uint64_t param2;
} idc_event_t;

typedef struct idc_msg_header {
    int what;
    int ret;
    long long when;
} idc_msg_header_t;

typedef struct idc_buffer {
    int fd;
    int size;

    int fence;
} idc_buffer_t;

typedef struct idc_gfx_buffer {
    uint32_t  width;
    uint32_t  height;
    uint32_t  stride;
    uint32_t  format;

    int       fd;
    int       fence;

    uint32_t  priv[8];
} idc_gfx_buffer_t;

class RawString;
class IDCMessage;

class IDCMessage {
public:
    IDCMessage();
    IDCMessage(int message);
    IDCMessage(const IDCMessage& other);
    virtual ~IDCMessage();

    enum
    {
        EXTRA_CHAR = 1,
        EXTRA_INT,
        EXTRA_DOUBLE,
        EXTRA_FLOAT,
        EXTRA_UNSIGNED_LONG_LONG,
        EXTRA_BOOL,
        EXTRA_STRING,
        EXTRA_CHAR_ARRAY
    };

    IDCMessage& operator=(int message);
    IDCMessage& operator=(const IDCMessage& message);
    bool operator==(const IDCMessage& message) const;

    IDCMessage& putExtra(const std::string &key, char value);
    IDCMessage& putExtra(const std::string &key, int value);
    IDCMessage& putExtra(const std::string &key, double value);
    IDCMessage& putExtra(const std::string &key, float value);
    IDCMessage& putExtra(const std::string &key, unsigned long long value);
    IDCMessage& putExtra(const std::string &key, bool value);
    IDCMessage& putExtra(const std::string &key, const std::string &value);
    IDCMessage& putExtra(const std::string &key, const unsigned char *value, unsigned int length);

    char getCharExtra(const std::string &key, char defaultValue = 0) const;
    int getIntExtra(const std::string &key, int defaultValue = 0) const;
    double getDoubleExtra(const std::string &key, double defaultValue = 0) const;
    float getFloatExtra(const std::string &key, float defaultValue = 0) const;
    unsigned long long getUnsignedLongLongExtra(const std::string &key, unsigned long long defaultValue = 0) const;
    bool getBoolExtra(const std::string &key, bool defaultValue = false) const;
    std::string getStringExtra(const std::string &key, const std::string defaultValue = "") const;
    const unsigned char* getArrayExtra(const std::string &key, unsigned int *arraylength, const unsigned char *defaultValue = NULL) const;

    int size() const;

    int  readData(uint8_t *data, int size);
    int  dataSize() const;
    int  setData(uint8_t *data, int size);

protected:
    RawString *m_pData;

private:
    int findValue(int type, const std::string &key,  void *value, int *valueLen) const;
    char getChar(const unsigned char *p) const;
    unsigned int getInt(const unsigned char *p) const;
    double getDouble(const unsigned char *p) const;
    float getFloat(const unsigned char *p) const;
    unsigned long long getLongLong(const unsigned char *p) const;

public:
    idc_msg_header_t m_msg;
};

class IDCParcel;

// Abstract interface of all parcelables.
class IDCParcelable {
public:
    virtual ~IDCParcelable() = default;

    IDCParcelable() = default;
    IDCParcelable(const IDCParcelable &) = default;

    // Write |this| parcelable to the given |parcel|.  Keep in mind that
    // implementations of writeToParcel must be manually kept in sync
    // with readFromParcel and the Java equivalent versions of these methods.
    //
    // 0 on success and an appropriate error otherwise.
    virtual int writeToParcel(IDCParcel *parcel) const = 0;

    // Read data from the given |parcel| into |this|.  After readFromParcel
    // completes, |this| should have equivalent state to the object that
    // wrote itself to the parcel.
    //
    // 0 on success and an appropriate error otherwise.
    virtual int readFromParcel(const IDCParcel *parcel) = 0;
};  // class Parcelable

class IDCParcel {
public:
    IDCParcel();
    ~IDCParcel();

    const uint8_t      *data() const;
    size_t              dataSize() const;
    size_t              dataAvail() const;
    size_t              dataPosition() const;
    size_t              dataCapacity() const;

    int                 setDataSize(size_t size);
    void                setDataPosition(size_t pos) const;
    int                 setDataCapacity(size_t size);

    int                 setData(const uint8_t *buffer, size_t len);

    int                 writeInt8(int8_t val);
    int                 writeUint8(uint8_t val);
    int                 writeInt16(int16_t val);
    int                 writeUint16(uint16_t val);
    int                 writeInt32(int32_t val);
    int                 writeUint32(uint32_t val);
    int                 writeInt64(int64_t val);
    int                 writeUint64(uint64_t val);
    int                 writeFloat(float val);
    int                 writeDouble(double val);
    int                 writeCString(const char *str);
    int                 writeString16(const char16_t *str, size_t len);

    int                 writeParcelable(const IDCParcelable &parcelable);

    int                 readInt8(int8_t *pArg) const;
    int                 readUint8(uint8_t *pArg) const;
    int                 readInt16(int16_t *pArg) const;
    int                 readUint16(uint16_t *pArg) const;
    int32_t             readInt32() const;
    int                 readInt32(int32_t *pArg) const;
    uint32_t            readUint32() const;
    int                 readUint32(uint32_t *pArg) const;
    int64_t             readInt64() const;
    int                 readInt64(int64_t *pArg) const;
    uint64_t            readUint64() const;
    int                 readUint64(uint64_t *pArg) const;
    float               readFloat() const;
    int                 readFloat(float *pArg) const;
    double              readDouble() const;
    int                 readDouble(double *pArg) const;
    const char         *readCString() const;

    int                 readParcelable(IDCParcelable *parcelable) const;

protected:
    int                 write(const void *data, size_t len);
    int                 read(void *data, size_t len) const;

private:
    uint8_t            *mData;
    size_t              mDataSize;
    size_t              mDataCapacity;
    mutable size_t      mDataPos;
};

int idc_read_events(uint32_t timeout);

#endif //_IDC_COMMON_H
