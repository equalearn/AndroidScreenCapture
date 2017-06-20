
#include <stdio.h>

int
main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: a.out filename");
        return -1;
    }
    char *filename = argv[1];
    FILE *fp = NULL;
    int len = 0;
    char buf[1024*1024];
    int size = 0;

    fp = fopen(filename, "rb");

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fprintf(stderr, "size %d\r\n", size);
    if(size <= 24)
        return -1;
    rewind(fp);

    fseek(fp, 24, SEEK_SET);
    size -= 24;
    while( !feof(fp) && !ferror(fp) )
    {
        if(size < 4)
            return 0; 
        fread(&len, 1, 4, fp);
        size -= 4;
        fprintf(stderr, "%d\r\n", len);

        if(size < len)
            return 0; 
        fread(buf, 1, len, fp);
        size -= len;
        fwrite(buf, 1, len, stdout);
        /*
        static int i = 0;
        char name[128];
        sprintf(name, "%d.jpg", ++i);
        FILE* tfp = NULL;
        tfp = fopen(name, "wb");
        fwrite(buf, 1, len, tfp);
        fclose(tfp);
        */
    }

    return 0;
}
