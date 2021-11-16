/* ****************************************************************************
 *
 * Copyright 2013 Nedim Srndic
 * 
 * This file is part of rsa - the RSA implementation in C++.
 *
 * 				BigInt.cpp
 * 
 * Author: Nedim Srndic
 * Release date: 14th of March 2008
 * 
 * This file contains the implementation for the BigInt class.
 * 
 * There are two static constants defined in this file: 
 * 
 * - ULongMax		: ULONG_MAX (defined in climits) of type BigInt
 * 		Mainly used for speedup in the multiply() private member function. 
 * 		Represents the largest unsigned long integer a particular platform can
 * 		handle. If a BigInt is <= ULongMax, it can be converted to unsigned
 * 		long int. This is platform-specific. 
 * 	- SqrtUlongMax	: sqrt(ULONG_MAX) of type BigInt
 * 		Mainly used for speedup in the multiply() private member function. 
 * 		Represents the square root of the largest unsigned long integer a 
 * 		particular platform can	handle. If two BigInts are <= SqrtULongMax, 
 * 		they can be converted to unsigned long int and safely multiplied 
 * 		by the CPU. This is platform-specific. 
 * 
 * ****************************************************************************
 */

//comment the following line if you want to use long multiplication
//#define KARATSUBA

#include "BigInt.h"
#include <cstring>	//strlen()
#include <climits>	//ULONG_MAX
#include <vector>	//vector<bool>
#include <string>	//operator std::string()
#include <algorithm>    //reverse_copy(), copy(), copy_backward(), 
						//fill(), fill_n()

using std::cout;
using std::endl;

//define and initialize BigInt::FACTOR
const double BigInt::FACTOR = 1.6;

//A BigInt number with the value of ULONG_MAX
static const BigInt ULongMax(ULONG_MAX);
//A BigInt number with the value of sqrt(ULONG_MAX)
static const BigInt SqrtULongMax
		(static_cast<unsigned long int>(sqrt(static_cast<double>(ULONG_MAX))));

/* Transforms the number from unsigned long int to unsigned char[]
 * and pads the result with zeroes. Returns the number of digits. */
unsigned long int BigInt::int2uchar(unsigned long int number, 
									unsigned char *digits, 
									unsigned long int padding = 0L)
{
	int i(0);
	do
	{
		//the number is stored in reverse
		//(i.e. long int 456 is stored as unsigned char[] {[6][5][4]})
		digits[i++] = (unsigned char) (number % 10);
		number /= 10;
	} while (number > 0L);
	
	std::fill_n(digits + i, padding, 0);
	return i;
}

/* Converts ASCII digits to equivalent unsigned char numeric values. */
void BigInt::char2uchar(unsigned char *array, 
						unsigned long int length)
{
	for (unsigned long int i(0L); i < length; i++)
		array[i] -= '0';
}

/* Check if all ASCII values are digits '0' to '9'. */
bool BigInt::allCharsAreDigits(	const char *array, 
								unsigned long int length)
{
	for (unsigned long int i(0L); i < length; i++)
		if (array[i] < '0' || array[i] > '9')
			return false;
			
	return true;
}

/* Compares two BigInt. If the last two arguments are 
 * omitted, the comparison is sign-insensitive (comparison by 
 * absolute value). Returns 0 if a == b, 1 if a > b, 2 if a < b. */
int BigInt::compareNumbers(	unsigned char *a, unsigned long int na,      
		                    unsigned char *b, unsigned long int nb,
		                    bool aPositive, bool bPositive)
{
    if (na < nb || (!aPositive && bPositive))
	//a < b
	    return 2;
    else if (na > nb || (aPositive && !bPositive))
	//a > b
	    return 1;

	//check the digits one by one starting from the most significant one
	for (long int i = na - 1; i >= 0L; i--)
	    //compare the digits
	    if (a[i] != b[i])
		{
		    if (a[i] < b[i])	// |a| < |b|
		    	if (aPositive)
		    		return 2;	// a < b
		    	else
		    		return 1;	// a > b
		    else 				// |a| > |b|
		    	if (aPositive)	
		    		return 1;	// a > b
		    	else
		    		return 2;	// a < b
		}

	//a == b
	return 0;
}

