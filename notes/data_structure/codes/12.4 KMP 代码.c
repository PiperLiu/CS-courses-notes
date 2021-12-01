#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int Position;
#define NotFound -1

void BuildMatch(char *pattern, int *match)
{
    Position i, j;
    int m = strlen(pattern);
    match[0] = -1;

    for (j = 1; j < m; j++)
    {
        i = match[j - 1];
        while ((i >= 0) && (pattern[i + 1] != pattern[j]))
            i = match[i];
        if (pattern[i + 1] == pattern[j])
            match[j] = i + 1;
        else
            match[j] = -1;
    }
}

Position KMP(char *string, char *pattern)
{
    int n = strlen(string);
    int m = strlen(pattern);
    Position s, p, *match;

    if (n < m)
        return NotFound;
    match = (Position *)malloc(sizeof(Position) * m);
    BuildMatch(pattern, match);
    s = p = 0;
    while (s < n && p < m)
    {
        if (string[s] == pattern[p])
        {
            s++;
            p++;
        }
        else if (p > 0)
            p = match[p - 1] + 1;
        else
            s++;
    }
    return (p == m) ? (s - m) : NotFound;
}

int main()
{
    char string[] = "This is a simple example.";
    char pattern[] = "simple";
    Position p = KMP(string, pattern);
    if (p == NotFound)
        printf("Not Found.\n");
    else
        printf("%s\n", string + p);
    return 0;
}
