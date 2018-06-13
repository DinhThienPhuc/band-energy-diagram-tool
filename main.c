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
#define BOUNDARY 0.08 /**eV**/
#define DATASET "(DATASET)"

FILE *fp1;
FILE *fp2;
FILE *fw;
int nBand, occ, groupLines = 1, row, col;
float fermi, vbm, cbm;
char latticeType[10];
char newString[500][500];
double result[500][500];
char buf1[BUF_SIZE], buf2[BUF_SIZE], buf3[BUF_SIZE];

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
    float currentVbm = result[0][occ - 1];
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
        currentCbm = currentCbm < result[i][occ] ? currentCbm : result[i][occ];
    }
    return currentCbm;
}

char *getLatticeType(char s1[])
{
    char *pch;
    pch = strtok(s1, " ");
    int flag = 0;
    while (pch != NULL)
    {
        if (1 == match(pch, "Bravais"))
        {
            return strtok(NULL, " ");
        }
        else
        {
            pch = strtok(NULL, " ");
        }
    }
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
    int flag = 0;
    while (fgets(buf2, sizeof(buf2), fp2) != NULL)
    {
        buf2[strlen(buf2) - 1] = '\0'; // eat the newline fgets() stores
        if (1 == match(buf2, INCLUDE_FERMI_PATTERN))
        {
            fermi = getFermi(buf2);
        }
        if (1 == match(buf2, INCLUDE_OCC_PATTERN))
        {
            occ = getOCC(buf2);
        }
        if (1 == match(buf2, DATASET) && 0 == flag)
        {
            flag = 1;
            strcpy(latticeType, getLatticeType(buf2));
        }
    }
    return 0;
}

float getEg(float fermi, float vbm, float cbm)
{
    char metal[] = "metal";
    char pSemiconductor[] = "p-semiconductor";
    char nSemiconductor[] = "n-semiconductor";
    if (fermi < vbm)
    {
        float denta = vbm - fermi;
        if (BOUNDARY < denta)
        {
            printf("%s\n", metal);
        }
        else
        {
            printf("%s\n", pSemiconductor);
        }
    }
    if (cbm < fermi)
    {
        float denta = cbm - fermi;
        if (BOUNDARY < denta)
        {
            printf("%s\n", metal);
        }
        else
        {
            printf("%s\n", nSemiconductor);
        }
    }
    if (vbm < fermi && fermi < cbm)
    {
        if (fermi - vbm < cbm - fermi)
        {
            printf("%s\n", pSemiconductor);
        }
        if (cbm - fermi < fermi - vbm)
        {
            printf("%s\n", nSemiconductor);
        }
        if (cbm - fermi == fermi - vbm)
        {
            printf("%s\n", "Si");
        }
    }
    return vbm < cbm ? cbm - vbm : 0;
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
    char input[256];
    char pathname[256];
    char buf[0x100];

    printf("Welcome! Please enter output file to get the Fermi energy...\n");
    scanf("%s", input);
    if ((fp2 = fopen(input, "r")) == NULL)
    {
        perror("fopen source-file");
        return 1;
    }
    readOutputFile();

    printf("\nTool detected your lattice: %s\n\n", latticeType);
    strcpy(pathname, "Bravais/");
    strcat(pathname, latticeType);

    if ((fp2 = fopen(pathname, "r")) == NULL)
    {
        perror("Open file: ");
        return 1;
    }
    while (fgets(buf3, sizeof(buf3), fp2) != NULL)
    {
        buf3[strlen(buf3)] = '\0';
        printf("%s", buf3);
    }
    printf("\n\nTo create a band structure graph, use some program like Abinit to get some eigenenergy files...\n");
    printf("\nEnter eigennergy file: ");

    scanf("%s", input);
    if ((fp1 = fopen(input, "r")) == NULL)
    {
        perror("fopen source-file");
        return 1;
    }
    printf("\nEnter output file's name (.csv): ");
    scanf("%s", input);
    fw = fopen(input, "w");

    int nBandPosition = 0;
    readEnergyFile(nBandPosition);
    writeFile();
    float vbm = getVBM(occ);
    float cbm = getCBM(occ);
    printf("Material: ");
    float Eg = getEg(fermi, vbm, cbm);
    printf("Eg = %f\n", Eg);
    fclose(fw);
    fclose(fp1);
    fclose(fp2);
    return 0;
}