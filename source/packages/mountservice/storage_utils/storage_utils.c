#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "storage_utils.h"

typedef struct {
	char *source;
	char *target;
	char *fs_type;
	char *options;
} MountEntry;

typedef struct {
	MountEntry *items;
	size_t size;
	size_t cap;
} MountEntryVec;

static void free_mount_entry(MountEntry *entry) {
	if (entry == NULL) {
		return;
	}
	free(entry->source);
	free(entry->target);
	free(entry->fs_type);
	free(entry->options);
	entry->source = NULL;
	entry->target = NULL;
	entry->fs_type = NULL;
	entry->options = NULL;
}

static void free_mount_vec(MountEntryVec *vec) {
	size_t i;
	if (vec == NULL) {
		return;
	}
	for (i = 0; i < vec->size; ++i) {
		free_mount_entry(&vec->items[i]);
	}
	free(vec->items);
	vec->items = NULL;
	vec->size = 0;
	vec->cap = 0;
}

static int push_mount_entry(MountEntryVec *vec, const MountEntry *entry) {
	MountEntry *new_items;
	if (vec == NULL || entry == NULL) {
		return EINVAL;
	}
	if (vec->size == vec->cap) {
		size_t new_cap = (vec->cap == 0) ? 16 : vec->cap * 2;
		new_items = (MountEntry *)realloc(vec->items, new_cap * sizeof(MountEntry));
		if (new_items == NULL) {
			return ENOMEM;
		}
		vec->items = new_items;
		vec->cap = new_cap;
	}
	vec->items[vec->size] = *entry;
	vec->size++;
	return 0;
}

static int run_command(const char *command) {
	int rc;
	int exit_code;
	if (command == NULL) {
		return EINVAL;
	}
	rc = system(command);
	if (rc == -1) {
		return (errno == 0) ? EIO : errno;
	}
	if (!WIFEXITED(rc)) {
		return EIO;
	}
	exit_code = WEXITSTATUS(rc);
	return (exit_code == 0) ? 0 : EIO;
}

static char *shell_escape(const char *value) {
	size_t i;
	size_t n;
	size_t out_len = 2;
	char *out;
	size_t pos = 0;
	if (value == NULL) {
		return NULL;
	}
	n = strlen(value);
	for (i = 0; i < n; ++i) {
		out_len += (value[i] == '\'') ? 4 : 1;
	}
	out = (char *)malloc(out_len + 1);
	if (out == NULL) {
		return NULL;
	}
	out[pos++] = '\'';
	for (i = 0; i < n; ++i) {
		if (value[i] == '\'') {
			out[pos++] = '\'';
			out[pos++] = '\\';
			out[pos++] = '\'';
			out[pos++] = '\'';
		} else {
			out[pos++] = value[i];
		}
	}
	out[pos++] = '\'';
	out[pos] = '\0';
	return out;
}

static char *unescape_mount_field(const char *field) {
	size_t i;
	size_t n;
	char *out;
	size_t pos = 0;
	if (field == NULL) {
		return NULL;
	}
	n = strlen(field);
	out = (char *)malloc(n + 1);
	if (out == NULL) {
		return NULL;
	}
	for (i = 0; i < n; ++i) {
		if (field[i] == '\\' && i + 3 < n &&
				isdigit((unsigned char)field[i + 1]) &&
				isdigit((unsigned char)field[i + 2]) &&
				isdigit((unsigned char)field[i + 3])) {
			int value = (field[i + 1] - '0') * 64 + (field[i + 2] - '0') * 8 + (field[i + 3] - '0');
			out[pos++] = (char)value;
			i += 3;
		} else {
			out[pos++] = field[i];
		}
	}
	out[pos] = '\0';
	return out;
}

static char *normalize_path(const char *path) {
	size_t n;
	char *out;
	if (path == NULL) {
		return NULL;
	}
	n = strlen(path);
	out = strdup(path);
	if (out == NULL) {
		return NULL;
	}
	while (n > 1 && out[n - 1] == '/') {
		out[n - 1] = '\0';
		n--;
	}
	return out;
}

static int path_eq(const char *a, const char *b) {
	char *na = normalize_path(a);
	char *nb = normalize_path(b);
	int same = 0;
	if (na != NULL && nb != NULL && strcmp(na, nb) == 0) {
		same = 1;
	}
	free(na);
	free(nb);
	return same;
}

