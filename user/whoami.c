#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[]) {
    // Mo file phien lam viec
    int fd = open(".current_user", 0); // 0 = O_RDONLY
    if (fd < 0) {
        printf("Chua co nguoi dung nao dang nhap vao he thong!\n");
        exit(1);
    }

    char current_user[32];
    int n = read(fd, current_user, sizeof(current_user) - 1);
    close(fd);

    if (n > 0) {
        current_user[n] = '\0';
        // Got bo dau xuong dong de in ra cho dep
        for(int i = 0; i < n; i++) {
            if(current_user[i] == '\n' || current_user[i] == '\r') {
                current_user[i] = '\0';
                break;
            }
        }
        // In ten nguoi dung ra man hinh
        printf("%s\n", current_user);
    } else {
        printf("Loi: File phien dang nhap bi rong!\n");
        exit(1);
    }

    exit(0);
}
