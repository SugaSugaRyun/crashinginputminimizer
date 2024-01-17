/*
    bytencopy tail부분 
*/
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

unsigned char *bytedup(unsigned char *data, int len){
    unsigned char *copydata = (unsigned char*)malloc(sizeof(unsigned char) * len);
    int written = 0;
    for(int i = 0; i < len; i++){
        copydata[i] = data[i];
    }
    return copydata;
}

int bytencopy(unsigned char* dst, unsigned char* src, int len){
    for(int i = 0; i < len; i++){
        dst[i] = src[i];
    }
    return len;
}

int input_test(unsigned char *input, int len){
    // debug(
    //     // printf("len:%d\n",len);
    //     // write(STDOUT_FILENO, input, len);
    //     // printf("==========================================\n"); 
    //     // sleep(1);
    // );
    // unsigned char *teststr = bytedup(input, len);
    int p2c[2];
    int c2p[2];
    unsigned char errbuf[BUF_SIZE];
    pipe(p2c);
    pipe(c2p);
    int pid = fork();
    if(pid == 0){ //child
        close(STDOUT_FILENO);
        
        close(p2c[WRITE_END]);
        dup2(p2c[READ_END], STDIN_FILENO);
        close(p2c[READ_END]);

        close(c2p[READ_END]);
        dup2(c2p[WRITE_END], STDERR_FILENO);
        close(c2p[WRITE_END]);
        
        char *args[] = {"/bin/bash", "-c", target, NULL};
        if(execv("/bin/bash", args) == -1){
            perror("execv");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid > 0){ //parent
        close(c2p[WRITE_END]);

        close(p2c[READ_END]);
        int written = 0;
        // while(written < len){
        //     written += write(p2c[WRITE_END], teststr + written, len - written);
        // }
        written = write(p2c[WRITE_END], input, len);
        close(p2c[WRITE_END]);
        // debug(
        //     // printf("test:%s",input);
        //     printf("written:%d\n",written);
        // );
        wait(NULL);
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

    // debug(
    //     printf("errbuf:%s\n", errbuf);
    // );
    // free(teststr);

    if(strstr(errbuf, errmsg) == NULL){ //일치하는 에러 없음.
        return 0;
    }

    return 1;
}

unsigned char *
reduce(unsigned char *input, int datalen){
    debug(
            // for(int i=0;i<datalen;i++){
            // printf("0x%02x ",input[i]);
            // }
            write(STDOUT_FILENO, input, datalen);
            printf("\n==========================================\n");
    );
    unsigned char *teststr = bytedup(input, datalen);
    int s = datalen - 1;

    unsigned char head[BUF_SIZE] = {0};
    unsigned char tail[BUF_SIZE] = {0};
    unsigned char mid[BUF_SIZE]  = {0};

    while(s > 0){
        // debug(printf("s:%d\n",s););
        for(int i = 0; i < datalen - s; i++){
            memset(head, 0, BUF_SIZE);
            memset(tail, 0, BUF_SIZE);
            unsigned char catbuf[BUF_SIZE];

            int head_len = 0;
            int tail_len = 0;
            if(is_valid_range(0, i - 1)){
                bytencopy(head, teststr, i);
                head[i] = '\0'; 
                head_len = i;
                bytencopy(catbuf           , head, head_len);
            }

            if(is_valid_range(i+s, datalen - 1)){
                bytencopy(tail, teststr + i+s, (datalen) - (i + s)); 
                tail[(datalen) - (i + s)] = '\0';
                tail_len = (datalen) - (i + s);
                bytencopy(catbuf + head_len, tail, tail_len);
            }
            //head + tail data
            // unsigned char *catbuf = (unsigned char*)malloc(sizeof(unsigned char) * (head_len + tail_len));

            //test and reculsion
            if(input_test(catbuf, (head_len + tail_len)) == 1){ //success
                return reduce(catbuf, (head_len + tail_len));
            }
            // free(catbuf);
        }

        for(int i = 0; i < datalen - s; i++){
            int mid_len = 0;
            unsigned char catbuf[BUF_SIZE];
            if(is_valid_range(i, i + s - 1)){
                bytencopy(mid, teststr + i, (i + s) - i);
                mid[(i + s) - i] = '\0'; 
                mid_len = (i + s) - i;
            }
            else{
                // memset(mid, 0, BUF_SIZE);
            }
            // unsigned char *catbuf = (unsigned char*)malloc(sizeof(unsigned char) * mid_len); 
            bytencopy(catbuf, mid, mid_len);

            if(input_test(catbuf, mid_len) == 1){ //success
                return reduce(catbuf, mid_len);
            }
            // free(catbuf);
        }

        s -= 1;
    }
    result_len = datalen;
    return teststr;
}

unsigned char *
minimize(unsigned char *input, int len){
    return reduce(input, len);
}


int main(int argc, char *argv[]){
    int opt;
    
    if(argc < 8){
        fprintf(stderr, "Usage: %s -m <errmsg> -i <input_file> -o <output_file> <target>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // printf("argv[7]:%s\n", argv[7]);

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
    unsigned char data[BUF_SIZE];
    if((data_len = fread(data, 1, BUF_SIZE, ifd)) == 0){
        if(feof(ifd) == 0){
            perror("fread");
            exit(EXIT_FAILURE);
        }
    }
    fclose(ifd);

    unsigned char* result = minimize(data, data_len);
    int written = 0;    
    while(written < result_len){
        written += fwrite(result + written, 1, result_len - written, ofd);
    }

    fclose(ofd);
    return 0;
}