static int parse_mountinfo_line(const char *line, MountEntry *entry) {
	char *copy = NULL;
	char *sep = NULL;
	char *pre = NULL;
	char *post = NULL;
	char *save = NULL;
	char *token = NULL;
	int i;
	char *mount_point = NULL;
	char *mount_options = NULL;
	char *fs_type = NULL;
	char *mount_source = NULL;
	char *super_options = NULL;

	if (line == NULL || entry == NULL) {
		return EINVAL;
	}
	memset(entry, 0, sizeof(*entry));

	copy = strdup(line);
	if (copy == NULL) {
		return ENOMEM;
	}
	sep = strstr(copy, " - ");
	if (sep == NULL) {
		free(copy);
		return EINVAL;
	}
	*sep = '\0';
	pre = copy;
	post = sep + 3;

	save = NULL;
	for (i = 0; i < 6; ++i) {
		token = strtok_r((i == 0) ? pre : NULL, " ", &save);
		if (token == NULL) {
			free(copy);
			return EINVAL;
		}
		if (i == 4) {
			mount_point = token;
		} else if (i == 5) {
			mount_options = token;
		}
	}

	save = NULL;
	fs_type = strtok_r(post, " ", &save);
	mount_source = strtok_r(NULL, " ", &save);
	super_options = strtok_r(NULL, " ", &save);
	if (fs_type == NULL || mount_source == NULL || super_options == NULL) {
		free(copy);
		return EINVAL;
	}

	entry->source = unescape_mount_field(mount_source);
	entry->target = unescape_mount_field(mount_point);
	entry->fs_type = strdup(fs_type);
	entry->options = strdup(mount_options);
	free(copy);

	if (entry->source == NULL || entry->target == NULL || entry->fs_type == NULL || entry->options == NULL) {
		free_mount_entry(entry);
		return ENOMEM;
	}
	return 0;
}

static int load_mount_entries(MountEntryVec *out) {
	FILE *fp = NULL;
	char *line = NULL;
	size_t line_cap = 0;
	ssize_t line_len;
	int rc = 0;

	if (out == NULL) {
		return EINVAL;
	}
	memset(out, 0, sizeof(*out));
	fp = fopen("/proc/self/mountinfo", "r");
	if (fp == NULL) {
		return (errno == 0) ? EIO : errno;
	}

	while ((line_len = getline(&line, &line_cap, fp)) >= 0) {
		MountEntry entry;
		if (line_len > 0 && line[line_len - 1] == '\n') {
			line[line_len - 1] = '\0';
		}
		rc = parse_mountinfo_line(line, &entry);
		if (rc == 0) {
			rc = push_mount_entry(out, &entry);
			if (rc != 0) {
				free_mount_entry(&entry);
			}
		}
		if (rc != 0) {
			break;
		}
	}

	free(line);
	fclose(fp);

	if (rc != 0) {
		free_mount_vec(out);
		return rc;
	}
	return 0;
}

static int mount_entry_target_cmp_desc(const void *lhs, const void *rhs) {
	const MountEntry *a = (const MountEntry *)lhs;
	const MountEntry *b = (const MountEntry *)rhs;
	size_t la = strlen(a->target);
	size_t lb = strlen(b->target);
	if (la < lb) {
		return 1;
	}
	if (la > lb) {
		return -1;
	}
	return strcmp(a->target, b->target);
}

