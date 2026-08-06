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

#ifndef CMESSAGE_H
#define CMESSAGE_H

#include <string>
#include <map>
#include "crawstring.h"

namespace universal_utils {
class CMessage
{
public:
    CMessage();
    CMessage(int message);
    CMessage(const CMessage& other);
    virtual ~CMessage();

    enum
    {
        EXTRA_CHAR = 1,
        EXTRA_INT,
        EXTRA_BOOL,
        EXTRA_DOUBLE,
        EXTRA_STRING,
        EXTRA_CHAR_ARRAY,
        EXTRA_UINTPTR,
    };

    CMessage& operator=(int message);
    CMessage& operator=(const CMessage& message);
    bool operator==(const CMessage& message) const;

    virtual int writeObject(char *out, unsigned int length) const;
    virtual void readObject(const char *in, unsigned length);

    void setArgRaw(const CRawString& raw);
    const CRawString& getArgRaw() const;

    CMessage& putExtra(const std::string &key, char value);
    CMessage& putExtra(const std::string &key, int value);
    CMessage& putExtra(const std::string &key, bool value);
    CMessage& putExtra(const std::string &key, const char *value);
    CMessage& putExtra(const std::string &key, const std::string &value);
    CMessage& putExtra(const std::string &key, const unsigned char *value, unsigned int length);
    CMessage& putExtra(const std::string &key, const std::map<int, int>& value);
    CMessage& putExtra(const std::string &key, uintptr_t value);
    CMessage& putExtra(const std::string &key, float value);
    CMessage& putExtra(const std::string &key, double value);

    char getCharExtra(const std::string &key, char defaultValue = 0) const;
    int getIntExtra(const std::string &key, int defaultValue = 0) const;
    float getFloatExtra(const std::string &key, float defaultValue = 0.0f) const;
    double getDoubleExtra(const std::string &key, double defaultValue = 0.0f) const;
    bool getBoolExtra(const std::string &key, bool defaultValue = false) const;
    std::string getStringExtra(const std::string &key, const std::string defaultValue = "") const;
    const unsigned char* getArrayExtra(const std::string &key, unsigned int *arraylength, const unsigned char *defaultValue = NULL) const;
    int getMapIntIntExtra(const std::string &key, std::map<int, int> &value) const;
    uintptr_t getUintptrExtra(const std::string &key, uintptr_t defaultValue) const;

    int size() const;

    int what;
    int arg1;
    int arg2;

    long long when;

protected:
    CRawString argRaw;

private:
    int findValue(int type, const std::string &key,  void *value, int *valueLen) const;
    char getChar(const unsigned char *p) const;
    unsigned int getInt(const unsigned char *p) const;
    double getDouble(const unsigned char *p) const;
    uintptr_t getUintptr(const unsigned char *p) const;
};
}
#endif // CMESSAGE_H
