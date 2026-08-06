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

#include <string.h>
#include <utility>
#include "cmessage.h"
#include "stringutils.h"

namespace universal_utils {
CMessage::CMessage() :
    what(0),
    arg1(0),
    arg2(0),
    when(0)
{


}

CMessage::CMessage(int message) :
    what(message),
    arg1(0),
    arg2(0),
    when(0)
{


}

CMessage::CMessage(const CMessage& other)
{
    what = other.what;
    arg1 = other.arg1;
    arg2 = other.arg2;

    when = other.when;

    argRaw = other.argRaw;
}

CMessage::~CMessage()
{

}

CMessage& CMessage::operator=(int message)
{
    what = message;

    arg1 = 0;
    arg2 = 0;

    when = 0;

    argRaw.clear();

    return *this;
}

CMessage& CMessage::operator=(const CMessage& message)
{
    what = message.what;
    arg1 = message.arg1;
    arg2 = message.arg2;

    when = message.when;

    argRaw = message.argRaw;

    return *this;
}

bool CMessage::operator==(const CMessage& message) const
{
    return (what == message.what);
}

int CMessage::writeObject(char *out, unsigned int length) const
{
    if (NULL == out) {
        return -1;
    }

    unsigned int outIndex = 0;
    unsigned int argRawLength = 0;
    unsigned int messageLength = sizeof(what) + sizeof(arg1) + sizeof(arg2) + sizeof(argRawLength) + argRaw.getStrLen();
    if (messageLength > length) {
        return -1;
    }

    memcpy(out, &what, sizeof(what));
    outIndex += sizeof(what);

    memcpy(out + outIndex, &arg1, sizeof(arg1));
    outIndex += sizeof(arg1);

    memcpy(out + outIndex, &arg2, sizeof(arg2));
    outIndex += sizeof(arg2);

    argRawLength = argRaw.getStrLen();
    if (argRawLength > 0) {
        memcpy(out + outIndex, &argRawLength, sizeof(argRawLength));
        outIndex += sizeof(argRawLength);

        memcpy(out + outIndex, argRaw.getStr(), argRaw.getStrLen());

    }

    return messageLength;
}

void CMessage::readObject(const char *in, unsigned int length)
{
    if (NULL == in) {
        return;
    }

    unsigned int inIndex = 0;
    unsigned int argRawlength = 0;

    if (length >= sizeof(what)) {
        memcpy(&what, in, sizeof(what));
        inIndex += sizeof(what);
        length -= sizeof(what);
    } else {
        return;
    }

    if (length >= sizeof(arg1)) {
        memcpy(&arg1, in + inIndex, sizeof(arg1));
        inIndex += sizeof(arg1);
        length -= sizeof(arg1);
    } else {
        return;

    }

    if (length >= sizeof(arg2)) {
        memcpy(&arg2, in + inIndex, sizeof(arg2));
        inIndex += sizeof(arg2);
        length -= sizeof(arg2);
    } else {
        return;
    }

    if (length >= sizeof(argRawlength)) {
        memcpy(&argRawlength, in + inIndex, sizeof(argRawlength));
        inIndex += sizeof(argRawlength);
        length -= sizeof(argRawlength);
    } else {
        return;
    }

    if (length >= argRawlength) {
        argRaw.clear();
        argRaw.setStr(in + inIndex, argRawlength, 0);
    }

}

void CMessage::setArgRaw(const CRawString& raw)
{
    argRaw = raw;
}

const CRawString& CMessage::getArgRaw() const
{
    return argRaw;
}


CMessage& CMessage::putExtra(const std::string &key, char value)
{
    unsigned int type = EXTRA_CHAR;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&value, sizeof(char));

    return *this;
}

/*use bool as char*/
CMessage& CMessage::putExtra(const std::string &key, bool value)
{
    unsigned int type = EXTRA_CHAR;
    unsigned int size = 0;

    char charValue = (char)value;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&charValue, sizeof(char));

    return *this;
}




CMessage& CMessage::putExtra(const std::string &key, int value)
{
    unsigned int type = EXTRA_INT;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&value, sizeof(int));

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, const char *value)
{
    if (value != NULL) {
        std::string valueStr = value;
        return putExtra(key, valueStr);
    }

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, const std::string &value)
{
    unsigned int type = EXTRA_STRING;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value len*/
    size = value.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(value.c_str(), size);

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, const unsigned char *value, unsigned int length)
{
    unsigned int type = EXTRA_CHAR_ARRAY;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value len*/
    argRaw.addTail(&length, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(value, length);

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, const std::map<int, int>& value)
{
    int length = value.size();
    std::string strValue = "";
    int mapKey = 0;
    int mapValue = 0;

    int i = 0;
    std::map<int, int>::const_iterator iter = value.begin();
    for (; iter != value.end(); iter++, i++) {
        mapKey = iter->first;
        mapValue = iter->second;
        // Encapsulating the contents of map into strValue, spliting keys and values with "|"
        // and sets with ","
        strValue += StringUtils::intToString(mapKey) + "|" +
            StringUtils::intToString(mapValue);
        if (i < length - 1) {
            strValue += ",";
        }
    }

    putExtra(key, strValue);

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, uintptr_t value)
{
    unsigned int type = EXTRA_UINTPTR;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&value, sizeof(uintptr_t));

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, float value)
{
    unsigned int type = EXTRA_DOUBLE;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&value, sizeof(float));

    return *this;
}