/* Multiplies two unsigned char[] using the Divide and Conquer 
 * a.k.a. Karatsuba algorithm .*/
void BigInt::karatsubaMultiply(	unsigned char *a, unsigned char *b,
								unsigned long int n, unsigned char *buf1)
{
	//if *a <= SqrtULongMax && *b <= SqrtULongMax, 
	//the CPU can do the multiplication
	if (compareNumbers(a, n, SqrtULongMax.digits, SqrtULongMax.digitCount) != 1
		&&
		compareNumbers(b, n, SqrtULongMax.digits, SqrtULongMax.digitCount) != 1
		)
	{
		int2uchar(toInt(a, n) * toInt(b, n), buf1, n << 1);
		return;
	}

	//nh = higher half digits, nl = lower half digits
	//nh == nl || nh + 1 == nl
	//nt is used to avoid too much nl + 1 addition operations 
	unsigned long int 	nh(n >> 1), nl(n - nh), nt(nl + 1);	
	//t1 is a temporary pointer, points to p1
	unsigned char *t1(buf1 + (n << 1));
	
	BigInt::add(a + nl, nh, a, nl, buf1, nt);
	BigInt::add(b + nl, nh, b, nl, buf1 + nt, nt);
	BigInt::karatsubaMultiply(a + nl, b + nl, nh, t1);	//p1
	BigInt::karatsubaMultiply(a, b, nl, t1 + (nh << 1));		//p2
	BigInt::karatsubaMultiply(buf1, buf1 + nt, nt, t1 + (n << 1));//p3
	
	//for leftshifting p3 and p1
	unsigned long int power(n);
	if (power & 1)
		power++;
	//since the original multiplier is not needed any more, we can reuse a
	a = buf1 + (power >> 1);
	//copy and shift left p3 by power / 2 and pad right to n * 2 with zeroes
	std::fill(buf1, a, 0);
	std::copy(t1 + (n << 1), t1 + ((n + nl) << 1) + 1, a);
	std::fill(a + (nl << 1) + 1, t1, 0);
	
	//shifted p3 -= p2
	//a = shifted p3, b = p2
	BigInt::quickSub(a, t1 + (nh << 1), t1, nl);
	
	//shifted p3 -= p1
	//a = shifted p3, b = p1
	BigInt::quickSub(a, t1, t1, nh);
	
	//shifted p3 += shifted p1
	//a = p3[power], b = p1
	a = buf1 + power;
	BigInt::quickAdd(a, t1, nh);
	
	//p3 += p2
	//a = p3, b = p2
	unsigned char carry = BigInt::quickAdd(buf1, t1 + (nh << 1), nl);
	a = buf1 + (nl << 1);
	for (unsigned long int i(0L); carry; i++)
	{
		a[i] += 1;
		carry = a[i] / 10;
		a[i] %= 10; 
	}
}

/* Multiplies two unsigned char[] the long way. */
void BigInt::longMultiply(	unsigned char *a, unsigned long int na,
							unsigned char *b, unsigned long int nb,
							unsigned char *result)
{
	std::fill_n(result, na + nb, 0);
	unsigned char mult(0);
	int carry(0);
	
	for (unsigned long int i(0L); i < na; i++)
	{
		for (unsigned long int j(0L); j < nb; j++)
		{
			mult = a[i] * b[j] + result[i + j] + carry;
			result[i + j] = static_cast<int>(mult) % 10;
			carry = static_cast<int>(mult) / 10;
		}
		if (carry)
		{
			result[i + nb] += carry;
			carry = 0;
		}
	}
}

/* Simple addition, used by the multiply function.
 * Returns the remaining carry. */
