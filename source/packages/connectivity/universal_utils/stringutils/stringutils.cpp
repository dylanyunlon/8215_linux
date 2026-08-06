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

#include "stringutils.h"

#include <cctype>
#include <random>
#include <sstream>
#include <stdio.h>
#include <string.h>

namespace universal_utils {

StringUtils::StringUtils()
{

}

StringUtils::~StringUtils()
{

}

int StringUtils::charToInt (char ch)
{
    int ret = -1;
    if (isInt(ch)) {
        ret = ch-'0';
    }
    return ret;
}

int StringUtils::stringToInt (const char *str)
{
    int ret = 0;
    std::stringstream ss;
    ss<<str;
    ss>>ret;
    return ret;
}

std::string StringUtils::intToString (int i)
{
    std::string ret;
    std::stringstream ss;
    ss<<i;
    ss>>ret;
    return ret;
}

bool StringUtils::isInt (char ch)
{
    bool ret = false;

    if (ch >= '0' && ch <= '9') {
        ret = true;
    }

    return ret;
}

//example: "abcdefg" to abcdefg.
std::string StringUtils::parserDoubleQuotes (std::string &str)
{
    std::string retStr = "";
    char *pstr = NULL;
    pstr = strtok(const_cast<char*>(str.c_str()), "\"");
    if (pstr != NULL) {
        retStr = pstr;
    }

    return retStr;
}

int StringUtils::stringSplit (std::vector<std::string>& outVector, const std::string& input, const std::string& delimiters)
{
    char *pstr = NULL;
    int ret = 0;

    pstr = strtok(const_cast<char*>(input.c_str()), delimiters.c_str());
    while (pstr != NULL) {
        outVector.push_back(std::string(pstr));
        pstr = strtok(NULL, delimiters.c_str());
        ret ++;
    }

    return ret;
}

//this method can retain "", example: input is ";ab;cde;fg" , delimiters is ";",  outVector size is 4, value () (ab)(cde)(fg)
int StringUtils::splitString (std::vector<std::string>& outVector, const std::string& input, const std::string& delimiters)
{
    std::string::size_type pos;
    std::string str = input + delimiters;
    unsigned int size = str.size();

    for (unsigned int i = 0; i < size; i++) {
        pos = str.find(delimiters, i);
        if (pos < size) {
            std::string value = str.substr(i, pos - i);
            outVector.push_back(value);
            i = pos + delimiters.size() - 1;
        }
    }

    return 0;
}

bool StringUtils::isAtoZ (char ch)
{
    bool ret = false;

    if (ch >= 'A' && ch <= 'Z') {
        ret = true;
    }

    return ret;
}

char StringUtils::toUpperCase (char ch)
{
    char ret = ch;
    if (ch >= 'a' && ch <= 'z') {
        ret = ch - ('a' - 'A');
    }

    return ret;
}

int StringUtils::stringToLower(const std::string &src, std::string& dest)
{
    size_t len = src.size();
    dest.assign("");

    for (size_t i = 0; i < len; i++) {
        if (src[i] <= 'Z' && src[i] >= 'A') {
            dest += tolower(src[i]);
        } else {
            dest += src[i];
        }
    }

    return 0;
}

int StringUtils::stringToUpper(const std::string &src, std::string& dest)
{
    size_t len = src.size();
    dest.assign("");

    for (size_t i = 0; i < len; i++) {
        if (src[i] <= 'z' && src[i] >= 'a') {
            dest += toUpperCase(src[i]);
        } else {
            dest += src[i];
        }
    }

    return 0;
}

int StringUtils::removeSpacesFromString(const std::string& in, std::string &out)
{
    out.clear();

    for (char c : in) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            out.push_back(c);
        }
    }

    return 0;
}

int StringUtils::generateRandomString(const std::string &originalSeed, size_t randomStringLen, std::string& randomString)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, originalSeed.size() -1);

    randomString = std::string(randomStringLen, ' ');

    for (size_t i = 0; i < randomStringLen; i++) {
        randomString[i] = originalSeed[dis(gen)];
    }

    return 0;
}

std::string StringUtils::convertArrayToStringAccordingSpecificDelimiters(
        const std::vector<int> &vec, const std::string& delimiters)
{
    std::ostringstream oss;

    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) {
            oss << delimiters;
        }
        oss << vec[i];
    }

    return oss.str();
}

}
