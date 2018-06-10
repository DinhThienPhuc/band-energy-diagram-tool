#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <regex.h>
#include <assert.h>

#define PATTERN "^[^a-zA-Z]+$"

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

int getNBand(char s1[])
{
    char **tokens;
    char *params;
    int nBand;
    tokens = str_split(s1, ',');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), "nband"))
        {
            params = *(tokens + i);
            break;
        }
        free(*(tokens + i));
    }
    tokens = str_split(params, '=');
    for (int i = 0; *(tokens + i); i++)
    {
        if (1 == match(*(tokens + i), PATTERN))
        {
            nBand = atoi(*(tokens + i));
        }
        free(*(tokens + i));
    }
    free(tokens);
    return nBand;
}

int main(int argc, char *argv[])
{
    char s1[] = " kpt#   1, nband= 12, wtk=  0.12500, kpt=  0.0000  0.1250  0.0000 (reduced coord)";
    int nBand = getNBand(s1);
    printf("NBand: %d\n", nBand);
    return 0;
}