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

#ifndef _IDCINFO_H_
#define _IDCINFO_H_

#include <string.h>

class IDCInfo : public IDCParcelable {
public:
    IDCInfo(void) {
        m_name[0] = 0;
        m_phone[0] = 0;
        strcpy(m_city, "Hefei");
        m_temperature = 0;
        m_age = 0;
    }

    ~IDCInfo(void) {
    }

    void setName(const char *name) {
        strcpy(m_name, name);
    }

    void getName(char *name) const {
        strcpy(name, m_name);
    }

    void setPhone(const char *phone) {
        strcpy(m_phone, phone);
    }

    void getPhone(char *phone) const {
        strcpy(phone, m_phone);
    }

    void setCity(const char *city) {
        strcpy(m_city, city);
    }

    void getCity(char *city) const {
        strcpy(city, m_city);
    }

    void setTemperature(int16_t temperature) {
        m_temperature = temperature;
    }

    int16_t getTemperature(void) const {
        return (m_temperature);
    }

    void setAge(uint8_t age) {
        m_age = age;
    }

    uint8_t getAge(void) const {
        return (m_age);
    }

protected:
    int writeToParcel(IDCParcel *parcel) const {
        parcel->writeCString(m_name);
        parcel->writeCString(m_phone);
        parcel->writeCString(m_city);
        parcel->writeInt16(m_temperature);
        parcel->writeUint8(m_age);

        return (0);
    }

    int readFromParcel(const IDCParcel *parcel) {
        strcpy(m_name, parcel->readCString());
        strcpy(m_phone, parcel->readCString());
        strcpy(m_city, parcel->readCString());
        parcel->readInt16(&m_temperature);
        parcel->readUint8(&m_age);

        return (0);
    }

private:
    char m_name[20];
    char m_phone[12];
    char m_city[20];
    int16_t m_temperature;
    uint8_t m_age;
};

#endif

