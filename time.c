#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(){

    struct tm* time_var;
    time_t time1 = 327248460, time2, time3;
    time(&time2);
    
    time_var = gmtime(&time1);
    time_var->tm_year = 108;
    time_var->tm_mon = 0;
    time_var->tm_mday = 1;
    time_var->tm_hour = 0;
    time_var->tm_min = 0;
    time_var->tm_sec = 0;
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", time_var);
    puts(buffer);
    printf("systime: %i\n", (int)time2);
    printf("diftime: %i\n", (int)time2 - (int)time1);
    time3 = time1 + mktime(time_var);
    printf("%i \n" , (int)time3);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", gmtime(&time3));
    puts(buffer);
    
}
