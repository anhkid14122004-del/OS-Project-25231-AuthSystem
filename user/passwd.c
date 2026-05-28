#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "seclib.h"

int main(int argc, char *argv[]) {
    int fd_session = open(".current_user", O_RDONLY);
    if(fd_session < 0) {
        printf("Loi: Ban phai dang nhap de doi mat khau!\n");
        exit(1);
    }
    
    char current_user[32];
    int n = read(fd_session, current_user, sizeof(current_user)-1);
    close(fd_session);
    current_user[n] = '\0';

    char old_pass[32], new_pass[32], old_hash_str[32], new_hash_str[32];
    printf("\n=== DOI MAT KHAU TAI KHOAN: %s ===\n", current_user);

    printf("Mat khau cu: ");
    setecho(0); gets(old_pass, sizeof(old_pass)); setecho(1); printf("\n");
    old_pass[strlen(old_pass) - 1] = '\0';

    printf("Mat khau moi: ");
    setecho(0); gets(new_pass, sizeof(new_pass)); setecho(1); printf("\n");
    new_pass[strlen(new_pass) - 1] = '\0';

    itoa(djb2_hash(old_pass), old_hash_str);
    itoa(djb2_hash(new_pass), new_hash_str);

    int lock_fd;
    while((lock_fd = open("users.lock", O_RDONLY)) >= 0) {
        close(lock_fd);
        printf("[!] He thong dang ban. Vui long doi...\n");
        // THAY THE HAM SLEEP BANG VONG LAP CAU GIO
        volatile int delay1;
        for(delay1 = 0; delay1 < 50000000; delay1++);
    }
    lock_fd = open("users.lock", O_CREATE | O_WRONLY);
    if (lock_fd >= 0) close(lock_fd);

    int fd = open("users.dat", O_RDONLY);
    if (fd < 0) {
        printf("Loi doc CSDL.\n"); unlink("users.lock"); exit(1);
    }

    char buf[1024];
    n = read(fd, buf, sizeof(buf));
    close(fd);
    buf[n] = '\0';

    char new_file_data[1024];
    new_file_data[0] = '\0';
    int i = 0, line_idx = 0;
    char line[128];
    int found_and_matched = 0;

    while (i < n) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            line[line_idx] = '\0';
            char f_user[32], f_hash[32];
            int j = 0, k = 0;
            while (line[j] != ':' && line[j] != '\0') { f_user[k++] = line[j++]; }
            f_user[k] = '\0';
            if (line[j] == ':') {
                j++; k = 0;
                while (line[j] != '\0') { f_hash[k++] = line[j++]; }
                f_hash[k] = '\0';
            }

            if (strcmp(current_user, f_user) == 0) {
                if (strcmp(old_hash_str, f_hash) == 0) {
                    found_and_matched = 1;
                    strcpy(new_file_data + strlen(new_file_data), current_user);
                    strcpy(new_file_data + strlen(new_file_data), ":");
                    strcpy(new_file_data + strlen(new_file_data), new_hash_str);
                    strcpy(new_file_data + strlen(new_file_data), "\n");
                } else {
                    printf("Loi: Mat khau cu khong chinh xac!\n");
                    unlink("users.lock"); exit(1);
                }
            } else {
                strcpy(new_file_data + strlen(new_file_data), line);
                strcpy(new_file_data + strlen(new_file_data), "\n");
            }
            line_idx = 0;
        } else { line[line_idx++] = buf[i]; }
        i++;
    }

    if (found_and_matched) {
        unlink("users.dat"); 
        fd = open("users.dat", O_CREATE | O_WRONLY);
        write(fd, new_file_data, strlen(new_file_data));
        close(fd);
        printf("-> Doi mat khau thanh cong!\n");
    }

    unlink("users.lock");
    exit(0);
}
