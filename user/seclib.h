#ifndef SECLIB_H
#define SECLIB_H

unsigned long djb2_hash(char *str);
void itoa(int n, char* buf);
void safe_gets(char *buf, int max);

#endif
