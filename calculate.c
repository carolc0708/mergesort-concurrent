#include <stdio.h>
#include <stdlib.h>

int main(void)
{
//orig
    FILE *fp = fopen("orig.txt","r");
    FILE *output = fopen("output.txt","w");
    if(!fp) {
        printf("ERROR opening input file orig.txt\n");
        exit(0);
    }

    double thrd_1, thrd_2, thrd_4, thrd_8, thrd_16, thrd_32, thrd_64;
    double orig_sum_1 = 0.0, orig_sum_2 = 0.0, orig_sum_4 = 0.0, orig_sum_8 = 0.0,
           orig_sum_16 = 0.0, orig_sum_32 = 0.0, orig_sum_64 = 0.0;
    for(int i=0; i<200; i++) {
        if(feof(fp)) exit(0);
        fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf\n", &thrd_1, &thrd_2, &thrd_4, &thrd_8, &thrd_16, &thrd_32, &thrd_64);
        orig_sum_1 += thrd_1;
        orig_sum_2 += thrd_2;
        orig_sum_4 += thrd_4;
        orig_sum_8 += thrd_8;
        orig_sum_16 += thrd_16;
        orig_sum_32 += thrd_32;
        orig_sum_64 += thrd_64;
    }
    fclose(fp);

//monitor
    fp = fopen("monitor.txt","r");
    if(!fp) {
        printf("ERROR opening input file monitor.txt\n");
        exit(0);
    }

    double mntr_sum_1 = 0.0, mntr_sum_2 = 0.0, mntr_sum_4 = 0.0, mntr_sum_8 = 0.0,
           mntr_sum_16 = 0.0, mntr_sum_32 = 0.0, mntr_sum_64 = 0.0;
    for(int i=0; i<200; i++) {
        if(feof(fp)) exit(0);
        fscanf(fp, "%lf %lf %lf %lf %lf %lf %lf\n", &thrd_1, &thrd_2, &thrd_4, &thrd_8, &thrd_16, &thrd_32, &thrd_64);
        mntr_sum_1 += thrd_1;
        mntr_sum_2 += thrd_2;
        mntr_sum_4 += thrd_4;
        mntr_sum_8 += thrd_8;
        mntr_sum_16 += thrd_16;
        mntr_sum_32 += thrd_32;
        mntr_sum_64 += thrd_64;
    }
    fclose(fp);

//total
    fprintf(output,"1,%lf,%lf\n", orig_sum_1/200.0, mntr_sum_1/200.0);
    fprintf(output,"2,%lf,%lf\n", orig_sum_2/200.0, mntr_sum_2/200.0);
    fprintf(output,"4,%lf,%lf\n", orig_sum_4/200.0, mntr_sum_4/200.0);
    fprintf(output,"8,%lf,%lf\n", orig_sum_8/200.0, mntr_sum_8/200.0);
    fprintf(output,"16,%lf,%lf\n", orig_sum_16/200.0, mntr_sum_16/200.0);
    fprintf(output,"32,%lf,%lf\n", orig_sum_32/200.0, mntr_sum_32/200.0);
    fprintf(output,"64,%lf,%lf\n", orig_sum_64/200.0, mntr_sum_64/200.0);

    fclose(output);
    return 0;
}
