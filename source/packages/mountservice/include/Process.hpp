/*
*copyright (c) 2018 AutoChips Inc.
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

#ifndef PROCESS_H
#define PROCESS_H
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <signal.h>
#include <string>
#include <string.h>
#include "Utils.hpp"

#define LOG_TAG "ProcessKiller"

class Process {
public:
	static int killProcessesWithOpenFiles(const char *path, int signal);
	static int getPid(const char *s);
	static int checkSymLink(int pid, const char *path, const char *name);
	static int checkFileMaps(int pid, const char *path);
	static int checkFileMaps(int pid, const char *path, char *openFilename, size_t max);
	static int checkFileDescriptorSymLinks(int pid, const char *mountPoint);
	static int checkFileDescriptorSymLinks(int pid, const char *mountPoint, char *openFilename, size_t max);
	static int getProcessName(int pid, std::string& out_name);

private:
	static int readSymLink(const char *path, char *link, size_t max);
	static int pathMatchesMountPoint(const char *path, const char *mountPoint);
};

#endif