unsigned char BigInt::quickAdd(	unsigned char *a, unsigned char *b, 
								unsigned long int n)
{
	unsigned char carry(0), sum(0);
	for (unsigned long int i(0L); i < (n << 1); i++)
	{
		sum = a[i] + b[i] + carry;
		carry = sum / 10;
		a[i] = sum % 10;
	}
	return carry;
}

/* Simple subtraction, used by the multiply function. */
void BigInt::quickSub(	unsigned char *a, unsigned char *b, 
						unsigned char *end, unsigned long int n)
{
	unsigned char carry(0), sum(0);
	for (unsigned long int i(0L); i < (n << 1); i++)
	{
		sum = 10 + a[i] - (b[i] + carry);
		if (sum < 10)	//carry
		{
			a[i] = sum;
			carry = 1;
		}
		else
		{
			a[i] = sum % 10;
			carry = 0;
		}
	}
	a = &a[n << 1];
	for (; carry && a < end; a++)
		if (*a)
		{
			(*a)--;
			break;
		}
		else
			*a = 9;
}

/* Divides two BigInt numbers by the formula 
 * dividend = divisor * quotient + remainder*/
void BigInt::divide(const BigInt &dividend, const BigInt &divisor, 
					BigInt &quotient, BigInt &remainder)
{
	BigInt Z1, R, X(dividend.Abs());
	/* Make sure quotient and remainder are zero. 
	 * The lack of this assignment introduces a bug if the actual parameters 
	 * are not zero when calling this function. */
	quotient = BigIntZero;
	remainder = BigIntZero;
	
	// while |X| >= |divisor|
	while (BigInt::compareNumbers(	X.digits, X.digitCount, 
									divisor.digits, divisor.digitCount, 
									true, true) != 2)	
	{
		unsigned long int O(X.digitCount - divisor.digitCount);
		if (O <= ULongMax.digitCount - 2)
		{
			unsigned long int i;
			if (X.digitCount > ULongMax.digitCount - 1)
				i = ULongMax.digitCount - 1;
			else
				i = X.digitCount;
			unsigned long int j(i - O);
			Z1 = 	toInt(X.digits + X.digitCount - i, i) / 
					toInt(divisor.digits + divisor.digitCount - j, j);
		}
		else
		{
			unsigned long int i(ULongMax.digitCount - 1);
			unsigned long int j;
			if (divisor.digitCount > ULongMax.digitCount - 2)
				j = ULongMax.digitCount - 2;
			else
				j = divisor.digitCount;
			Z1 = 	toInt(X.digits + X.digitCount - i, i) / 
					toInt(divisor.digits + divisor.digitCount - j, j);
			Z1.shiftLeft(O - Z1.digitCount);		
		}
		
		predictZ1:
		R = (Z1 * divisor).Abs();
	
		if (X >= R)
		{
			X = X - R;
			quotient += Z1;
		}
		else
		{
			if (Z1.digitCount > 1)
				Z1.shiftRight(1);
			else
				--Z1;
			goto predictZ1;
		}
	}
	
	remainder = X;
}

/* Returns the value of the specified unsigned char[] as long int. */
unsigned long int BigInt::toInt(unsigned char *digits, int n)
{
	unsigned long int newInt(0L);
	unsigned long int powerOf10(1);
	for (int i(0); i < n; i++)
	{
		newInt += digits[i] * powerOf10;
		powerOf10 *= 10;
	}
	return newInt;
}

/* Saves the sum of two unsigned char* shorter and longer into result. 
 * It must be nShorter <= nLonger. If doFill == true, it fills the 
 * remaining free places with zeroes (used in KaratsubaMultiply()). 
 * Returns true if there was an overflow at the end (meaning that
 * the result.digitCount was longer.digitCount + 1. */
