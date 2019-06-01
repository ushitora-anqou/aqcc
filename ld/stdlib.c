#include "ld.h"

int isdigit(int c) { return '0' <= c && c <= '9'; }

int isalpha(int c) { return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'); }

int isalnum(int c) { return isdigit(c) || isalpha(c); }

int isspace(int c)
{
    switch (c) {
        case ' ':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            return 1;
    }
    return 0;
}

void *memcpy(void *dst, const void *src, int n)
{
    for (int i = 0; i < n; i++) *((char *)dst + i) = *((char *)src + i);
    return dst;
}

char *strcpy(char *dst, const char *src)
{
    char *ret = dst;
    while (*src != '\0') *dst++ = *src++;
    *dst = '\0';
    return ret;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s1 == *s2) s1++, s2++;
    return (*s1 & 0xff) - (*s2 & 0xff);
}

int strlen(const char *s)
{
    int cnt = 0;
    while (*s++ != '\0') cnt++;
    return cnt;
}

void *memset(void *s, int c, int n)
{
    for (int i = 0; i < n; i++) *((char *)s + i) = c;
    return s;
}

int vsprintf(char *str, const char *format, va_list ap)
{
    const char *p = format, *org_str = str;
    while (*p != '\0') {
        if (*p != '%') {
            *str++ = *p++;
            continue;
        }

        p++;
        switch (*p++) {
            case '\0':
                goto end;

            case 'c':
                *str++ = va_arg(ap, int);
                break;

            case 's': {
                char *src = va_arg(ap, char *);
                while (*src != '\0') *str++ = *src++;
            } break;

            case 'd': {
                int ival = va_arg(ap, int);

                if (ival == 0) {
                    *str++ = '0';
                    break;
                }

                if (ival < 0) {
                    *str++ = '-';
                    ival *= -1;
                }

                int i = 0, buf[256];  // TODO: enough length?
                for (; ival != 0; ival /= 10) buf[i++] = ival % 10;
                while (--i >= 0) *str++ = '0' + buf[i];
            } break;

            default:
                assert(0);
        }
    }

end:
    *str = '\0';

    return str - org_str;
}

void *syscall(int number, ...);

_Noreturn void exit(int status)
{
    // __NR_exit
    syscall(60, status);
}

void *brk(void *addr)
{
    // __NR_brk
    // printf("initbrk %d\n", addr);
    return syscall(12, addr);
}

void *malloc(int size)
{
    static char *malloc_pointer_head = 0;
    static int malloc_remaining_size = 0;

    if (malloc_pointer_head == 0) {
        char *p = brk(0);
        int size = 0x32000000;
        brk(p + size);
        // printf("init %d\n", p);
        malloc_pointer_head = p;
        malloc_remaining_size = size;
    }

    if (malloc_remaining_size < size) {
        printf("BUG%d\n", malloc_remaining_size);
        printf("BUG%d\n", size);
        return NULL;
    }

    char *ret = malloc_pointer_head + 4;
    malloc_pointer_head += size + 4;
    malloc_remaining_size -= size + 4;

    // printf("%d\n", malloc_remaining_size);
    // printf("%d\n", size);
    // printf("%d\n", ret);
    // printf("%d\n", ret + size);
    return ret;
}

int open(const char *path, int oflag, int mode)
{
    return (int)syscall(2, path, oflag, mode);
}

int close(int fd) { return (int)syscall(3, fd); }

struct _IO_FILE {
    int fd;
};

int write(int fd, const void *buf, int count)
{
    return (int)syscall(1, fd, buf, count);
}

int read(int fd, const void *buf, int count)
{
    return (int)syscall(0, fd, buf, count);
}

FILE *fopen(const char *pathname, const char *mode)
{
    if (mode[0] == 'w') {
        FILE *file = (FILE *)malloc(sizeof(FILE));
        // O_CREAT | O_WRONLY | O_TRUNC
        file->fd = open(pathname, 64 | 1 | 512, 0644);
        if (file->fd == -1) return NULL;
        return file;
    }

    if (mode[0] == 'r') {
        FILE *file = (FILE *)malloc(sizeof(FILE));
        //  O_RDONLY
        file->fd = open(pathname, 0, 0);
        if (file->fd == -1) return NULL;
        return file;
    }

    assert(0);
}

int fclose(FILE *stream) { return close(stream->fd); }

int fputc(int c, FILE *stream)
{
    char buf[1];
    buf[0] = c & 0xff;
    return write(stream->fd, buf, 1);
}

int fgetc(FILE *stream)
{
    char buf[1];
    int res = read(stream->fd, buf, 1);
    if (res <= 0) return EOF;
    return buf[0] & 0xff;
}

int fprintf(FILE *stream, const char *format, ...)
{
    char buf[512];  // TODO: enough length?
    va_list args;

    va_start(args, format);
    int cnt = vsprintf(buf, format, args);
    va_end(args);

    write(stream->fd, buf, cnt);
    return cnt;
}

int printf(const char *format, ...)
{
    char buf[512];  // TODO: enough length?
    va_list args;

    va_start(args, format);
    int cnt = vsprintf(buf, format, args);
    va_end(args);

    write(1, buf, cnt);
    return cnt;
}

void assert(int cond)
{
    if (cond) return;
    // fprintf(stderr, "[ASSERT] %d\n", cond);
    printf("[ASSERT] %d\n", cond);
    exit(EXIT_FAILURE);
}
