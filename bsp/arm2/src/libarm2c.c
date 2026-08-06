#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/* ================= 完全假的堆 ================= */
size_t strnlen(const char*s, size_t maxlen)
{
	size_t i;
	for (i = 0; i < maxlen && s[i]; i++);
	return i;
}

void *_sbrk(ptrdiff_t incr)
{
    errno = ENOMEM;
    return (void *)-1;
}

/* ================= 文件 / IO 全部是假 ================= */

int _close(int fd)
{
    errno = EBADF;
    return -1;
}

int _lseek(int fd, int ptr, int dir)
{
    return 0;
}

int _read(int fd, char *ptr, int len)
{
    errno = EBADF;
    return -1;
}

/*
 * write：直接吞掉，保证 printf 不炸
 */
int _write(int fd, const char *ptr, int len)
{
    return len;
}

int _fstat(int fd, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    return 1;
}

/* ================= 进程相关：裸机兜底 ================= */

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}

void _exit(int status)
{
    while (1);
}
