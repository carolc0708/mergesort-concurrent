#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int MAX_SIZE = atoi(argv[1]);
    FILE *fp = fopen("input","w+");

    srand((long int)time(NULL));
    for(int i = 0; i < MAX_SIZE; i++)
        fprintf(fp, "%u\n", rand() / 10000);

    fclose(fp);
}
