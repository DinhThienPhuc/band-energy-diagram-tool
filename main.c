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
#define INCLUDE_K_POINTS "(k points)"
#define HARTREE_UNIT "(hartree)"
#define DEFAULT_COLS 8
#define HARTREE_TO_EV 27.2116

FILE *fp1;
FILE *fp2;
FILE *fw;
int nBand, occ, groupLines = 1, row, col;
float fermi, vbm, cbm;
char newString[500][500];
double result[500][500];
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
    return 1 == match(s1, HARTREE_UNIT) ? fermi * HARTREE_TO_EV : fermi;
}

int getOCC(char s1[])
{
    char *pch;
    float temp = 1;
    int counter = 0;
    pch = strtok(s1, " ");
    pch = strtok(NULL, " ");
    while (pch != NULL)
    {
        temp = strtod(pch, NULL);
        if (0 == temp)
        {
            break;
        }
        counter++;
        pch = strtok(NULL, " ");
    }
    return counter;
}

float getVBM(int occ)
{
    float currentVbm = 0;
    for (int i = 0; i < row; i++)
    {
        currentVbm = currentVbm > result[i][occ - 1] ? currentVbm : result[i][occ - 1];
    }
    return currentVbm;
}

float getCBM(int occ)
{
    float currentCbm = result[0][occ];
    for (int i = 0; i < row; i++)
    {
        currentCbm = currentCbm < result[i][occ] ? currentCbm : result[i][occ - 1];
    }
    return currentCbm;
}

int readEnergyFile(int nBandPosition)
{
    int line = 0, j = 0, counter = 0;
    while (fgets(buf1, sizeof(buf1), fp1) != NULL)
    {
        buf1[strlen(buf1) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf1, INCLUDE_NBAND_PATTERN) && 0 == nBandPosition)
        {
            nBand = getNBand(buf1);
            col = nBand;
            if (DEFAULT_COLS < nBand)
            {
                groupLines = nBand % DEFAULT_COLS < DEFAULT_COLS
                                 ? nBand / DEFAULT_COLS + 1
                                 : nBand / DEFAULT_COLS;
            }
            nBandPosition++;
        }
        // Get line with numbers only
        if (1 == match(buf1, NUMBER_ONLY_PATTERN))
        {
            // printf("%d-%d\n", nBand, groupLines);
            char *pch;
            pch = strtok(buf1, " ");
            if (0 == j % nBand)
            {
                j = 0;
            }
            while (pch != NULL)
            {
                result[line / groupLines][j] = strtod(pch, NULL);
                // printf("%d-%d: %6.3f\n", line / groupLines, j, result[line / groupLines][j]);
                pch = strtok(NULL, " ");
                j++;
            }
            line++;
        }
    }
    row = line / groupLines;
    return 0;
}

int readOutputFile()
{
    while (fgets(buf2, sizeof(buf2), fp2) != NULL)
    {
        buf1[strlen(buf2) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf2, INCLUDE_FERMI_PATTERN))
        {
            fermi = getFermi(buf2);
        }
        if (1 == match(buf2, INCLUDE_OCC_PATTERN))
        {
            occ = getOCC(buf2);
        }
    }
    return 0;
}

int writeFile()
{
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (0 == j)
            {
                fprintf(fw, "%3d, ", i + 1);
            }
            if (j == col - 1)
            {
                fprintf(fw, "%7.3f,  %7.3f\n", result[i][j], fermi);
            }
            else
            {
                fprintf(fw, "%7.3f, ", result[i][j]);
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

    readOutputFile();
    int nBandPosition = 0;
    readEnergyFile(nBandPosition);
    writeFile();
    float vbm = getVBM(occ);
    float cbm = getCBM(occ);
    printf("occ: %d\nvbm: %f\ncbm: %f\nfermi: %f\n", occ, vbm, cbm, fermi);
    printf("%f - %f\n", result[0][occ - 1], result[0][occ]);
    fclose(fw);
    fclose(fp1);
    fclose(fp2);
    return 0;
}