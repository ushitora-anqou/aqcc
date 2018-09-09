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
    return *s1 - *s2;
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

typedef __builtin_va_list va_list;
#ifndef va_start
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
#endif

int vsprintf_wrap(char *str, const char *format, va_list ap)
{
    const char *p = format;
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

    return 0;  // TODO: should return length of printed letters
}

void *syscall_wrap(int number, ...);

void exit_wrap(int status)
{
    // __NR_exit
    syscall_wrap(60, status);
}

// void *brk_wrap(void *addr)
//{
//    // __NR_brk
//    return syscall_wrap(12, addr);
//}
//
// void *malloc_wrap(int size)
//{
//    static void *malloc_pointer_head = 0;
//    static int malloc_remaining_size = 0;
//
//    if (malloc_pointer_head == 0) {
//        void *p = brk_wrap(0);
//        int size = 0x32000000;
//        void *q = brk_wrap(p + size);
//        // printf("init %d\n", malloc_remaining_size);
//        // printf("init %p\n", p);
//        // printf("init %p\n", q);
//        malloc_pointer_head = p;
//        malloc_remaining_size = size;
//    }
//
//    if (malloc_remaining_size < size) {
//        printf("BUG%d\n", malloc_remaining_size);
//        printf("BUG%d\n", size);
//        return 0;
//    }
//
//    void *ret = malloc_pointer_head + 4;
//    malloc_pointer_head += size + 4;
//    malloc_remaining_size -= size + 4;
//
//    // printf("%d\n", malloc_remaining_size);
//    // printf("%d\n", size);
//    // printf("%p\n", ret);
//    // printf("%p\n", ret + size);
//    return ret;
//}

int open_wrap(const char *path, int oflag)
{
    return (int)syscall_wrap(2, path, oflag);
}
