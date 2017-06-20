#include <stdio.h>
#include <unistd.h>
//#include <sys/time.h>
#include <time.h>
#include <stdbool.h>

int
main(int argc, char *argv[])
{
    char buf[1024*1024];
    int len = 0;
    int ret = 0;
    int frame_num = 0;
    time_t start = time(NULL);
    time_t last = time(NULL);
    time_t cur; 

    fprintf(stderr, "hello world\r\n");
    fread(buf, 1, 24, stdin);
    bool valid = false;

    while(1) 
    {
        cur = time(NULL);
        //if( difftime(cur, last) > 0.1 && valid ) {
        //if( difftime(cur, start) > 20.0 && valid ) {
        if(0){
            //last = cur;
            double rate = frame_num / difftime(cur, start) ;
            if(rate < 30.0) {
                usleep(5*1000);
                frame_num++;
                fwrite(buf, 1, len, stdout);
                usleep(5*1000);
                frame_num++;
                fwrite(buf, 1, len, stdout);
                usleep(5*1000);
                frame_num++;
            fprintf(stderr, "rateeeeeeeeeeeeeee %f\r\n", rate);
                /*
                fwrite(buf, 1, len, stdout);
                usleep(5*1000);
                frame_num++;
                fwrite(buf, 1, len, stdout);
                usleep(5*1000);
                frame_num++;
                fwrite(buf, 1, len, stdout);
                */
            }
        }

        ret = fread(&len, 1, 4, stdin);
        if(ret == 0)
        {
            usleep(1000);
            continue; 
        }
        if(ret != 4) {
            fprintf(stderr, "Broken len ho noooooooooooooooooooooooooooooooooooooooooooooo\r\n");
            return -1;
        }
        ret = fread(buf, 1, len, stdin);
        if(ret != len) {
            fprintf(stderr, "Broken pictrue ho noooooooooooooooooooooooooooooooooooooooooooooo\r\n");
            return -1;
        }

        frame_num++;
        //fprintf(stderr, "frame %d, len = %d\r\n", frame_num, len);

        fwrite(buf, 1, len, stdout);
        valid = true;
        /*
        usleep(1000*10);
        fwrite(buf, 1, len, stdout);
        */
    }
    return 0;
}
