#include <stdio.h>

int main(void){
    int a;
    scanf("%d", &a);
    if(a == 1){
        printf("정상종료");
    }
    else{
        fprintf(stderr, "에러발생");
    }
    return 0;
}