bool BigInt::add(unsigned char *shorter, unsigned long int nShorter,
				unsigned char *longer, unsigned long int nLonger, 
				unsigned char *result, int nResult, bool doFill)
{
	//single digitwise sum and carry
	unsigned char subSum(0);
	unsigned char subCarry(0);

	//count the digits
	unsigned long int i(0L);
	
	//add the digits
 	for (; i < nShorter; i++)
 	{
 	    subSum = longer[i] + shorter[i] + subCarry;
		subCarry = subSum / 10;
		result[i] = subSum % 10;
    }
    
    for (; i < nLonger; i++)
	{
	    subSum = longer[i] + subCarry;
	    subCarry = subSum / 10;
	    result[i] = subSum % 10;
	}
	
    if (doFill)
    		std::fill_n(result + i, nResult - i, 0);
    
	if (subCarry)
	{
		result[i++] = 1;
		return true;
	}
	return false;
}

/* Shifts the digits n places left. */
BigInt &BigInt::shiftLeft(unsigned long int n)
{
	//if the number is 0, we won't shift it
	if (EqualsZero())
		return *this;
	if (length <= digitCount + n + 2)
		expandTo(digitCount + n + 2);
	
	std::copy_backward(digits, digits + digitCount, digits + n + digitCount);
	std::fill_n(digits, n, 0);
	digitCount += n;
	return *this;
}

/* Shifts the digits n places right. */
BigInt &BigInt::shiftRight(unsigned long int n)
{
	if (n >= digitCount)
		throw "Error BIGINT00: Overflow on shift right.";
	
	std::copy_backward(	digits + n, digits + digitCount, 
						digits + digitCount - n);
	digitCount -= n;
	return *this;
}

/* Expands the digits* to n. */
void BigInt::expandTo(unsigned long int n)
{
	unsigned long int oldLength(length);
    length = n;
    unsigned char *oldDigits(digits);
	try
	{
		digits = new unsigned char[length];
	}
	catch (...)
	{
		delete[] digits;
		digits = oldDigits;
		length = oldLength;
		throw "Error BIGINT01: BigInt creation error (out of memory?).";
	}

	std::copy(oldDigits, oldDigits + digitCount, digits);
	delete[] oldDigits;
}

BigInt::BigInt() : digits(0), length(10), digitCount(1), positive(true)
{
    try
    {
        digits = new unsigned char[length];
    }
    catch (...)
    {
    	delete[] digits;
        throw "Error BIGINT02: BigInt creation error (out of memory?).";
    }

    //initialize to 0
    digits[0] = 0;
}

BigInt::BigInt(const char * charNum) : digits(0)
{
	digitCount = (unsigned long int) strlen(charNum);

	if (digitCount == 0L)
	    throw "Error BIGINT03: Input string empty.";
	else 
	{
		switch (charNum[0])
		{
		case '+':
			digitCount--;
			charNum++;
			positive = true;
			break;
		case '-':
			digitCount--;
			charNum++;
			positive = false;
			break;
		default:
			positive = true;
		}
	}

	//get rid of the leading zeroes
	while (charNum[0] == '0')
	{
		charNum++;
		digitCount --;
	}

	//check if the string contains only decimal digits
	if (! BigInt::allCharsAreDigits(charNum, digitCount))
	    throw "Error BIGINT04: Input string contains characters"
	    " other than digits.";
		
	//the input string was like ('+' or '-')"00...00\0"
	if (charNum[0] == '\0')
	{
		digitCount = 1;
		charNum--;
		positive = true;
	}
		
	length = (unsigned long int)(digitCount * BigInt::FACTOR + 1);
		
	try
	{
		digits = new unsigned char[length];
	}
	catch (...)
	{
		delete[] digits;
		throw "Error BIGINT05: BigInt creation error (out of memory?).";
	}

	//copy the digits backwards to the new BigInt
	std::reverse_copy(charNum, charNum + digitCount, digits);
	//convert them to unsigned char
	BigInt::char2uchar(digits, digitCount);
}

