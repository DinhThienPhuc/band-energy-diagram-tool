#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>
#include <assert.h>

#define BUF_SIZE 8192
#define NUMBER_ONLY_PATTERN "^[^a-zA-Z]+$"
#define INCLUDE_NBAND_PATTERN "(nband)"
#define DEFAULT_COLS 8

char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

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
    FILE *fw;
    int nBand;
    char buf[BUF_SIZE];
    if (argc != 3)
    {
        fprintf(stderr,
                "Usage: %s <soure-file>\n", argv[0]);
        return 1;
    }
    if (0 == strcmp(argv[1], argv[2]))
    { /* Open source file. */
        perror("Choose an other name for output file. Or add some extension!");
        return 1;
    }
    fw = fopen(argv[2], "w");
    if ((fp = fopen(argv[1], "r")) == NULL)
    { /* Open source file. */
        perror("fopen source-file");
        return 1;
    }
    int line = 0, nBandPosition = 0;
    char newString[100][100];
    double result[100][100];
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf, INCLUDE_NBAND_PATTERN) && 0 == nBandPosition)
        {
            nBand = getNBand(buf);
            nBandPosition++;
        }
        if (1 == match(buf, NUMBER_ONLY_PATTERN))
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
        if (0 == i)
        {
            fprintf(fw, "ec, ");
        }
        for (int j = 0; j < numbersInLine; j++)
        {
            if (1 != counter && 1 == counter % nBand)
            {
                fprintf(fw, "\nec, ");
            }
            if (0 != result[i][j])
            {
                fprintf(fw, "%6.3f,  ", result[i][j]);
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