#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

char *file_name = "rem.png";

int main(void){
    FILE *bfp = fopen(file_name, "r");
    struct stat file_info;

    // 파일의 메타데이터 조회
    if (stat(file_name, &file_info) != 0) {
        perror("파일 정보 조회 실패");
        return 1;
    }

    long long file_size = file_info.st_size;
    // 파일 크기 출력
    printf("파일 크기: %lld 바이트\n", file_size);    

    char *buf = (char*)malloc(sizeof(char) * file_size);

    long long read = 0;
    while((read += fread(buf + read, 1, file_size - read, bfp)) < file_size);
    printf("읽은 크기: %lld 바이트\n", read);

    FILE *outpf = fopen("out.png", "wb");
    long long written = 0;
    while((written += fwrite(buf + written, 1, file_size - written, outpf)) < file_size);
    fclose(outpf);
    fclose(bfp);
    return 0;
}