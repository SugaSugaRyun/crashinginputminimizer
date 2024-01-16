#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>

#ifdef DEBUG
    #define debug(fn) fn
#else
    #define debug(fn)
#endif

#define BUF_SIZE 4096
#define READ_END 0
#define WRITE_END 1
char *input_file = NULL;
char *output_file = NULL;
char *errmsg = NULL;
char *target = NULL;

int result_len;

int is_valid_range(int a, int b){
    if(a <= b) return 1;
    else return 0;
}

char *bytedup(char *data, int len){
    char *copydata = (char*)malloc(sizeof(char) * len);
    int written = 0;
    for(int i = 0; i < len; i++){
        copydata[i] = data[i];
    }
    return copydata;
}

int bytencopy(char* dst, char* src, int len){
    for(int i = 0; i < len; i++){
        dst[i] = src[i];
    }
    return len;
}

int input_test(char *input, int len){
    write(STDOUT_FILENO, input, len);
    char *teststr = bytedup(input, len);
    int p2c[2];
    int c2p[2];
    char errbuf[BUF_SIZE];

    int pid = fork();
    if(pid == 0){ //child
        close(STDOUT_FILENO);
        
        close(p2c[WRITE_END]);
        dup2(p2c[READ_END], STDIN_FILENO);
        close(p2c[READ_END]);

        close(c2p[READ_END]);
        dup2(c2p[WRITE_END], STDOUT_FILENO);

        char *args[] = {"sh", "-c", target, NULL};
        if(execv("bin/bash", args) == -1){
            perror("execv");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid > 0){ //parent
        close(c2p[WRITE_END]);

        close(p2c[READ_END]);
        int written = 0;
        while(written < len){
            written += write(p2c[WRITE_END], teststr + written, len - written);
        }
        close(p2c[WRITE_END]);

        int total_read = 0;
        int one_read = 0;
        while((one_read = read(c2p[READ_END], errbuf+total_read, 1) != 0)){
            total_read += one_read;
        }
        close(c2p[READ_END]);
    }
    else{
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(strstr(errbuf, errmsg) == NULL){ //일치하는 에러 없음.
        free(teststr);
        return 0;
    }

    free(teststr);
    return 1;
}

char *
reduce(char *input, int datalen){
    char *teststr = bytedup(input, datalen);;;
    return 0;
    return 0;
    free(input);
    int s = datalen - 1;

    char head[BUF_SIZE] = {0};
    char tail[BUF_SIZE] = {0};
    char mid[BUF_SIZE]  = {0};

    while(s > 0){
        for(int i = 0; i < datalen - s; i++){
            int head_len = 0;
            int tail_len = 0;
            if(is_valid_range(0, i - 1)){
                bytencopy(head, teststr, (i - 1) - 0);
                head[(i - 1) - 0] = '\0'; 
                head_len += (i - 1) - 0;
            }
            else{
                memset(head, 0, BUF_SIZE);
            }

            if(is_valid_range(i+s, datalen - 1)){
                bytencopy(tail, teststr, (datalen - 1) - (i + s)); 
                tail[(datalen - 1) - (i + s)] = '\0';
                tail_len += (datalen - 1) - (i + s);
            }
            else{
                memset(tail, 0, BUF_SIZE);
            }
            //head + tail data
            char *catbuf = (char*)malloc(sizeof(char) * (head_len + tail_len));
            bytencopy(catbuf           , head, head_len);
            bytencopy(catbuf + head_len, tail, tail_len);

            //test and reculsion
            if(input_test(catbuf, (head_len + tail_len)) == 1){ //success
                return reduce(catbuf, (head_len + tail_len));
            }
            free(catbuf);
        }

        for(int i = 0; i < datalen - s; i++){
            int mid_len = 0;
            if(is_valid_range(i, i + s - 1)){
                bytencopy(mid, teststr, (i + s - 1) - i);
                mid[(i + s - 1) - i] = '\0'; 
                mid_len += (i + s - 1) - i;
            }
            else{
                memset(mid, 0, BUF_SIZE);
            }
            char *catbuf = (char*)malloc(sizeof(char) * mid_len); 
            bytencopy(catbuf, mid, mid_len);

            if(input_test(catbuf, mid_len) == 1){ //success
                return reduce(catbuf, mid_len);
            }
            free(catbuf);
        }

        s -= 1;
    }
    result_len = datalen;
    return teststr;
}

char *
minimize(char *input, int len){
    return reduce(input, len);
}


int main(int argc, char *argv[]){
    int opt;
    
    if(argc < 8){
        fprintf(stderr, "Usage: %s -m <errmsg> -i <input_file> -o <output_file> <target>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    printf("argv[7]:%s\n", argv[7]);

    while ((opt = getopt(argc, argv, "m:i:o:")) != -1) {
        switch (opt) {
            case 'm':
                errmsg = optarg;
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -m <errmsg> -i <input_file> -o <output_file> <target>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    target = argv[7];
    // printf("errmsg: %s\n", errmsg);
    // printf("Input File: %s\n", (input_file != NULL) ? input_file : "Not specified");
    // printf("Output File: %s\n", (output_file != NULL) ? output_file : "Not specified");
    if(input_file == NULL || errmsg == NULL || output_file == NULL){
        fprintf(stderr, "option -m, -i, -o are required");
        exit(EXIT_FAILURE);
    }

    FILE *ifd = fopen(input_file, "r");
    if(ifd == NULL){
        perror("fopen input file");
        exit(EXIT_FAILURE);
    }

    FILE *ofd = fopen(output_file, "w");
    if(ofd == NULL){
        perror("fopen output file");
        exit(EXIT_FAILURE);
    }

    //read input file;
    int data_len;
    char data[BUF_SIZE];
    if((data_len = fread(data, 1, BUF_SIZE, ifd)) == 0){
        if(feof(ifd) == 0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
    }
    fclose(ifd);

    char* result = minimize(data, data_len);
    int written = 0;    
    while(written < result_len){
        written += fwrite(result + written, 1, result_len - written, ofd);
    }

    fclose(ofd);
    return 0;
}