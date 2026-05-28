#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "seclib.h"

int main(int argc, char *argv[]) {
    int fd_session = open(".current_user", O_RDONLY);
    if(fd_session < 0) {
        int fd_check = open("users.dat", O_RDONLY);
        if(fd_check >= 0) {
            close(fd_check);
            printf("Loi: Ban phai dang nhap he thong truoc!\n");
            exit(1);
        }
    } else {
        char current_user[32];
        int n = read(fd_session, current_user, sizeof(current_user)-1);
        close(fd_session);
        current_user[n] = '\0';
        
        if(strcmp(current_user, "admin") != 0) {
            printf("TU CHOI TRUY CAP: Chi co 'admin' moi duoc tao tai khoan!\n");
            exit(1);
        }
    }

    char input_user[32], input_pass[32], hash_str[32];
    printf("\n=== DANG KY TAI KHOAN ===\n");
    printf("Username: ");
    gets(input_user, sizeof(input_user));
    input_user[strlen(input_user) - 1] = '\0';

    printf("Password: ");
    setecho(0);
    gets(input_pass, sizeof(input_pass));
    input_pass[strlen(input_pass) - 1] = '\0';
    setecho(1);
    printf("\n");

    itoa(djb2_hash(input_pass), hash_str);

    int lock_fd;
    int retries = 0;
    
    while((lock_fd = open("users.lock", O_RDONLY)) >= 0) {
        close(lock_fd);
        printf("[!] He thong dang ban ghi du lieu. Dang cho toi luot...\n");
        
        // THAY THE HAM SLEEP
        volatile int delay1;
        for(delay1 = 0; delay1 < 50000000; delay1++); 
        
        retries++;
        if(retries > 10) {
            printf("Loi: Timeout! Khong the truy cap CSDL luc nay.\n");
            exit(1);
        }
    }
    
    lock_fd = open("users.lock", O_CREATE | O_WRONLY);
    if (lock_fd >= 0) close(lock_fd);

    printf("[DEBUG] Da lay duoc khoa Mutex. Dang ghi...\n");
    
    // CO TINH DELAY DE CHUP ANH
    volatile int delay2;
    for(delay2 = 0; delay2 < 300000000; delay2++); 

    int fd = open("users.dat", O_RDWR | O_CREATE);
    if(fd >= 0) {
        char tmp;
        while(read(fd, &tmp, 1) > 0); 
        write(fd, input_user, strlen(input_user));
        write(fd, ":", 1);
        write(fd, hash_str, strlen(hash_str));
        write(fd, "\n", 1);
        close(fd);
        printf("-> Dang ky thanh cong!\n");
    } else {
        printf("-> Loi CSDL!\n");
    }

    unlink("users.lock");
    exit(0);
}