int format_partition(const char *mount_path, const char *fs_type, bool force_format)
{
	MountEntryVec entries;
	MountEntryVec related;
	MountEntry input_mount;
	const char *device;
	int found = 0;
	size_t i;
	int rc;
	char *escaped = NULL;
	char cmd[4096];

	if (mount_path == NULL || mount_path[0] == '\0') {
		return EINVAL;
	}
	if (fs_type == NULL || strcmp(fs_type, "ext4") != 0) {
		return EINVAL;
	}

	memset(&entries, 0, sizeof(entries));
	memset(&related, 0, sizeof(related));
	memset(&input_mount, 0, sizeof(input_mount));

	rc = load_mount_entries(&entries);
	if (rc != 0) {
		return rc;
	}

	for (i = 0; i < entries.size; ++i) {
		if (path_eq(entries.items[i].target, mount_path)) {
			input_mount.source = strdup(entries.items[i].source);
			input_mount.target = strdup(entries.items[i].target);
			input_mount.fs_type = strdup(entries.items[i].fs_type);
			input_mount.options = strdup(entries.items[i].options);
			if (input_mount.source == NULL || input_mount.target == NULL ||
					input_mount.fs_type == NULL || input_mount.options == NULL) {
				free_mount_vec(&entries);
				free_mount_entry(&input_mount);
				return ENOMEM;
			}
			found = 1;
			break;
		}
	}
	if (!found) {
		free_mount_vec(&entries);
		return ENOENT;
	}
	device = input_mount.source;
	if (device == NULL || device[0] == '\0') {
		free_mount_vec(&entries);
		free_mount_entry(&input_mount);
		return ENODEV;
	}

	for (i = 0; i < entries.size; ++i) {
		if (strcmp(entries.items[i].source, device) == 0) {
			MountEntry m = {0};
			m.source = strdup(entries.items[i].source);
			m.target = strdup(entries.items[i].target);
			m.fs_type = strdup(entries.items[i].fs_type);
			m.options = strdup(entries.items[i].options);
			if (m.source == NULL || m.target == NULL || m.fs_type == NULL || m.options == NULL) {
				free_mount_entry(&m);
				free_mount_vec(&entries);
				free_mount_vec(&related);
				free_mount_entry(&input_mount);
				return ENOMEM;
			}
			rc = push_mount_entry(&related, &m);
			if (rc != 0) {
				free_mount_entry(&m);
				free_mount_vec(&entries);
				free_mount_vec(&related);
				free_mount_entry(&input_mount);
				return rc;
			}
		}
	}

	qsort(related.items, related.size, sizeof(MountEntry), mount_entry_target_cmp_desc);

	for (i = 0; i < related.size; ++i) {
		if (umount(related.items[i].target) == 0) {
			continue;
		}
		if (errno == EBUSY && !force_format) {
			free_mount_vec(&entries);
			free_mount_vec(&related);
			free_mount_entry(&input_mount);
			return EBUSY;
		}
		if (errno == EBUSY && force_format) {
			escaped = shell_escape(related.items[i].target);
			if (escaped == NULL) {
				free_mount_vec(&entries);
				free_mount_vec(&related);
				free_mount_entry(&input_mount);
				return ENOMEM;
			}
			snprintf(cmd, sizeof(cmd), "fuser -km %s >/dev/null 2>&1", escaped);
			free(escaped);
			escaped = NULL;
			rc = run_command(cmd);
			if (rc != 0) {
				free_mount_vec(&entries);
				free_mount_vec(&related);
				free_mount_entry(&input_mount);
				return rc;
			}
			if (umount(related.items[i].target) == 0) {
				continue;
			}
		}
		rc = (errno == 0) ? EIO : errno;
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return rc;
	}

	escaped = shell_escape(device);
	if (escaped == NULL) {
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return ENOMEM;
	}
	snprintf(cmd, sizeof(cmd), "blkdiscard %s", escaped);
	free(escaped);
	escaped = NULL;
	rc = run_command(cmd);
	if (rc != 0) {
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return rc;
	}

	escaped = shell_escape(device);
	if (escaped == NULL) {
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return ENOMEM;
	}
    snprintf(cmd, sizeof(cmd), "echo 3 > /proc/sys/vm/drop_caches");
    run_command(cmd);
    memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "mke2fs -t ext4 -b 1024 -O ^huge_file -F %s", escaped);
	free(escaped);
	escaped = NULL;
	rc = run_command(cmd);
	if (rc != 0) {
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return rc;
	}

	escaped = shell_escape(device);
	if (escaped == NULL) {
		free_mount_vec(&entries);
		free_mount_vec(&related);
		free_mount_entry(&input_mount);
		return ENOMEM;
	}
	{
		char *escaped_target = shell_escape(input_mount.target);
		if (escaped_target == NULL) {
			free(escaped);
			free_mount_vec(&entries);
			free_mount_vec(&related);
			free_mount_entry(&input_mount);
			return ENOMEM;
		}
		snprintf(cmd, sizeof(cmd), "mount -t ext4 %s %s", escaped, escaped_target);
		free(escaped_target);
	}
	free(escaped);
	rc = run_command(cmd);

	free_mount_vec(&entries);
	free_mount_vec(&related);
	free_mount_entry(&input_mount);
	return rc;
}

