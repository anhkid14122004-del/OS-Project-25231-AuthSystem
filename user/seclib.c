#include "kernel/types.h"
#include "user/user.h"
#include "seclib.h"

unsigned long djb2_hash(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) { hash = ((hash << 5) + hash) + c; }
    return hash;
}

void itoa(int n, char* buf) {
    int i = 0, sign = n;
    if (sign < 0) n = -n;
    do { buf[i++] = n % 10 + '0'; } while ((n /= 10) > 0);
    if (sign < 0) buf[i++] = '-';
    buf[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = buf[j]; buf[j] = buf[k]; buf[k] = temp;
    }
}

void safe_gets(char *buf, int max) {
    gets(buf, max);
    int len = strlen(buf);
    if(len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
}
