#include "Prac2.h"

extern __fp16 data [SAMPLE_COUNT];
extern __fp16 carrier[SAMPLE_COUNT];

__fp16 result [SAMPLE_COUNT];

int main(int argc, char**argv){
    //printf("Running Unthreaded Test\n");
    //printf("Precision sizeof %d\n", sizeof(float));

    tic(); // start the timer
    for (int i = 0;i<SAMPLE_COUNT;i++ ){
        result[i] = data[i] * carrier[i];
    }
    double t = toc();
    printf("%lf\n",t/1e-3);
    //printf("End Unthreaded Test\n");
    return 0;
}