int format_partition2(const char *device_node, const char *fs_type, int force_format) {
    MountEntryVec entries;
    MountEntryVec related;
    size_t i;
    int rc;
    char *escaped = NULL;
    char cmd[4096];

    if (device_node == NULL || device_node[0] == '\0') {
        return EINVAL;
    }
    if (fs_type == NULL || strcmp(fs_type, "ext4") != 0) {
        return EINVAL;
    }

    memset(&entries, 0, sizeof(entries));
    memset(&related, 0, sizeof(related));

    rc = load_mount_entries(&entries);
    if (rc != 0) {
        return rc;
    }

    for (i = 0; i < entries.size; ++i) {
        if (strcmp(entries.items[i].source, device_node) == 0) {
            MountEntry m = {0};
            m.source = strdup(entries.items[i].source);
            m.target = strdup(entries.items[i].target);
            m.fs_type = strdup(entries.items[i].fs_type);
            m.options = strdup(entries.items[i].options);
            if (m.source == NULL || m.target == NULL || m.fs_type == NULL || m.options == NULL) {
                free_mount_entry(&m);
                free_mount_vec(&entries);
                free_mount_vec(&related);
                return ENOMEM;
            }
            rc = push_mount_entry(&related, &m);
            if (rc != 0) {
                free_mount_entry(&m);
                free_mount_vec(&entries);
                free_mount_vec(&related);
                return rc;
            }
        }
    }

    qsort(related.items, related.size, sizeof(MountEntry), mount_entry_target_cmp_desc);

    for (i = 0; i < related.size; ++i) {
        if (umount(related.items[i].target) == 0) {
            continue;
        }
        if (errno == EBUSY && !force_format) {
            free_mount_vec(&entries);
            free_mount_vec(&related);
            return EBUSY;
        }
        if (errno == EBUSY && force_format) {
            escaped = shell_escape(related.items[i].target);
            if (escaped == NULL) {
                free_mount_vec(&entries);
                free_mount_vec(&related);
                return ENOMEM;
            }
            snprintf(cmd, sizeof(cmd), "fuser -km %s", escaped);
            free(escaped);
            escaped = NULL;
            rc = run_command(cmd);
            if (rc != 0) {
                free_mount_vec(&entries);
                free_mount_vec(&related);
                return rc;
            }
            if (umount(related.items[i].target) == 0) {
                continue;
            }
        }
        rc = (errno == 0) ? EIO : errno;
        free_mount_vec(&entries);
        free_mount_vec(&related);
        return rc;
    }

    escaped = shell_escape(device_node);
    if (escaped == NULL) {
        free_mount_vec(&entries);
        free_mount_vec(&related);
        return ENOMEM;
    }
    snprintf(cmd, sizeof(cmd), "blkdiscard %s", escaped);
    free(escaped);
    escaped = NULL;
    rc = run_command(cmd);
    if (rc != 0) {
        free_mount_vec(&entries);
        free_mount_vec(&related);
        return rc;
    }

    escaped = shell_escape(device_node);
    if (escaped == NULL) {
        free_mount_vec(&entries);
        free_mount_vec(&related);
        return ENOMEM;
    }
    snprintf(cmd, sizeof(cmd), "mke2fs -t ext4 -b 1024 -O ^huge_file -F %s", escaped);
    free(escaped);
    escaped = NULL;
    rc = run_command(cmd);
    if (rc != 0) {
        free_mount_vec(&entries);
        free_mount_vec(&related);
        return rc;
    }

    for (i = 0; i < related.size; ++i) {
        char *escaped_device = shell_escape(device_node);
        char *escaped_target = NULL;
        if (escaped_device == NULL) {
            free_mount_vec(&entries);
            free_mount_vec(&related);
            return ENOMEM;
        }
        escaped_target = shell_escape(related.items[i].target);
        if (escaped_target == NULL) {
            free(escaped_device);
            free_mount_vec(&entries);
            free_mount_vec(&related);
            return ENOMEM;
        }
        snprintf(cmd, sizeof(cmd), "mount -t ext4 %s %s", escaped_device, escaped_target);
        free(escaped_device);
        free(escaped_target);
        rc = run_command(cmd);
        if (rc != 0) {
            free_mount_vec(&entries);
            free_mount_vec(&related);
            return rc;
        }
    }

    free_mount_vec(&entries);
    free_mount_vec(&related);
    return 0;
}
