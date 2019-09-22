//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "bootstrap.h"

/* open/fcntl.  */
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#ifndef O_CREAT
# define O_CREAT	   0100	/* Not fcntl.  */
#endif
#ifndef O_EXCL
# define O_EXCL		   0200	/* Not fcntl.  */
#endif
#ifndef O_NOCTTY
# define O_NOCTTY	   0400	/* Not fcntl.  */
#endif
#ifndef O_TRUNC
# define O_TRUNC	  01000	/* Not fcntl.  */
#endif
#ifndef O_APPEND
# define O_APPEND	  02000
#endif
#ifndef O_NONBLOCK
# define O_NONBLOCK	  04000
#endif
#ifndef O_NDELAY
# define O_NDELAY	O_NONBLOCK
#endif
#ifndef O_SYNC
# define O_SYNC	       04010000
#endif
#define O_FSYNC		O_SYNC
#ifndef O_ASYNC
# define O_ASYNC	 020000
#endif
#ifndef __O_LARGEFILE
# define __O_LARGEFILE	0100000
#endif

#if defined(__i386__)
#define AX "eax"
#define BX "ebx"
#define CX "ecx"
#define DX "edx"
#define SP "esp"
#define SYSCALL "int $0x80"
#elif defined(__x86_64)
#define AX "rax"
#define BX "rbx"
#define CX "rcx"
#define DX "rdx"
#define SP "rsp"
#define SYSCALL "int $0x80"
#endif

inline void exit(int __status);
inline void execve(const char *, const char *[], const char *[]);

void exit(int __status) {
    asm volatile (
        "\n\tmov $1,%%"AX";"
        "\n\tmov %0,%%"BX";"
        "\n\t"SYSCALL";"
        :: "m"(__status)
    );
}

void _write(int fd, const void *buf, size_t count) {
    asm volatile (
        "\n\tmov $4, %%"AX";"
        "\n\tmov %0, %%"BX";"
        "\n\tmov %1, %%"CX";"
        "\n\tmov %2, %%"DX";"
        "\n\t"SYSCALL";"
        :
        : "m"(fd), "m"(buf), "m"(count)
    );
}

void _open(const char *pathname, int flags, mode_t mode) {
    asm volatile (
        "\n\tmov $5, %%"AX";"
        "\n\tmov %0, %%"BX";"
        "\n\tmov %1, %%"CX";"
        "\n\tmov %2, %%"DX";"
        "\n\t"SYSCALL";"
        :
        : "m"(pathname), "m"(flags), "m"(mode)
    );
}

void _close(unsigned int fd) {
    asm volatile (
        "\n\tmov $6, %%"AX";"
        "\n\tmov %0, %%"BX";"
        "\n\t"SYSCALL";"
        :
        : "m"(fd)
    );
}

void _fork() {
    asm volatile (
        "\n\tmov $2, %%"AX";"
        "\n\t"SYSCALL";"
        ::
    );
}

void execve(const char *pathname, const char *argv[], const char *envp[]) {
    asm volatile (
        "\n\tmov $0xB, %%"AX";"
        "\n\tmov %0, %%"BX";"
        "\n\tmov %1, %%"CX";"
        "\n\tmov %2, %%"DX";"
        "\n\t"SYSCALL";"
        :
        : "m"(pathname), "m"(argv), "m"(envp)
    );
}

void _waitpid(pid_t pid, int *wstatus, int options) {
    asm volatile (
        "\n\tmov $0x7, %%"AX";"
        "\n\tmov %0, %%"BX";"
        "\n\tmov %1, %%"CX";"
        "\n\tmov %2, %%"DX";"
        "\n\t"SYSCALL";"
        :
        : "m"(pid), "m"(wstatus), "m"(options)
    );
}

void _wait(int *wstatus) {
    _waitpid(-1, wstatus, 0);
}

size_t (*write)(int, const void*, size_t) = (size_t (*)(int, const void*, size_t))&_write;
int (*open)(const char*, int, ...) = (int (*)(const char *, int, ...))&_open;
int (*close)(int fd) = (int (*)(int))&_close;
pid_t (*fork)(void) = (pid_t (*)(void))&_fork;
pid_t (*waitpid)(pid_t, int *, int) = (pid_t (*)(pid_t, int *, int))&_waitpid;
pid_t (*wait)(int *) = (pid_t (*)(int *))&_wait;

const char *archive = _binary_archive_start;
const size_t size = (size_t)_binary_archive_size;

inline void extract_from_elf();
inline void uncompress();

void extract_from_elf() {
    int fd = open(BINARY_ARCHIVE_TEMP, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRGRP | S_IXGRP);
    size_t written = write(fd, archive, size);
    if (written != size) {
        exit(1);
    }
    close(fd);
}

#define NULL (void *)0

void uncompress() {
    pid_t pid;
    if ((pid = fork()) == 0) {
        static const char *argv[] = {
            "/bin/tar",
            "-zxvf",
            BINARY_ARCHIVE_TEMP,
            NULL
        };
        static const char *env[] = {
            NULL
        };
        execve(argv[0], argv, env);
    } else {
        int wstatus;
        wait(&wstatus);
    }
}

void launch() {
    static const char *argv[] = {
        "./"BINARY_ARCHIVE_NAME"/bootstrap.sh",
        NULL
    };
    static const char *env[] = {
        NULL
    };
    execve(argv[0], argv, env);
}

int main() {
    extract_from_elf();
    uncompress();
    launch();
    return 1;
}

void _start() {
    exit(main());
}