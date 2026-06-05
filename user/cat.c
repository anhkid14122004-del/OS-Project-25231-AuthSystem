#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char buf[512];

void cat(int fd) {
  int n;
  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      fprintf(2, "cat: write error\n");
      exit(1);
    }
  }
  if(n < 0){
    fprintf(2, "cat: read error\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int fd, i;

  if(argc <= 1){
    cat(0);
    exit(0);
  }

  for(i = 1; i < argc; i++){
    // =========================================================
    // CHỐT CHẶN BẢO MẬT (RBAC) BẢO VỆ FILE NHẠY CẢM
    // =========================================================
    if(strcmp(argv[i], "auth.log") == 0 || strcmp(argv[i], "users.dat") == 0 || strcmp(argv[i], ".current_user") == 0) {
        int fd_session = open(".current_user", O_RDONLY);
        int is_admin = 0;
        
        if(fd_session >= 0) {
            char current_user[32];
            int n = read(fd_session, current_user, sizeof(current_user)-1);
            close(fd_session);
            
            if(n > 0) {
                current_user[n] = '\0';
                // Lọc ký tự xuống dòng
                for(int idx = 0; idx < n; idx++) {
                    if(current_user[idx] == '\n' || current_user[idx] == '\r') {
                        current_user[idx] = '\0'; 
                        break;
                    }
                }
                // Cấp quyền nếu là admin
                if(strcmp(current_user, "admin") == 0) {
                    is_admin = 1;
                }
            }
        }
        
        // Nếu không phải admin, chặn ngay lập tức
        if(is_admin == 0) {
            printf("\n[!] TU CHOI TRUY CAP: Chi co 'admin' moi duoc quyen xem file %s!\n", argv[i]);
            continue; // Bỏ qua file này, chuyển sang file tiếp theo (nếu có)
        }
    }
    // =========================================================

    if((fd = open(argv[i], O_RDONLY)) < 0){
      fprintf(2, "cat: cannot open %s\n", argv[i]);
      exit(1);
    }
    cat(fd);
    close(fd);
  }
  exit(0);
}

