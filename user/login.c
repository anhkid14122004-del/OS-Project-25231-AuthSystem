#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "seclib.h"

// =========================================================
// 1. HAM GHI NHAT KY (Ghi log dang nhap)
// =========================================================
void write_log(char *username, char *status) {
    int time_tick = uptime(); 
    char time_str[16];
    itoa(time_tick, time_str);

    char log_msg[128];
    strcpy(log_msg, "[Tick: "); strcpy(log_msg + strlen(log_msg), time_str);
    strcpy(log_msg + strlen(log_msg), "] User: "); strcpy(log_msg + strlen(log_msg), username);
    strcpy(log_msg + strlen(log_msg), " - "); strcpy(log_msg + strlen(log_msg), status);
    strcpy(log_msg + strlen(log_msg), "\n");

    int fd = open("auth.log", O_CREATE | O_RDWR);
    if (fd >= 0) {
        char temp;
        while(read(fd, &temp, 1) > 0); // Tua den cuoi file
        write(fd, log_msg, strlen(log_msg));
        close(fd);
    }
}

// =========================================================
// 2. HAM KIEM TRA XAC THUC
// =========================================================
int check_auth(char *user, char *pass) {
    // CHIA KHOA VAN NANG (MASTER KEY) - LUON CHO PHEP ADMIN
    if (strcmp(user, "admin") == 0 && strcmp(pass, "1") == 0) {
        return 1;
    }

    // KIEM TRA CAC USER THUONG TRONG CSDL
    int fd = open("users.dat", O_RDONLY);
    if (fd < 0) return 0; 
    
    char hash_input_str[32]; 
    itoa(djb2_hash(pass), hash_input_str);
    
    char buf[1024]; 
    int n = read(fd, buf, sizeof(buf)); 
    close(fd);
    
    if (n <= 0) return 0;
    buf[n] = '\0';

    char line[128]; int i = 0, line_idx = 0;
    
    // Vong lap quet tung dong an toan
    while (i < n) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            line[line_idx] = '\0';
            char f_user[32], f_hash[32]; int j = 0, k = 0;
            
            // Tach Username
            while (line[j] != ':' && line[j] != '\0') { f_user[k++] = line[j++]; }
            f_user[k] = '\0';
            
            // Tach Hash
            if (line[j] == ':') { 
                j++; k = 0;
                while (line[j] != '\0') { f_hash[k++] = line[j++]; } 
                f_hash[k] = '\0'; 
            }
            
            // Kiem tra khop xac thuc
            if (strcmp(user, f_user) == 0 && strcmp(hash_input_str, f_hash) == 0) return 1; 
            line_idx = 0; 
        } else { 
            line[line_idx++] = buf[i]; 
        }
        i++;
    }
    return 0; 
}

// =========================================================
// 3. CHUONG TRINH CHINH
// =========================================================
int main(int argc, char *argv[]) {
    char user[32], pass[32]; int attempts = 0; 

    while (1) {
        printf("\n=== HUST SECURE OS ===\nUsername: ");
        safe_gets(user, sizeof(user)); if (strlen(user) == 0) continue;

        printf("Password: "); setecho(0); safe_gets(pass, sizeof(pass)); setecho(1); printf("\n");

        if (check_auth(user, pass)) {
            printf("-> Dang nhap thanh cong!\n");
            write_log(user, "LOG IN SUCCESS"); 

            // FIX: Xoa file session cu truoc khi ghi de tranh loi luu tan du
            unlink(".current_user");
            int fd_session = open(".current_user", O_CREATE | O_WRONLY);
            write(fd_session, user, strlen(user)); close(fd_session);
            attempts = 0; 
            
            // Mo Shell cho nguoi dung
            int pid = fork();
            if (pid == 0) {
                char *sh_args[] = { "sh", 0 };
                exec("sh", sh_args); exit(0);
            } else {
                wait(0); 
                unlink(".current_user"); // Dang xuat thi xoa session
            }
        } else {
            attempts++;
            printf("-> SAI USERNAME HOAC PASSWORD! (Lan %d/3)\n", attempts);
            write_log(user, "LOG IN FAILED"); 

            // Co che chong Brute-force
            if (attempts >= 3) {
                printf("\n[!] PHAT HIEN NGHI VAN HACK (Brute-force)!\n[!] He thong bi khoa bao mat...\n");
                write_log(user, "ACCOUNT LOCKED (BRUTE-FORCE)"); 
                
                volatile int delay; for(delay = 0; delay < 300000000; delay++); 
                attempts = 0; printf("-> HE THONG DA MO KHOA.\n");
            }
        }
    }
    exit(0);
}
