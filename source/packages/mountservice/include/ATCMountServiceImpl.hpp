/*
copyright (c) 2018 AutoChips Inc.
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
#ifndef ATCMOUNTSERVICIMPL_H
#define ATCMOUNTSERVICIMPL_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Utils.hpp"

class ATCMountServiceImpl {
public:
	ATCMountServiceImpl();
	~ATCMountServiceImpl();
#if 0
	void nvStoreRead(const std::shared_ptr<CommonAPI::ClientId> _client, int32_t _id, nvStoreReadReply_t _reply);
	void nvStoreWrite(const std::shared_ptr<CommonAPI::ClientId> _client, int32_t _id, int32_t _mode, nvStoreWriteReply_t _reply);
	void nvStoreFlush(const std::shared_ptr<CommonAPI::ClientId> _client, int32_t _id, nvStoreFlushReply_t _reply);
	void sendMessage(const std::string _action, const std::string _mountPoint);
	void sendMessage2LMS(const std::string _action, const std::string _mountPoint, const std::string _uuid);
	void enablePorcessExternalStorage(const std::shared_ptr<CommonAPI::ClientId> _client, enablePorcessExternalStorageReply_t _reply);
	void disablePorcessExternalStorage(const std::shared_ptr<CommonAPI::ClientId> _client, disablePorcessExternalStorageReply_t _reply);
	void formatExternalStorage(const std::shared_ptr<CommonAPI::ClientId> _client, std::string _source, std::string _fsType, uint32_t _numSectors, formatExternalStorageReply_t _reply);
#endif
};

#endif

