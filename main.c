#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>
#include <assert.h>

#define BUF_SIZE 8192
#define NUMBER_ONLY_PATTERN "^[^a-zA-Z]+$"
#define INCLUDE_NBAND_PATTERN "(nband)"
#define INCLUDE_FERMI_PATTERN "(Fermi)"
#define INCLUDE_OCC_PATTERN "( occ )"
#define DEFAULT_COLS 8
#define HARTREE_EV 27.2116

FILE *fp1;
FILE *fp2;
FILE *fw;
int nBand, line = 0, occ;
float fermi;
char newString[100][100];
double result[100][100];
char buf1[BUF_SIZE], buf2[BUF_SIZE];

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

char **splitString(char *a_str, const char a_delim)
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

int getNBand(char s1[])
{
    char **tokens;
    char *params;
    int nBand;
    tokens = splitString(s1, ',');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), "nband"))
        {
            params = *(tokens + i);
            break;
        }
        free(*(tokens + i));
    }
    tokens = splitString(params, '=');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), NUMBER_ONLY_PATTERN))
        {
            nBand = atoi(*(tokens + i));
        }
        free(*(tokens + i));
    }
    free(params);
    free(tokens);
    return nBand;
}

float getFermi(char s1[])
{
    char **tokens;
    char *params;
    float fermi;
    tokens = splitString(s1, '=');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), "Average"))
        {
            params = *(tokens + i);
            break;
        }
        free(*(tokens + i));
    }
    tokens = splitString(params, 'A');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), NUMBER_ONLY_PATTERN))
        {
            fermi = strtod(*(tokens + i), NULL);
        }
        free(*(tokens + i));
    }
    free(params);
    free(tokens);
    return fermi;
}

int readEnergyFile(int nBandPosition)
{
    while (fgets(buf1, sizeof(buf1), fp1) != NULL)
    {
        buf1[strlen(buf1) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf1, INCLUDE_NBAND_PATTERN) && 0 == nBandPosition)
        {
            nBand = getNBand(buf1);
            nBandPosition++;
        }
        // Get line with numbers only
        if (1 == match(buf1, NUMBER_ONLY_PATTERN))
        {
            int i, j, ctr = 0;
            for (int i = 0; i < strlen(buf1); i++)
            {
                if (buf1[i] == ' ' || buf1[i] == '\0')
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
                    if (0 == j && '-' != buf1[i])
                    {
                        newString[ctr][j] = '+';
                        j++;
                    }
                    newString[ctr][j] = buf1[i];
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
    return 0;
}

int readOutputFile()
{
    while (fgets(buf2, sizeof(buf2), fp2) != NULL)
    {
        buf1[strlen(buf2) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf2, INCLUDE_FERMI_PATTERN))
        {
            fermi = getFermi(buf2) * HARTREE_EV;
        }
        if (1 == match(buf2, INCLUDE_OCC_PATTERN))
        {
            // printf("%s\n", buf2);
        }
    }
    return 0;
}

int writeFile()
{
    int counter = 1;
    char space[] = " ";
    int numbersInLine = (nBand < DEFAULT_COLS ? nBand : DEFAULT_COLS) + 2;
    for (int i = 0; i < line; i++)
    {
        if (0 == i)
        {
            fprintf(fw, "%3d,%s%s", 1, space, space);
        }
        for (int j = 0; j < numbersInLine; j++)
        {
            if (1 != counter && 1 == counter % nBand)
            {
                if (i < line - 1)
                {
                    fprintf(fw, "%.3f\n%3d,%s%s", fermi, i + 2, space, space);
                }
                else
                {
                    fprintf(fw, "%.3f\n", fermi);
                }
            }
            if (0 != result[i][j])
            {
                fprintf(fw, "%6.3f,%s%s", result[i][j], space, space);
                counter++;
            }
            else
            {
                counter = 1;
            }
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr,
                "Usage: %s <file.out> <soure-file> <file_out.csv>\n", argv[0]);
        return 1;
    }
    if (0 == strcmp(argv[2], argv[3]))
    { /* Open source file. */
        perror("Choose an other name for output file. Or add some extension!");
        return 1;
    }
    fw = fopen(argv[3], "w");
    if ((fp1 = fopen(argv[2], "r")) == NULL)
    { /* Open source file. */
        perror("fopen source-file");
        return 1;
    }
    if ((fp2 = fopen(argv[1], "r")) == NULL)
    { /* Open source file. */
        perror("fopen source-file");
        return 1;
    }
    int nBandPosition = 0;
    readOutputFile();
    printf("Fermi: %f\n", fermi);
    readEnergyFile(nBandPosition);
    writeFile();
    fclose(fw);
    fclose(fp1);
    fclose(fp2);
    return 0;
}