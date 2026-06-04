#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "seclib.h"

int main(int argc, char *argv[]) {
    // 1. CHỐT CHẶN PHÂN QUYỀN (RBAC)
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

    // 2. TỐI ƯU HÓA NHẬP LIỆU CHO ĐA TIẾN TRÌNH (KỊCH BẢN 5)
    if(argc == 3) {
        // Chế độ chạy ngầm (Non-interactive): Lấy từ đối số truyền vào
        strcpy(input_user, argv[1]);
        strcpy(input_pass, argv[2]);
        printf("[DEBUG] Bat dau tien trinh dang ky ngam cho user: %s\n", input_user);
    } else {
        // Chế độ bình thường (Interactive): Hỏi qua bàn phím
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
    }

    itoa(djb2_hash(input_pass), hash_str);
    int lock_fd;
    int retries = 0;
    
    // 3. CƠ CHẾ MUTEX: CHỜ VÀ YÊU CẦU KHÓA (ACQUIRE)
    while((lock_fd = open("users.lock", O_RDONLY)) >= 0) {
        close(lock_fd);
        printf("[!] He thong dang ban ghi du lieu. Dang cho toi luot...\n");

        volatile int delay1;
        for(delay1 = 0; delay1 < 50000000; delay1++); 

        retries++;
        if(retries > 10) {
            printf("Loi: Timeout! Khong the truy cap CSDL luc nay.\n");
            exit(1);
        }
    }
    
    // Khóa CSDL bằng cách tạo file lock
    lock_fd = open("users.lock", O_CREATE | O_WRONLY);
    if (lock_fd >= 0) close(lock_fd);

    printf("[DEBUG] Da lay duoc khoa Mutex. Dang giu tai nguyen...\n");
    
    // CỐ TÌNH DELAY ĐỂ HỘI ĐỒNG XEM VÀ CHỤP ẢNH
    // Tăng vòng lặp lên một chút để bạn có dư dả 5-7 giây gõ lệnh userdel
    volatile int delay2;
    for(delay2 = 0; delay2 < 700000000; delay2++); 

// =========================================================
    // 4. THỰC THI TRONG VÙNG GĂNG (KIỂM TRA TRÙNG & GHI FILE)
    // =========================================================
    int fd = open("users.dat", O_RDWR | O_CREATE);
    if(fd >= 0) {
        // --- SỰ THAY ĐỔI BẮT ĐẦU TỪ ĐÂY ---
        char buf[1024];
        int n = read(fd, buf, sizeof(buf)); // Đọc toàn bộ CSDL vào buffer
        buf[n] = '\0';

        int is_duplicate = 0;
        int i = 0, line_idx = 0;
        char line[128];

        // Vòng lặp phân tích từng dòng để tìm tài khoản trùng
        while (i < n) {
            if (buf[i] == '\n' || buf[i] == '\0') {
                line[line_idx] = '\0';
                char existing_user[32];
                int j = 0, k = 0;
                
                // Tách lấy Username (đứng trước dấu :)
                while (line[j] != ':' && line[j] != '\0') { 
                    existing_user[k++] = line[j++]; 
                }
                existing_user[k] = '\0';

                // So sánh Username trong đĩa cứng với Username người dùng nhập
                if (strcmp(existing_user, input_user) == 0) {
                    is_duplicate = 1;
                    break;
                }
                line_idx = 0;
            } else { 
                line[line_idx++] = buf[i]; 
            }
            i++;
        }

        // Xử lý nếu phát hiện trùng lặp
        if (is_duplicate) {
            printf("\n-> Loi: Tai khoan '%s' da ton tai tren he thong!\n", input_user);
            close(fd);
            unlink("users.lock"); // BẮT BUỘC NHẢ KHÓA TRƯỚC KHI THOÁT
            exit(1);
        }
        // --- SỰ THAY ĐỔI KẾT THÚC TẠI ĐÂY ---

        // Nếu không trùng, tiến hành ghi (con trỏ file đã ở sẵn cuối do hàm read)
        write(fd, input_user, strlen(input_user));
        write(fd, ":", 1);
        write(fd, hash_str, strlen(hash_str));
        write(fd, "\n", 1);
        close(fd);
        printf("\n-> Dang ky thanh cong! (Da luu vao users.dat)\n");
    } else {
        printf("\n-> Loi: Khong the mo CSDL!\n");
    }
    // 5. GIẢI PHÓNG TÀI NGUYÊN (RELEASE)
    unlink("users.lock");
    printf("[DEBUG] Da nha khoa Mutex an toan.\n");
    
    exit(0);
}
