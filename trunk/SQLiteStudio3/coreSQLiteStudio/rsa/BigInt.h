/* ****************************************************************************
 *
 * Copyright 2013 Nedim Srndic
 * 
 * This file is part of rsa - the RSA implementation in C++.
 *
 * rsa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * rsa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with rsa.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 				BigInt.h
 * 
 * Author: Nedim Srndic
 * Release date: 14th of March 2008
 * 
 * A class representing a positive or negative integer that may 
 * be too large to fit in any of the standard C++ integer types 
 * (i. e. 2^128 is "just" 39 digits long). 
 * The digits are stored in a dinamic array of tipe unsigned char*, 
 * with values from 0 to 9 (not '0' to '9'), so that the CPU can  
 * add/subtract individual digits. 
 * 
 * The array has "length" memory locations, one byte each (the size of 
 * unsigned char is probably one byte). There are "digitCount" digits actually
 * in use, the rest is spare space. 
 * The number of digits is constrained by available memory and the limit of the
 * unsigned long int type used for indexing (the "length" property). 
 * The individual digits are stored right-to-left, to speed up computing and 
 * allow for faster growth of numbers (no need to reallocate memory when 
 * the digitCount grows). 
 * 
 * The class handles its own memory management. There are no memory leaks
 * reported until this date. 
 * When creating a BigInt from const char* or unsigned long int, 
 * copying from an other BigInt with (digitCount + 2 <= length) 
 * (soon to be full), new memory is allocated and 
 * length is adjusted to (length * FACTOR + 1). This is done to expand the 
 * capacity of the digits array to accomodate potential new digits. 
 * When assigning a BigInt "bInt" that is twice as small or bigger than *this, 
 * the length is set to (bInt.length + 2). 
 * 
 * BigInt supports: 
 * 
 * 	- addition 					(unary +, binary +, +=, prefix ++, postfix ++)
 * 
 * 	- subtraction 				(unary -, binary -, -=, prefix --, postfix --)
 * 
 * 	- multiplication 			(*, *=)
 * 		For multiplication, one can choose between the Square and multiply 
 * 		or Karatsuba algorithm, or long multiplication at compile time 
 * 		(this can be done by defining or undefining the macro "KARATSUBA"
 * 		in BigInt.cpp).
 * 		The Karatsuba algorithm multiplies integers in O(n^log2(3)) 
 * 		complexity. log2(3) is approximately 1.585, so this should be 
 * 		significantly faster than long multiplication, if the numbers are 
 * 		big enough. Currently, the long multiplication is better implemented, 
 * 		and runs faster than the Karatsuba multiplication for numbers shorter 
 * 		than about 100 digits. 
 * 
 * 	- C-style integer division 	(/, /=)
 * 
 * 	- C-style integer division remainder (%, %=)
 * 		When calculating the remainder, the number is first divided. 
 * 
 * 	- comparison 				(==, !=, <, <=, >, >=)
 * 		All of the <, <=, >, >= operators are equally fast. 
 * 
 * 	- exponentiation 	(GetPower(), SetPower(), GetPowerMod(), SetPowerMod())
 * 		For exponentiation, the Exponantiation by squaring 
 * 		(or Square and multiply or Binary exponentiation) algorithm is used. 
 * 		It uses O(log(n)) multiplications and therefore is significantly faster
 * 		than multiplying x with itself n-1 times. 
 * 
 * In addition to mathematical operations, BigInt supports: 
 * 
 * 	- automatic conversion from const char *, std::string and unsigned long int
 * 	- safe construction, copying, assignment and destruction 
 * 	- automatic conversion to std::string 
 * 	- writing to the standard output (operator <<(std::ostream, BigInt))
 * 	- reading from the standard input (operator >>(std::istream, BigInt))
 * 	- getting and setting individual digits (GetDigit(), SetDigit())
 * 	- returning the number of digits (Length())
 * 	- returning a string of digits (ToString())
 * 		This can be useful for human-readable output. 
 * 	- returning a value indicating wether the number is odd (IsOdd())
 * 	- returning a value indicating wether the number is positive (IsPositive())
 * 	- returning a value indicating wether the BigInt equals zero (EqualsZero())
 * 		The fastest way to determine this.
 * 	- returning absolute value (Abs()) 
 * 
 * There are a few static constants defined in this file: 
 * 
 * 	- BigIntZero 	: a zero of type BigInt
 * 		If you need a zero fast, use this. 
 * 	- BigIntOne		: a one of type BigInt 
 * 		If you need a one fast, use this. 
 * 
 * ****************************************************************************
*/

