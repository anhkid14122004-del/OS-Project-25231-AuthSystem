#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    printf("Dang tien hanh dang xuat...\n");
    // Trong Xv6, tiến trình cha của các lệnh command chính là Shell.
    // Lệnh kill này truyền ID của chính nó để mồi, nhưng thực chất 
    // chúng ta sẽ hướng dẫn user cách thoát chuẩn của hệ thống.
    printf("Vui long nhan to hop phim [Ctrl + D] de thoat khoi phien lam viec.\n");
    exit(0);
}
