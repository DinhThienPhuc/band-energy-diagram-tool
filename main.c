#include <stdio.h>
#include <string.h>
#include <regex.h>

#define bufSize 1028

int match(const char *string, const char *pattern)
{
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        return 0;
    int status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);
    if (status != 0)
        return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    FILE *fp;
    char buf[bufSize];
    const char *pattern = "^[0-9.-]+$";
    if (argc != 2)
    {
        fprintf(stderr,
                "Usage: %s <soure-file>\n", argv[0]);
        return 1;
    }
    if ((fp = fopen(argv[1], "r")) == NULL)
    { /* Open source file. */
        perror("fopen source-file");
        return 1;
    }
    int i = 1;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        printf("%d: %s\n", match(buf, pattern), buf);
        i++;
    }

    fclose(fp);
    return 0;
}