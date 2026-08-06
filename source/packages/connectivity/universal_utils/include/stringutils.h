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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <list>
#include <sstream>
#include <string>
#include <vector>

namespace universal_utils {

class StringUtils
{
public:
    StringUtils();
    ~StringUtils();

    static int charToInt (char ch);
    static int stringToInt (const char *str);

    template <typename T>
    static T stringToData(const unsigned char *str)
    {
        T ret;
        std::stringstream ss;
        ss << str;
        ss >> ret;

        return ret;
    }

    static std::string intToString (int i);
    static bool isInt (char ch);
    static bool isAtoZ (char ch);
    static char toUpperCase (char ch);

    //example: "abcdefg" to abcdefg.
    static std::string parserDoubleQuotes (std::string &str);
    static int stringSplit (std::vector<std::string>& outVector, const std::string& input, const std::string& delimiters);

    //this method can retain "", example: input is ";ab;cde;fg" , delimiters is ";",  outVector size is 4, value () (ab)(cde)(fg)
    static int splitString (std::vector<std::string>& outVector, const std::string& input, const std::string& delimiters);

    static int stringToLower(const std::string &src, std::string& dest);
    static int stringToUpper(const std::string &src, std::string& dest);
    static int removeSpacesFromString(const std::string& in, std::string &out);
    static int generateRandomString(const std::string &originalSeed, size_t randomStringLen, std::string& randomString);
    static std::string convertArrayToStringAccordingSpecificDelimiters(const std::vector<int> &vec, const std::string& delimiters);

};

}
#endif // STRINGUTILS_H
