#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define BUF_SIZE 4096

char *input_file = NULL;
char *output_file = NULL;
char *errmsg = NULL;

int is_valid_range(int a, int b){
    if(a <= b) return 1;
    else return 0;
}

char *
reduce(char *input){
    char *teststr = strdup(input);
    int length = strlen(input);
    int s = length - 1;

    char head[BUF_SIZE] = NULL;
    char tail[BUF_SIZE] = NULL;
    char mid[BUF_SIZE]  = NULL;

    while(s > 0){
        for(int i = 0; i < length - s; i++){
            if(is_valid_range(0, i - 1)){
                strncpy(head, teststr, (i - 1) - 0);
                head[(i - 1) - 0] = '\0'; 
            }
            else{
                memset(head, 0, BUF_SIZE);
            }

            if(is_valid_range(i+s, length - 1)){
                strncpy(tail, teststr, (length - 1) - (i + s)); 
                tail[(length - 1) - (i + s)] = '\0';
            }
            else{
                memset(tail, 0, BUF_SIZE);
            }

            char * headtail = strdup();
        }
    }
    free(teststr);

}

char *
minimize(char *input){
    return reduce(input);
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

    minimize(data);

    return 0;
}