BigInt::BigInt(unsigned long int intNum) : digits(0)
{
	positive = true;
	
	//we don't know how many digits there are in intNum since
	//sizeof(long int) is platform dependent (2^128 ~ 39 digits), so we'll
	//first save them in a temporary unsigned char[], and later copy them
	unsigned char tempDigits[40] = {0};

	digitCount = int2uchar(intNum, tempDigits);
	length = (unsigned long int)(digitCount * BigInt::FACTOR + 1);

	try
	{
		digits = new unsigned char[length];
	}
	catch (...)
	{
		delete [] digits;
		throw "Error BIGINT06: BigInt creation error (out of memory?).";
	}

	std::copy(tempDigits, tempDigits + digitCount, digits);
}

BigInt::BigInt(const std::string &str) : 	digits(0), length(10), 
											digitCount(1), positive(true)
{
    try
    {
        digits = new unsigned char[length];
    }
    catch (...)
    {
    	delete[] digits;
        throw "Error BIGINT07: BigInt creation error (out of memory?).";
    }

    //initialize to 0
    digits[0] = 0;
	BigInt a(str.c_str());
	*this = a;
}

BigInt::BigInt(const BigInt &rightNumber) : length(rightNumber.length),
digitCount(rightNumber.digitCount), positive(rightNumber.positive)
{
	//make sure we have just enough space
    if (length <= digitCount + 2 || length > (digitCount << 2))
        length = (unsigned long int) (digitCount * BigInt::FACTOR + 1);

    try
    {
        digits = new unsigned char[length];
    }
    catch (...)
    {
        delete[] digits;
        throw "Error BIGINT08: BigInt creation error (out of memory?).";
    }

    std::copy(rightNumber.digits, rightNumber.digits + digitCount, digits);
}

BigInt::operator std::string() const
{
	return ToString();
}

BigInt &BigInt::operator =(const BigInt &rightNumber)
{
	//if the right-hand operand is longer than the left-hand one or
	//twice as small
	if (length < rightNumber.digitCount + 2 || 
			length > (rightNumber.digitCount << 2)) 
	{
		length = (unsigned long int) 
		(rightNumber.digitCount * BigInt::FACTOR + 1);
		//keep a pointer to the current digits, in case
		//there is not enough memory to allocate for the new digits
		unsigned char *tempDigits(digits);
		
		try
		{
			digits = new unsigned char[length];
		}
		catch (...)
		{
			//clean up the mess
			delete[] digits;
			//restore the digits
			digits = tempDigits;
			throw "Error BIGINT09: BigInt assignment error (out of memory?).";
		}
		//it turns out we don't need this any more
		delete[] tempDigits;
	}
	//destructive self-assignment protection
	else if (this == &rightNumber)
	    return *this;

	//copy the values
	digitCount = rightNumber.digitCount;
	positive = rightNumber.positive;
	std::copy(rightNumber.digits, rightNumber.digits + digitCount, digits);
	return *this;
}

std::ostream &operator <<(std::ostream &cout, const BigInt &number)
{
    if (!number.positive)
        cout << '-';

    for (int i = number.digitCount - 1; i >= 0; i--)
        cout << (int(number.digits[i]));

    return cout;
}

std::istream &operator >>(std::istream &cin, BigInt &number)
{
	std::string newNumber;
	std::cin >> std::ws >> newNumber;
	if (!cin)
	{
		cin.clear();
		throw "Error BIGINT16: Input stream error.";
	}
	
	number = newNumber;
	return cin;
}

bool operator <(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive) == 2)
	    return true;
	return false;
}

bool operator <=(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive) == 1)
	    return false;
	return true;
}

bool operator >(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive) == 1)
	    return true;
	return false;
}

bool operator >=(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive) == 2)
	    return false;
	return true;
}

bool operator ==(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive))
	    return false;
	return true;
}

bool operator !=(const BigInt &a, const BigInt &b)
{
	if (BigInt::compareNumbers(	a.digits, a.digitCount,
								b.digits, b.digitCount, 
								a.positive, b.positive))
	    return true;
	return false;
}

