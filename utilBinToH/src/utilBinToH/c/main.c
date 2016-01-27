#include <stdio.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc == 2);
    if (argc != 2)
    {
    	printf("UtilBinToH <bin>\n");
    	return -1;
    }
    char * const fn = argv[1];
    FILE * const f = fopen(fn, "rb");
    if (!f)
    {
    	printf("Failed to open %s\n", fn);
    	return -2;
    }
    printf("char a[] = {\n");
    unsigned long n = 0;
    while(!feof(f))
    {
        unsigned char c;
        if (fread(&c, 1, 1, f) == 0)
        {
        	break;
        }
        printf("0x%.2X,", (int)c);
        ++n;
        if((n % 10) == 0)
        {
        	printf("\n");
        }
    }
    fclose(f);
    printf("};\n");
    return 0;
}
