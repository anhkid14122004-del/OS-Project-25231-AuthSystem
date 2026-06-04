#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "seclib.h"

// Ham ghi log danh rieng cho viec xoa user
void write_log(char *target, char *status) {
    int time_tick = uptime(); 
    char time_str[16];
    itoa(time_tick, time_str);

    char log_msg[128];
    strcpy(log_msg, "[Tick: "); strcpy(log_msg + strlen(log_msg), time_str);
    strcpy(log_msg + strlen(log_msg), "] Admin action: Delete user '"); strcpy(log_msg + strlen(log_msg), target);
    strcpy(log_msg + strlen(log_msg), "' - "); strcpy(log_msg + strlen(log_msg), status);
    strcpy(log_msg + strlen(log_msg), "\n");

    int fd = open("auth.log", O_CREATE | O_RDWR);
    if (fd >= 0) {
        char temp;
        while(read(fd, &temp, 1) > 0); // Tua den cuoi file
        write(fd, log_msg, strlen(log_msg));
        close(fd);
    }
}

int main(int argc, char *argv[]) {
    // =========================================================
    // 1. KIEM TRA QUYEN ADMIN (RBAC)
    // =========================================================
    int fd_session = open(".current_user", 0); // 0 = O_RDONLY
    if(fd_session < 0) {
        printf("Loi: Ban phai dang nhap de su dung lenh nay!\n");
        exit(1);
    }
    
    char current_user[32];
    int n = read(fd_session, current_user, sizeof(current_user)-1);
    close(fd_session);
    
    if(n > 0) {
       current_user[n] = '\0';
        // Loc bo ky tu xuong dong
        for(int i = 0; i < n; i++) {
            if(current_user[i] == '\n' || current_user[i] == '\r') {
                current_user[i] = '\0';
                break;
            }
        }
    }

    if(strcmp(current_user, "admin") != 0) {
        printf("TU CHOI TRUY CAP: Chi co 'admin' moi duoc xoa tai khoan!\n");
        exit(1);
    }

    // =========================================================
    // 2. PRO-TIP: KIỂM TRA MUTEX TRƯỚC (FAIL-FAST)
    // =========================================================
    // Nếu có tiến trình khác (như register) đang khóa file, đuổi ra ngoài ngay
    int check_fd;
    if((check_fd = open("users.lock", O_RDONLY)) >= 0) {
        close(check_fd);
        printf("[!] He thong dang ban ghi du lieu. Vui long quay lai sau!\n");
        exit(1); 
    }

    // =========================================================
    // 3. NHAP TAI KHOAN CAN XOA (Khi hệ thống rảnh)
    // =========================================================
    char target_user[32];
    printf("\n=== QUAN LY TAI KHOAN (DELETE) ===\n");
    printf("Nhap ten tai khoan can xoa: ");
    gets(target_user, sizeof(target_user));
    
    // Loai bo ky tu xuong dong khi nhap
    if(strlen(target_user) > 0 && target_user[strlen(target_user)-1] == '\n') {
        target_user[strlen(target_user)-1] = '\0';
    }
    if (strlen(target_user) == 0) exit(0);

    // Bao ve tai khoan root (Anti-bricking)
    if(strcmp(target_user, "admin") == 0) {
        printf("Loi: Khong the xoa tai khoan goc (admin)!\n");
        exit(1);
    }

    // =========================================================
    // 4. CHÍNH THỨC SẬP KHÓA (ACQUIRE MUTEX)
    // =========================================================
    int lock_fd = open("users.lock", O_CREATE | O_WRONLY);
    if (lock_fd >= 0) close(lock_fd);

    // =========================================================
    // 5. DOC CSDL VA LOC BO TAI KHOAN CAN XOA
    // =========================================================
    int fd = open("users.dat", O_RDONLY);
    if (fd < 0) {
        printf("Loi doc CSDL.\n"); 
        unlink("users.lock"); // Nhớ nhả khóa nếu lỗi
        exit(1);
    }

    char buf[1024];
    n = read(fd, buf, sizeof(buf));
    close(fd);
    buf[n] = '\0';

    char new_file_data[1024];
    new_file_data[0] = '\0';
    int i = 0, line_idx = 0, found = 0;
    char line[128];

    while (i < n) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            line[line_idx] = '\0';
            char f_user[32];
            int j = 0, k = 0;
            while (line[j] != ':' && line[j] != '\0') { f_user[k++] = line[j++]; }
            f_user[k] = '\0';

            if (strcmp(target_user, f_user) == 0) {
                found = 1; // Tim thay user -> Bo qua khong copy vao data moi
            } else {
                // Chep cac user hop le sang data moi
                strcpy(new_file_data + strlen(new_file_data), line);
                strcpy(new_file_data + strlen(new_file_data), "\n");
            }
            line_idx = 0;
        } else { line[line_idx++] = buf[i]; }
        i++;
    }

    // =========================================================
    // 6. LUU LAI CSDL, GHI LOG VA NHẢ KHÓA (RELEASE)
    // =========================================================
    if (found) {
        unlink("users.dat"); 
        fd = open("users.dat", O_CREATE | O_WRONLY);
        write(fd, new_file_data, strlen(new_file_data));
        close(fd);
        printf("-> Da xoa vinh vien tai khoan '%s'!\n", target_user);
        write_log(target_user, "SUCCESS");
    } else {
        printf("-> Loi: Khong tim thay tai khoan '%s' tren he thong!\n", target_user);
        write_log(target_user, "FAILED (USER NOT FOUND)");
    }

    unlink("users.lock"); // NHA KHOA
    exit(0);
}
