// Ejercicio 6a
#include "syscall.h"

unsigned strlen (const char *s ) {
    unsigned i;
    for(i=0; s[i]!='\0'; i++);
    return i;
}

int puts2 (const char *s ) {
    unsigned len = strlen(s);
    return Write(s, len, CONSOLE_OUTPUT);
}

void itoa (int n , char *str) {
    if (n < 0) {
        str[0] = '-';
        str++;
    }

    int length = 0;
    for (int digit; n >= 0; n /= 10, length++) {
        digit = n % 10;
        str[length] = digit;
    }
    char ch;
    for (int i=0, last=length-1; i<=last; i++, last--) {
        ch = str[i];
        str[i] = str[last];
        str[last] = ch;
    }
}
