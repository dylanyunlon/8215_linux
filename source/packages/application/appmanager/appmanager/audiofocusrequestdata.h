/*
* Copyright (c) 2016 AutoChips Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef AUDIOFOCUSREQUESTDATA_H
#define AUDIOFOCUSREQUESTDATA_H


class AudioFocusRequestData
{
private:
    unsigned int m_data;
public:
    explicit AudioFocusRequestData(unsigned int data);
    AudioFocusRequestData() = delete;

    unsigned int getOutput();
    unsigned int getFocusType();
    unsigned int getStreamType();

    static const char* decodeOutput(unsigned int output);
    static const char* decodeFocusType(unsigned int focusType);
    static const char* decodeStreamType(unsigned int streamtype);
};

#endif //AUDIOFOCUSREQUESTDATA_H