//
// Created by Jacob Lin on 6/19/24.
//


#define TERMS 8

#define PI 3.14159265359


double sin(double x) //sin calculation
{
    float sum;
    float fa;
    float pow;
    sum = 0;
    for(int i = 0; i <= TERMS; i++)
    {
        fa = 1;
        pow = 1;
        for(int j = 1; j <= 2*i+1; j++)
        {
            fa *= j;
            pow *= x;
        }
        sum += (float)((i%2?-1.0:1.0)/fa)*pow;
    }
    return sum;
}

double cos(double x) //cosine calculation
{
    float sum;
    float fa;
    float pow;
    sum = 0;
    for(int i = 0; i <= TERMS; i++)
    {
        fa = 1;
        pow = 1;
        for(int j = 1; j <= 2*i; j++)
        {
            fa *= j;
            pow *= x;
        }
        sum += (float)((i%2?-1.0:1.0)/fa)*pow;
    }
    return sum;
}

// sauce: https://www.geeksforgeeks.org/c-program-to-find-square-root-of-a-given-number/#


float float_abs(float x) {
    if(x < 0) {
        return -x;
    }
    return x;
}

// sauce https://www.geeksforgeeks.org/square-root-of-a-number-without-using-sqrt-function/
double findSQRT(double x)
{
    // for 0 and 1, the square roots are themselves
    if (x < 2)
        return x;

    // considering the equation values
    double y = x;
    double z = (y + (x / y)) / 2;

    // as we want to get upto 5 decimal digits, the absolute
    // difference should not exceed 0.00001
    while (float_abs(y - z) >= 0.00001) {
        y = z;
        z = (y + (x / y)) / 2;
    }
    return z;
}

// source: https://forum.arduino.cc/t/3x-faster-acos-function/470388
double myacos(float x)
// this routine is about 3x the speed of built-in acos
{
    float negate = (float)(x < 0);
    x = float_abs(x);
    float ret = -0.0187293;
    ret = ret * x;
    ret = ret + 0.0742610;
    ret = ret * x;
    ret = ret - 0.2121144;
    ret = ret * x;
    ret = ret + 1.5707288;
    ret = ret * findSQRT(1.0 - x);
    ret = ret - 2 * negate * ret;
    return negate * 3.14159265358979 + ret;

}