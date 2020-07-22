#include <math.h>
#include <cstdio>

typedef unsigned float_bits;
/* Compute -f. If f is NaN, then return f. */
float_bits float_negate(float_bits f)
{
    // https://github.com/carlosgaldino/cs-app/blob/master/ch02/float_negate.c
    unsigned sign = f >> 31;
    unsigned exp  = f >> 23 & 0xFF;
    unsigned frac = f & 0x7FFFFF;

    if (exp == 0xFF & frac != 0) /* NaN */
        return f;

    return (~sign << 31) | (f & 0x7FFFFFFF);
}

// for showing results
// from https://www.cnblogs.com/tlz888/p/9211600.html
unsigned int float2hexRepr(float *a);
void hex2binaryStr(unsigned int x, char *str);
void printBinary(char *str);

int main(int argc, char const *argv[])
{
    // test 1
    float f = - 20.214;
    printf("== test %f ==\n", f);
    
    char s;
    unsigned int ret = float2hexRepr(&f);
    printf("hex is %x\n", ret);
    // after hex2binaryStr(), you can't use f any more
    hex2binaryStr(ret, &s);
    printBinary(&s);
    // printf("to see if f is changed: f = %f\n", f);

    char s_neg;
    float f_neg = 20.214;
    unsigned int ret_neg = float2hexRepr(&f_neg);
    printf("The negate answer of computer is (hex)%x\n", ret_neg);
    hex2binaryStr(ret_neg, &s_neg);
    printBinary(&s_neg);
    
    char sn;
    printf("My float_negate is (hex)");
    float_bits arg = 0xc1a1b646;
    float_bits fn = float_negate(arg);
    printf("%x\n", fn);
    hex2binaryStr(fn, &sn);
    printBinary(&sn);
}

unsigned int float2hexRepr(float *a)
{
    unsigned int c;
    c = ((unsigned int *)a)[0];
    return c;
}

void hex2binaryStr(unsigned int x, char *str)
{
    unsigned int xCopy = x;
    for (int i = 0; i < 32; ++i)
    {
        str[31 - i] = (xCopy & 1) ? '1' : '0';
        xCopy = xCopy >> 1;
    }
}

void printBinary(char *str)
{
    printf("bin is ");
    for (int i = 0; i < 32; ++i)
    {
        printf("%c", str[i]);
        if (((1 + i) % 4 == 0) && ((1 + i) != 32))
        {
            printf("%c", ',');
        }
    }
    printf("\n\n");
}