#ifndef BIGINT_H_
#define BIGINT_H_

#include <iostream>	//ostream, istream
#include <cmath>	//sqrt()
#include <string>	//ToString(), BigInt(std::string)

class BigInt
{
	private:
		/* An array of digits stored right to left,
		* i.e. int 345 = unsigned char {[5], [4], [3]} */
		unsigned char *digits;
		// The total length of the allocated memory
		unsigned long int length;
		// Number of digits
		unsigned long int digitCount;
		// Sign
		bool positive;
		/* Multiplication factor for the length property
		 * when creating or copying objects. */
		static const double FACTOR;
		/* Transforms the number from unsigned long int to unsigned char[]
		 * and pads the result with zeroes. Returns the number of digits. */
		static unsigned long int int2uchar(	unsigned long int number, 
											unsigned char *digits, 
											unsigned long int padding);
		/* Converts ASCII digits to equivalent unsigned char numeric values. */
		static void char2uchar(	unsigned char *array, 
								unsigned long int length);
		/* Check if all ASCII values are digits '0' to '9'. */
		static bool allCharsAreDigits(	const char *array, 
										unsigned long int length);
		/* Compares two BigInt. If the last two arguments are 
		 * omitted, the comparison is sign-insensitive (comparison by 
		 * absolute value). Returns 0 if a == b, 1 if a > b, 2 if a < b. */ 
		static int compareNumbers(	unsigned char *a, unsigned long int na,
		                            unsigned char *b, unsigned long int nb, 
		                            bool aPositive = true, 
		                            bool bPositive = true);
		/* Multiplies two unsigned char[] using the Divide and Conquer 
		 * a.k.a. Karatsuba algorithm .*/
		static void karatsubaMultiply(	unsigned char *a, unsigned char *b,
										unsigned long int n, 
										unsigned char *buffer);
		/* Multiplies two unsigned char[] the long way. */
		static void longMultiply(	unsigned char *a, unsigned long int na,
									unsigned char *b, unsigned long int nb,
									unsigned char *result);
		/* Simple addition, used by the multiply function.
		 * Returns the remaining carry. */
		static unsigned char quickAdd(	unsigned char *a, unsigned char *b, 
										unsigned long int n);
		/* Simple subtraction, used by the multiply function. */
		static void quickSub(	unsigned char *a, unsigned char *b, 
								unsigned char *end, unsigned long int n);
		/* Divides two BigInt numbers. */
		static void divide(	const BigInt &dividend, const BigInt &divisor,
							BigInt &quotient, BigInt &remainder);
		/* Returns the value of the specified unsigned char[] as long int. */
		static unsigned long int toInt(unsigned char *digits, int n);
		/* Saves the sum of two unsigned char* shorter and longer into result. 
		 * It must be nShorter <= nLonger. If doFill == true, it fills the 
		 * remaining free places with zeroes (used in KaratsubaMultiply()). 
		 * Returns true if there was an overflow at the end (meaning that
		 * the result.digitCount was longer.digitCount + 1. */
		static bool add(unsigned char *shorter, unsigned long int nShorter, 
					unsigned char *longer, unsigned long int nLonger, 
					unsigned char *result, int nResult, 
					bool doFill = true);
		/* Shifts the digits n places left. */
		BigInt &shiftLeft(unsigned long int n);
		/* Shifts the digits n places right. */
		BigInt &shiftRight(unsigned long int n);
		/* Expands the digits* to n. */
		void expandTo(unsigned long int n);
	public:
		BigInt();
		BigInt(const char *charNum);
		BigInt(unsigned long int intNum);
		BigInt(const std::string &str);
		BigInt(const BigInt &number);
		BigInt &operator =(const BigInt &rightNumber);
		~BigInt();
		operator std::string() const;
		friend std::ostream &operator <<(	std::ostream &cout, 
											const BigInt &number);
		friend std::istream &operator >>(	std::istream &cin, 
											BigInt &number);
		friend bool operator <(const BigInt &a, const BigInt &b);
		friend bool operator <=(const BigInt &a, const BigInt &b);
		friend bool operator >(const BigInt &a, const BigInt &b);
		friend bool operator >=(const BigInt &a, const BigInt &b);
		friend bool operator ==(const BigInt &a, const BigInt &b);
		friend bool operator !=(const BigInt &a, const BigInt &b);
		friend BigInt operator + (const BigInt &a, const BigInt &b);
		BigInt &operator+();
		BigInt &operator++();
		BigInt operator++(int);
		BigInt &operator+=(const BigInt &number);
		BigInt operator-() const;
		friend BigInt operator-(const BigInt &a, const BigInt &b);
		BigInt &operator--();
		BigInt operator--(int);
		BigInt &operator-=(const BigInt &number);
		friend BigInt operator*(const BigInt &a, const BigInt &b);
		BigInt &operator*=(const BigInt &number);
		friend BigInt operator/(const BigInt &a, const BigInt &b);
		BigInt &operator/=(const BigInt &number);
		friend BigInt operator%(const BigInt &a, const BigInt &b);
		BigInt &operator%=(const BigInt &number);
		/* Returns *this to the power of n 
		 * using the fast Square and Multiply algorithm. */
		BigInt GetPower(unsigned long int n) const;
		/* *this = *this to the power of n. */
		void SetPower(unsigned long int n);
		/* Returns *this to the power of n 
		 * using the fast Square and Multiply algorithm. */
		BigInt GetPower(BigInt n) const;
		/* *this = *this to the power of n. */
		void SetPower(BigInt n);
		/* Returns (*this to the power of b) mod n. */
		BigInt GetPowerMod(const BigInt &b, const BigInt &n) const;
		/* *this = (*this to the power of b) mod n. */
		void SetPowerMod(const BigInt &b, const BigInt &n);
		/* Returns the 'index'th digit (zero-based, right-to-left). */
		unsigned char GetDigit(unsigned long int index) const;
		/* Sets the value of 'index'th digit 
		 * (zero-based, right-to-left) to 'value'. */
		void SetDigit(unsigned long int index, unsigned char value);
		/* Returns the number of digits. */
		unsigned long int Length() const;
		/* Returns true if *this is positive, otherwise false. */
		bool IsPositive() const;
		/* Returns true if *this is odd, otherwise false. */
		bool IsOdd() const;
		/* Returns the value of BigInt as std::string. */
		std::string ToString(bool forceSign = false) const;
		/* Returns a value indicating whether *this equals 0. */
		bool EqualsZero() const;
		/* Returns the absolute value. */
		BigInt Abs() const;
};

inline BigInt::~BigInt()
{
	delete[] digits;
}

inline BigInt &BigInt::operator+()
{
	return *this;
}

/* Returns the number of digits. */
inline unsigned long int BigInt::Length() const
{
	return digitCount;
}

/* Returns true if *this is positive, otherwise false. */
inline bool BigInt::IsPositive() const
{
	return positive;
}

/* Returns true if *this is odd, otherwise false. */
inline bool BigInt::IsOdd() const
{
	return digits[0] & 1;
}

/* Returns a value indicating whether *this equals 0. */
inline bool BigInt::EqualsZero() const
{
	return digitCount == 1 && digits[0] == 0;
}
		
// A BigInt number with the value of 0. 
static const BigInt BigIntZero;
// A BigInt number with the value of 1. 
static const BigInt BigIntOne(1L);


#endif /*BIGINT_H_*/