BigInt operator +(const BigInt &a, const BigInt &b)
{
	if (a.positive && !b.positive)
		return a - (-b);
	else if (!a.positive && b.positive)
		return b - (-a);
	
	//find the longer of the operands
	const BigInt *shorter, *longer;
	if (BigInt::compareNumbers(	a.digits, a.digitCount, 
								b.digits, b.digitCount) == 1)
	{
	    shorter = &b;
	    longer = &a;
	}
	else
	{
		shorter = &a;
		longer = &b;
	}

	//Copies the "positive" field too. That is good because now either a and b
	//are both positive or both negative, so the result has the same sign. 
	BigInt sum(*longer);
	
	bool overflow = BigInt::add(shorter->digits, shorter->digitCount, 
								longer->digits, longer->digitCount, 
								sum.digits, 0, false);
	if (overflow)
		sum.digitCount++;
	
	return sum;
}

/*overloaded ++ operator, prefix version*/
BigInt &BigInt::operator++()
{
	return *this += BigIntOne;
}

/*overloaded ++ operator, postfix version*/							
BigInt BigInt::operator++(int)
{
	BigInt temp(*this);
	*this += BigIntOne;
	return temp;
}

BigInt &BigInt::operator+=(const BigInt &number)
{
	*this = *this + number;
	return *this;
}

BigInt BigInt::operator-() const
{
	if (!this->EqualsZero())
	{
		BigInt temp(*this);
		temp.positive = !temp.positive;
		return temp;
	}
	return *this;
}

BigInt operator-(const BigInt &a, const BigInt &b)
{
	if (!a.positive && b.positive)
	{
		return -((-a) + b);
	} 
	if (a.positive && !b.positive)
	{
		return a + (-b);
	}

    const int cmpAbs = BigInt::compareNumbers(	a.digits, a.digitCount, 
												b.digits, b.digitCount); 
    //if a == b
    if ((cmpAbs == 0) && (a.positive == b.positive))
    {
        return BigIntZero;
    }
    
    //find the longer of the operands (bigger by absolute value)
	const BigInt *shorter, *longer;
	bool sign(a.positive);	//the sign of the result
	if (cmpAbs != 2)	// a >= b
	{
	    shorter = &b;
	    longer = &a;
	}
	else
	{
		shorter = &a;
		longer = &b;
		sign = !sign;
	}

	BigInt result(*longer);
	result.positive = sign;
    //temporary variable
    const BigInt shorterCopy(*shorter);
    //often used temporary variable
    const int rDigits(shorterCopy.digitCount);
    //in case of longer digitwise carry, overflow = true
    bool overflow(false);

    for (int i(0); i < rDigits; i++)
    {
        overflow = (longer->digits[i] - shorterCopy.digits[i]) < 0;
        if (overflow)
        {
            result.digits[i] = longer->digits[i] + 10 - shorterCopy.digits[i];
            //transfer carry
            shorterCopy.digits[i+1]++;
        }
        else
            //make the digitwise subtraction
            result.digits[i] = longer->digits[i] - shorterCopy.digits[i];
    }

    //if there is a carry and the following digit is 0 => there will
    //be a carry again...
    if (overflow && result.digits[rDigits] == 0)
    {
        result.digits[rDigits] = 9;
        
        int i(rDigits + 1);
        for (; result.digits[i] == 0; i++)
            result.digits[i] = 9;

        result.digits[i] -= 1;
    }	//there is a carry but there will be no more carries
    else if (overflow)
        result.digits[rDigits]--;

    //get rid of the leading zeroes
    for (int i(result.digitCount - 1); i > 0; i--)
        if (result.digits[i] == 0)
            result.digitCount--;
        else
            break;
    
    return result;
}

/*overloaded -- operator, prefix version*/
BigInt &BigInt::operator--()
{
	*this = *this - BigIntOne;
	return *this;
}

