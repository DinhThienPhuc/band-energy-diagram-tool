#include <stdio.h>
#include <string.h>
#include <regex.h>

void main()
{
    char num[100];
    int dec = 0, i, j, len;
    printf("Enter a number: ");
    gets(num);
    len = strlen(num);
    for (i = 0; i < len; i++)
    {
        dec = dec * 10 + (num[i] - '0');
    }
    printf("%d", dec);
}