CMessage& CMessage::putExtra(const std::string &key, double value)
{
    unsigned int type = EXTRA_DOUBLE;
    unsigned int size = 0;

    /*key len*/
    size = key.size();
    argRaw.addTail(&size, sizeof(unsigned int));

    /*key*/
    argRaw.addTail(key.c_str(), key.size());

    /*type*/
    argRaw.addTail(&type, sizeof(unsigned int));

    /*value*/
    argRaw.addTail(&value, sizeof(double));

    return *this;
}

char CMessage::getCharExtra(const std::string &key, char defaultValue) const
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

int CMessage::getIntExtra(const std::string &key, int defaultValue) const
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

float CMessage::getFloatExtra(const std::string &key, float defaultValue) const
{
    float value = 0;
    int valuelen = sizeof(float);

    int ret = findValue(EXTRA_DOUBLE, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

double CMessage::getDoubleExtra(const std::string &key, double defaultValue) const
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

bool CMessage::getBoolExtra(const std::string &key, bool defaultValue) const
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

std::string CMessage::getStringExtra(const std::string &key, const std::string defaultValue) const
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

const unsigned char* CMessage::getArrayExtra(const std::string &key, unsigned int *arraylength, const unsigned char *defaultValue) const
{
    unsigned char* value = NULL;
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

int CMessage::getMapIntIntExtra(const std::string &key, std::map<int, int> &value) const
{
    value.clear();

    std::string strValue = "";

    strValue = getStringExtra(key, "");
    if (strValue == "") {
        return -1;
    }

    // Divide the strValue into substrings according to ","
    std::vector<std::string> outString;
    outString.clear();
    StringUtils::splitString (outString, strValue, ",");

    std::vector<std::string> keyValueStr;
    for (int i = 0; i < outString.size(); i++) {
        keyValueStr.clear();
        // Divide the outString[i] into substrings according to "|"
        StringUtils::splitString(keyValueStr, outString[i], "|");

        // get the key and value of the map
        int mapKey = StringUtils::stringToInt (keyValueStr[0].c_str());
        int mapValue = StringUtils::stringToInt (keyValueStr[1].c_str());
        value.insert(std::pair<int, int>(mapKey, mapValue));
    }

    return 0;
}

uintptr_t CMessage::getUintptrExtra(const std::string &key, uintptr_t defaultValue) const
{
    uintptr_t value = 0;
    int valuelen = sizeof(uintptr_t);

    int ret = findValue(EXTRA_UINTPTR, key, (void*)&value, &valuelen);
    if (ret < 0) {
        return defaultValue;
    } else {
        return value;
    }
}

int CMessage::size() const
{
    return sizeof(what) + sizeof(arg1) + sizeof(arg2) + sizeof(when) + argRaw.getStrLen();
}

int CMessage::findValue(int type, const std::string &key,  void *value, int *valueLen) const
{
    unsigned int keylen = 0;
    int tmpType;
    unsigned int remain = argRaw.getStrLen();
    std::string tmpKey;

    const unsigned char *data = argRaw.getStr();

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

                case EXTRA_UINTPTR: {
                    *(uintptr_t*)value = getUintptr(data);
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
                }
                break;

                case EXTRA_INT: {
                    data += sizeof(int);
                    remain -= sizeof(int);
                }
                break;

                case EXTRA_DOUBLE: {
                    data += sizeof(double);
                    remain -= sizeof(double);
                }
                break;

                case EXTRA_STRING: {
                    unsigned int valueStringLen;

                    valueStringLen = getInt(data);
                    data += sizeof(unsigned int);
                    remain -= sizeof(unsigned int);

                    data += valueStringLen;
                    remain -= valueStringLen;
                }
                break;

                case EXTRA_CHAR_ARRAY: {
                    unsigned int valueArrayLen;

                    valueArrayLen = getInt(data);
                    data += sizeof(unsigned int);
                    remain -= sizeof(unsigned int);

                    data += valueArrayLen;
                    remain -= valueArrayLen;
                }
                break;

                case EXTRA_UINTPTR: {
                    data += sizeof(uintptr_t);
                    remain -= sizeof(uintptr_t);
                }
                break;
            }
        }
    }

    return -1;
}



char CMessage::getChar(const unsigned char *p) const
{
    char value = 0;

    memcpy(&value, p, sizeof(char));

    return value;
}


unsigned int CMessage::getInt(const unsigned char *p) const
{
    unsigned int value = 0;

    memcpy(&value, p, sizeof(unsigned int));

    return value;
}

double CMessage::getDouble(const unsigned char *p) const
{
    double value = 0;

    memcpy(&value, p, sizeof(double));

    return value;
}

uintptr_t CMessage::getUintptr(const unsigned char *p) const
{
    uintptr_t value = 0;

    memcpy(&value, p, sizeof(uintptr_t));

    return value;
}
}
