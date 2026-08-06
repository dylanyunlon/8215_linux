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
#include "Process.hpp"

int Process::readSymLink(const char *path, char *link, size_t max) {
	struct stat s;
	int length;

	if (lstat(path, &s) < 0)
		return 0;
		if ((s.st_mode & S_IFMT) != S_IFLNK)
			return 0;

		// we have a symlink
		length = readlink(path, link, max- 1);
		if (length <= 0)
			return 0;
		link[length] = 0;
		return 1;
}

int Process::pathMatchesMountPoint(const char* path, const char* mountPoint) {
	int length = strlen(mountPoint);
	if (length > 1 && strncmp(path, mountPoint, length) == 0) {
		// we need to do extra checking if mountPoint does not end in a '/'
		if (mountPoint[length - 1] == '/')
			return 1;
		// if mountPoint does not have a trailing slash, we need to make sure
		// there is one in the path to avoid partial matches.
		return (path[length] == 0 || path[length] == '/');
	}

	return 0;
}

int Process::getProcessName(int pid, std::string& out_name) {
	char process_names[128] = "";
	char buff_num[8] = "";
	char line[1024];

	out_name.clear();

	strlcat(process_names, "/proc/", sizeof(process_names));
	sprintf(buff_num, "%d", pid);
	strlcat(process_names, buff_num, sizeof(process_names));
	strlcat(process_names, "/cmdline", sizeof(process_names));

	FILE* fp = fopen(process_names, "r");
	if (!fp) {
		atc_sysloge ("Failed to fopen(%s)!\r\n", process_names);
		return -errno;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		out_name.append(line);
		memset(line, 0x0, sizeof(line));
	}

	fclose(fp);
	return 0;
}

int Process::checkFileDescriptorSymLinks(int pid, const char *mountPoint) {
	return checkFileDescriptorSymLinks(pid, mountPoint, NULL, 0);
}

int Process::checkFileDescriptorSymLinks(int pid, const char *mountPoint, char *openFilename, size_t max) {
	// compute path to process's directory of open files
	char    path[PATH_MAX];
	snprintf(path, sizeof(path), "/proc/%d/fd", pid);
	DIR *dir = opendir(path);
	if (!dir)
		return 0;

	// remember length of the path
	int parent_length = strlen(path);
	// append a trailing '/'
	path[parent_length++] = '/';

	struct dirent* de;
	while ((de = readdir(dir))) {
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")
			|| strlen(de->d_name) + parent_length + 1 >= PATH_MAX)
			continue;

		// append the file name, after truncating to parent directory
		path[parent_length] = 0;
		strlcat(path, de->d_name, PATH_MAX);

		char link[PATH_MAX];

		if (readSymLink(path, link, sizeof(link)) && pathMatchesMountPoint(link, mountPoint)) {
			if (openFilename) {
				memset(openFilename, 0, max);
				strlcpy(openFilename, link, max);
			}
			closedir(dir);
			return 1;
		}
	}

	closedir(dir);
	return 0;
}

int Process::checkFileMaps(int pid, const char *mountPoint) {
	return checkFileMaps(pid, mountPoint, NULL, 0);
}

int Process::checkFileMaps(int pid, const char *mountPoint, char *openFilename, size_t max) {
	FILE *file;
	char buffer[PATH_MAX + 100];

	snprintf(buffer, sizeof(buffer), "/proc/%d/maps", pid);
	file = fopen(buffer, "re");
	if (!file)
		return 0;

	while (fgets(buffer, sizeof(buffer), file)) {
		// skip to the path
		const char* path = strchr(buffer, '/');
		if (path && pathMatchesMountPoint(path, mountPoint)) {
			if (openFilename) {
				memset(openFilename, 0, max);
				strlcpy(openFilename, path, max);
			}
			fclose(file);
			return 1;
		}
	}

	fclose(file);
	return 0;
}

int Process::checkSymLink(int pid, const char *mountPoint, const char *name) {
	char    path[PATH_MAX];
	char    link[PATH_MAX];

	snprintf(path, sizeof(path), "/proc/%d/%s", pid, name);
	if (readSymLink(path, link, sizeof(link)) && pathMatchesMountPoint(link, mountPoint))
		return 1;
	return 0;
}

int Process::getPid(const char *s) {
	int result = 0;
	while (*s) {
		if (!isdigit(*s)) return -1;
			result = 10 * result + (*s++ - '0');
	}
	return result;
}

/*
* Hunt down processes that have files open at the given mount point.
*/
int Process::killProcessesWithOpenFiles(const char *path, int signal) {
	int count = 0;
	DIR* dir;
	struct dirent* de;

	if (!(dir = opendir("/proc"))) {
		atc_sysloge ("opendir failed (%s)!\r\n", strerror(errno));
		return count;
	}

	while ((de = readdir(dir))) {
		int pid = getPid(de->d_name);
		if (pid == -1) {
			continue;
		}

		std::string name;
		getProcessName(pid, name);

		char openfile[PATH_MAX];

		if (checkFileDescriptorSymLinks(pid, path, openfile, sizeof(openfile))) {
			atc_sysloge ("Process %s (%d) has open file %s!\r\n", name.c_str(), pid, openfile);
		} else if (checkFileMaps(pid, path, openfile, sizeof(openfile))) {
			atc_sysloge ("Process %s (%d) has open filemap for %s!\r\n", name.c_str(), pid, openfile);
		} else if (checkSymLink(pid, path, "cwd")) {
			atc_sysloge ("Process %s (%d) has cwd within %s!\r\n", name.c_str(), pid, path);
		} else if (checkSymLink(pid, path, "root")) {
			atc_sysloge ("Process %s (%d) has chroot within %s!\r\n", name.c_str(), pid, path);
		} else if (checkSymLink(pid, path, "exe")) {
			atc_sysloge ("Process %s (%d) has executable path within %s!\r\n", name.c_str(), pid, path);
		} else {
			continue;
		}

		if (signal != 0) {
			atc_sysloge ("Sending %s to process (%d, %s)!\r\n", strsignal(signal), pid, name.c_str());
			kill(pid, signal);
			count++;
		}
	}
	closedir(dir);
	return count;
}


