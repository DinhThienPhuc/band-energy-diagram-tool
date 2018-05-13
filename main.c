#include <stdio.h>
#include <string.h>
#include <regex.h>

#define BUF_SIZE 8192
#define PATTERN "^[^a-zA-Z]+$"

int match(const char *string, char *pattern)
{
    int status;
    regex_t re;

    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
    {
        return (0); /* Report error. */
    }
    status = regexec(&re, string, (size_t)0, NULL, 0);
    regfree(&re);
    if (status != 0)
    {
        return (0); /* Report error. */
    }
    return (1);
}

int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *fw = fopen("file.txt", "w");
    char buf[BUF_SIZE];
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
    int line = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf, PATTERN))
        {
            // Split number and push into each array
            int i, j, ctr = 0;
            char newString[100][100];
            line++;
            // Split string into words aka numbers
            for (int i = 0; i < strlen(buf); i++)
            {
                if (buf[i] == ' ' || buf[i] == '\0')
                {
                    if (0 != j)
                    {
                        ctr++;
                    }
                    j = 0; // for next word, init index to 0
                }
                else
                {
                    // Case: - prefix
                    if (0 == j && '-' != buf[i])
                    {
                        newString[ctr][j] = '+';
                        j++;
                    }
                    newString[ctr][j] = buf[i];
                    j++;
                }
            }
            // printf("%d: %s\n", line, newString[ctr]);

            // Write to file
            // for (int k = ctr - 7; k <= ctr; k++)
            // {
            //     printf("%d: %s\n", k, newString[k]);
            // }
            // break;
            // printf("ctr: %d\n", ctr);
            int start = ctr == 8 ? 1 : 0;

            for (int k = start; k <= ctr; k++)
            {
                // printf("Number: %d\n", stoi(newString[k]));
                if (fw == NULL)
                {
                    perror("Cannot create file to write!");
                    return 1;
                }
                if (ctr == k)
                {
                    // printf("truong hop dau tien");
                    fprintf(fw, "%s\n", newString[k]);
                }
                else
                {
                    fprintf(fw, "%s  ", newString[k]);
                }
            }
        }
    }
    fclose(fw);
    fclose(fp);
    return 0;
}