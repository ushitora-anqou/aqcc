#define NULL_wrap 0
#define EOF_wrap -1

#ifdef __GNUC__
typedef __builtin_va_list va_list;
#else
#endif
#ifndef __GNUC__
typedef struct {
    int gp_offset;
    int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
} va_list[1];
#endif
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

int isdigit_wrap(int c) { return '0' <= c && c <= '9'; }

int isalpha_wrap(int c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

int isalnum_wrap(int c) { return isdigit_wrap(c) || isalpha_wrap(c); }

int isspace_wrap(int c)
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

void *memcpy_wrap(void *dst, const void *src, int n)
{
    for (int i = 0; i < n; i++) *((char *)dst + i) = *((char *)src + i);
    return dst;
}

char *strcpy_wrap(char *dst, const char *src)
{
    char *ret = dst;
    while (*src != '\0') *dst++ = *src++;
    *dst = '\0';
    return ret;
}

int strcmp_wrap(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s1 == *s2) s1++, s2++;
    return (*s1 & 0xff) - (*s2 & 0xff);
}

int strlen_wrap(const char *s)
{
    int cnt = 0;
    while (*s++ != '\0') cnt++;
    return cnt;
}

void *memset_wrap(void *s, int c, int n)
{
    for (int i = 0; i < n; i++) *((char *)s + i) = c;
    return s;
}

int vsprintf_wrap(char *str, const char *format, va_list ap)
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

void *syscall_wrap(int number, ...);

void exit_wrap(int status)
{
    // __NR_exit
    syscall_wrap(60, status);
}

int printf_wrap(const char *format, ...);
void *brk_wrap(void *addr)
{
    // __NR_brk
    // printf_wrap("initbrk %d\n", addr);
    return syscall_wrap(12, addr);
}

void *malloc_wrap(int size)
{
    static char *malloc_pointer_head = 0;
    static int malloc_remaining_size = 0;

    if (malloc_pointer_head == 0) {
        char *p = brk_wrap(0);
        int size = 0x32000000;
        char *q = brk_wrap(p + size);
        // printf_wrap("init %d\n", p);
        // printf_wrap("init %d\n", q);
        malloc_pointer_head = p;
        malloc_remaining_size = size;
    }

    if (malloc_remaining_size < size) {
        printf_wrap("BUG%d\n", malloc_remaining_size);
        printf_wrap("BUG%d\n", size);
        return NULL_wrap;
    }

    char *ret = malloc_pointer_head + 4;
    malloc_pointer_head += size + 4;
    malloc_remaining_size -= size + 4;

    // printf_wrap("%d\n", malloc_remaining_size);
    // printf_wrap("%d\n", size);
    // printf_wrap("%d\n", ret);
    // printf_wrap("%d\n", ret + size);
    return ret;
}

int open_wrap(const char *path, int oflag, int mode)
{
    return (int)syscall_wrap(2, path, oflag, mode);
}

int close_wrap(int fd) { return (int)syscall_wrap(3, fd); }

typedef struct FILE_wrap {
    int fd;
} FILE_wrap;

int write_wrap(int fd, const void *buf, int count)
{
    return (int)syscall_wrap(1, fd, buf, count);
}

int read_wrap(int fd, const void *buf, int count)
{
    return (int)syscall_wrap(0, fd, buf, count);
}

FILE_wrap *fopen_wrap(const char *pathname, const char *mode)
{
    if (mode[0] == 'w') {
        FILE_wrap *file = (FILE_wrap *)malloc_wrap(sizeof(FILE_wrap));
        // O_CREAT | O_WRONLY | O_TRUNC
        file->fd = open_wrap(pathname, 64 | 1 | 512, 0644);
        if (file->fd == -1) return NULL_wrap;
        return file;
    }

    if (mode[0] == 'r') {
        FILE_wrap *file = (FILE_wrap *)malloc_wrap(sizeof(FILE_wrap));
        //  O_RDONLY
        file->fd = open_wrap(pathname, 0, 0);
        if (file->fd == -1) return NULL_wrap;
        return file;
    }

    assert(0);
}

int fclose_wrap(FILE_wrap *stream) { return close_wrap(stream->fd); }

int fputc_wrap(int c, FILE_wrap *stream)
{
    char buf[1];
    buf[0] = c & 0xff;
    return write_wrap(stream->fd, buf, 1);
}

int fgetc_wrap(FILE_wrap *stream)
{
    char buf[1];
    int res = read_wrap(stream->fd, buf, 1);
    if (res <= 0) return EOF_wrap;
    return buf[0] & 0xff;
}

int fprintf_wrap(FILE_wrap *stream, const char *format, ...)
{
    char buf[512];  // TODO: enough length?
    va_list args;

    va_start(args, format);
    int cnt = vsprintf_wrap(buf, format, args);
    va_end(args);

    write_wrap(stream->fd, buf, cnt);
    return cnt;
}

int printf_wrap(const char *format, ...)
{
    char buf[512];  // TODO: enough length?
    va_list args;

    va_start(args, format);
    int cnt = vsprintf_wrap(buf, format, args);
    va_end(args);

    write_wrap(1, buf, cnt);
    return cnt;
}