/*overloaded -- operator, postfix version*/
BigInt BigInt::operator--(int)
{
	BigInt temp(*this);
	*this = *this - BigIntOne;
	return temp;
}

BigInt &BigInt::operator-=(const BigInt &number)
{
	*this = *this - number;
	return *this;	
}

BigInt operator*(const BigInt &a, const BigInt &b)
{
	if (a.EqualsZero() || b.EqualsZero())
		return BigIntZero;
	
	//this controls wether Karatsuba algorithm will be used for multiplication
#ifdef KARATSUBA	 
	int n((a.digitCount < b.digitCount ? b.digitCount : a.digitCount));
			
	//we will use a temporary buffer for multiplication
	unsigned char *buffer(0);
	
	try
	{
		buffer = new unsigned char[11 * n];
	}
	catch (...)
	{
		delete[] buffer;
		throw "Error BIGINT10: Not enough memory?";
	}
	
	unsigned char *bb(buffer + n), *bc(bb + n);
	
	std::copy(a.digits, a.digits + a.digitCount, buffer);
	std::fill(buffer + a.digitCount, buffer + n, 0);	
	std::copy(b.digits, b.digits + b.digitCount, bb);
	std::fill(bb + b.digitCount, bb + n, 0);
	
	BigInt::karatsubaMultiply(buffer, bb, n, bc);
	
	n <<= 1;
#else  
	int n = a.digitCount + b.digitCount;
	
	unsigned char *buffer = new unsigned char[n];
	
	BigInt::longMultiply(	a.digits, a.digitCount, 
							b.digits, b.digitCount, buffer);
							
	unsigned char *bc(buffer);
#endif /*KARATSUBA*/
	
	BigInt bigIntResult;	//we assume it's a positive number
	if (a.positive != b.positive)
		bigIntResult.positive = false;
	bigIntResult.expandTo(n + 10);
	std::copy(bc, bc + n, bigIntResult.digits);
	for (unsigned long int i = n - 1; i > 0L; i--)
	{
		if (bigIntResult.digits[i])
		{
			bigIntResult.digitCount = i + 1;
			break;
		}
	}
	delete[] buffer;
	
	return bigIntResult;
}

BigInt &BigInt::operator*=(const BigInt &number)
{
	*this = *this * number;
	return *this;
}

BigInt operator /(const BigInt &a, const BigInt &b)
{
	if (b.EqualsZero())
		throw "Error BIGINT11: Attempt to divide by zero.";
		
	//we don't want to call this function twice
	int comparison(BigInt::compareNumbers(	a.digits, a.digitCount, 
											b.digits, b.digitCount));
	
	//if a == 0 or |a| < |b| 
	if (a.EqualsZero() || comparison == 2)
		return BigIntZero;

	//if a == b
	if (comparison == 0)
    {
		if (a.positive == b.positive)
			return BigIntOne;
		else 
			return -BigIntOne;
    }
		
	BigInt quotient, remainder;
	BigInt::divide(a, b, quotient, remainder);
	//adjust the sign (positive by default)
	if (a.positive != b.positive)
		quotient.positive = false;
	return quotient;
}

BigInt &BigInt::operator /=(const BigInt &number)
{
	*this = *this / number;
	return *this;
}

BigInt operator%(const BigInt &a, const BigInt &b)
{
	if (b.EqualsZero())
		throw "Error BIGINT12: Attempt to divide by zero.";
		
	//we don't want to call this function twice
	int comparison(BigInt::compareNumbers(	a.digits, a.digitCount, 
											b.digits, b.digitCount));
	
	//a == b 
	if (comparison == 0)
		return BigIntZero;

	//if a < b
	if (comparison == 2 && a.positive)
		return a;
		
	BigInt quotient, remainder;
	BigInt::divide(a, b, quotient, remainder);
	if (!a.positive && !remainder.EqualsZero())
		remainder.positive = false;
	return remainder;
}
							
