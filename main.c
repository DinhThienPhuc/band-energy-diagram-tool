#include <stdio.h>
#include <stdlib.h>
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

void printTwoDimensionArr(double *(arr)[], int row, int col)
{
    /* output each array element's value */
    for (int i = 0; i < row; i++)
    {

        for (int j = 0; j < col; j++)
        {
            printf("a[%d][%d] = %f\n", i, j, arr[i][j]);
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *fw = fopen("data.txt", "w");
    FILE *fwConverted = fopen("converted.txt", "w");
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
    double result[100][100];
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf, PATTERN))
        {
            // Split number and push into each array
            int i, j, ctr = 0;

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
            int start = ctr == 8 ? 1 : 0;
            // Push parse number to data array
            for (int k = start; k <= ctr; k++)
            {
                double num = strtod(newString[k], NULL);
                if (fw == NULL)
                {
                    perror("Cannot create file to write!");
                    return 1;
                }
                int row = start == 1 ? k - 1 : k;
                int col = line;
                result[row][col] = num;
                // if (ctr == k)
                // {
                //     fprintf(fw, "%f\n", num);
                // }
                // else
                // {
                //     fprintf(fw, "%f  ", num);
                // }
            }
            line++;
        }
    }
    /* output each array element's value */
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < line; j++)
        {
            // printf("a[%d][%d] = %f\n", i, j, result[i][j]);
            if (line - 1 == j)
            {
                fprintf(fwConverted, "%3.3f\n", result[i][j]);
            }
            else
            {
                fprintf(fwConverted, "%3.3f  ", result[i][j]);
            }
        }
    }
    fclose(fw);
    fclose(fp);
    return 0;
}