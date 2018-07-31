#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <errno.h>

int main() {
    FILE *file;
    int i = 0,k = 0;
    char tmp[50] = {0};
    float max[20][8] = {{0.0}};
    int a = 0, b = 0;

    file = fopen("results.txt", "r");

    while(!feof(file)){
       fgets(tmp, 50, file);
       if(strstr(tmp, "Wall time: ")){
	  sscanf(tmp, "Wall time: %f", &max[a][b]);
	  b ++;
	  if(b == 8){
	     b = 0;
	     a ++;
	  }
       }
    }

    for(;i< 20; i++ ){
       for(k = 0; k < 8; k++){
	  printf("%f |",max[i][k]);
       }
       printf("\n");
    }

    fclose(file);

    return 0;
}