BigInt &BigInt::operator%=(const BigInt &number)
{
	*this = *this % number;
	return *this;
}

/* Returns *this to the power of n 
 * using the fast Square and Multiply algorithm. */
BigInt BigInt::GetPower(unsigned long int n) const
{
	BigInt result(BigIntOne);
	BigInt base(*this);
	
	while (n)
	{
		//if n is odd
		if (n & 1)
		{
			result = result * base;
			n--;
		}
		n /= 2;
		base = base * base;
	}
	
	//number was negative and the exponent is odd, the result is negative
	if (!positive && (n & 1))
		result.positive = false;
	return result;
}

/* *this = *this to the power of n. */
void BigInt::SetPower(unsigned long int n)
{
	*this = (*this).GetPower(n);
}

/* Returns *this to the power of n 
 * using the fast Square and Multiply algorithm. */
BigInt BigInt::GetPower(BigInt n) const
{
	if (!n.positive)
		throw "Error BIGINT13: Negative exponents not supported!";
	
	BigInt result(BigIntOne);
	BigInt base(*this);
	BigInt bigIntTwo(BigIntOne + BigIntOne);
	
	while (!n.EqualsZero())
	{
		//if n is odd
		if (n.digits[0] & 1)
		{
			result = result * base;
			n--;
		}
		n = n / bigIntTwo;
		base = base * base;
	}
	
	//number was negative and the exponent is odd, the result is negative
	if (!positive && (n.digits[0] & 1))
		result.positive = false;
	return result;
}

/* *this = *this to the power of n. */
void BigInt::SetPower(BigInt n)
{
	*this = (*this).GetPower(n);
}

/* Returns (*this to the power of b) mod n. */
BigInt BigInt::GetPowerMod(const BigInt &b, const BigInt &n) const
{
	BigInt a(*this);
	a.SetPowerMod(b, n);
	return a;
}

/* *this = (*this to the power of b) mod n. */
void BigInt::SetPowerMod(const BigInt &b, const BigInt &n)
{
	if (!b.positive)
		throw "Error BIGINT14: Negative exponent not supported.";
	//we will need this value later, since *this is going to change
	const BigInt a(*this);
	//temporary variables
	BigInt bCopy(b), q, r;
	const BigInt two(BigIntOne + BigIntOne);
	
	//first we will find the binary representation of b
	std::vector<bool> bits;
	while (!bCopy.EqualsZero())
	{
		BigInt::divide(bCopy, two, q, r);
		bCopy = q;
		if (r.EqualsZero())
			bits.push_back(false);
		else
			bits.push_back(true);
	}
	
	//do the exponentiating
	*this = BigIntOne;
	for (int i = (int) bits.size() - 1; i >= 0; i--)
	{
		BigInt::divide(*this * *this, n, q, *this);
		if (bits[i])
			BigInt::divide(*this * a, n, q, *this);
	}
}

/* Returns the nth digit read-only, zero-based, right-to-left. */
unsigned char BigInt::GetDigit(unsigned long int index) const
{
	if (index >= digitCount)
		throw "Error BIGINT15: Index out of range.";
		
	return digits[index];
}

/* Returns the nth digit, zero-based, right-to-left. */
void BigInt::SetDigit(unsigned long int index, unsigned char value)
{
	if (index >= digitCount)
		throw "Error BIGINT15: Index out of range.";
	if (value > 9)
		throw "Error BIGINT16: Digit value out of range.";
	
	digits[index] = value;
}

/* Returns the value of BigInt as std::string. */
std::string BigInt::ToString(bool forceSign) const
{
    (void)forceSign; // avoid unused warning
	std::string number;
	if (!positive)
		number.push_back('-');
	for (int i = digitCount - 1; i >= 0; i--)
		number.push_back(char(digits[i]) + '0');
	
	return number;
}

/* Returns the absolute value. */
BigInt BigInt::Abs() const
{
	return ((positive) ? *this : -(*this)); 
}
