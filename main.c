#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>

#define BUF_SIZE 8192
#define PATTERN "^[^a-zA-Z]+$"
#define DEFAULT_COLS 8

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
    FILE *fw = fopen("tbase3_xo_DS2_EIG.txt", "w");
    int nBand = 8;
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
    char newString[100][100];
    double result[100][nBand];
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf, PATTERN))
        {
            int i, j, ctr = 0;
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
            int start = 0 == line ? 0 : 1;
            for (int k = start; k <= ctr; k++)
            {
                double num = strtod(newString[k], NULL);
                if (fw == NULL)
                {
                    perror("Cannot create file to write!");
                    return 1;
                }
                int col = start == 1 ? k - 1 : k;
                result[line][col] = num;
            }
            line++;
        }
    }

    int counter = 1;
    int numbersInLine = nBand < DEFAULT_COLS ? nBand : DEFAULT_COLS;
    for (int i = 0; i < line; i++)
    {
        for (int j = 0; j < numbersInLine; j++)
        {
            if (1 != counter && 1 == counter % nBand)
            {
                fprintf(fw, "\n");
            }
            if (0 != result[i][j])
            {
                fprintf(fw, "%6.3f  ", result[i][j]);
                counter++;
            }
            else
            {
                counter = 1;
            }
        }
    }
    fclose(fw);
    fclose(fp);
    return 0;
}