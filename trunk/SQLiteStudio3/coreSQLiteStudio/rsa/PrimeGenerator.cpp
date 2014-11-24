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
 * 				PrimeGenerator.cpp
 * 
 * Author: Nedim Srndic
 * Release date: 14th of March 2008
 * 
 * This file contains the implementation for the PrimeGenerator class.
 * 
 * There is a static constant defined in this file: 
 * 
 * - RandMax		: RAND_MAX (defined in cstdlib) of type BigInt
 * 		Mainly used for speedup in the Generate member function. 
 * 		Represents the largest random unsigned long integer that a particular 
 * 		platform can generate. This is platform-specific. 
 * 	 
 * ****************************************************************************
 */

#include "PrimeGenerator.h"
#include <string>
#include <cstdlib> // rand()

/* Generates a random number with digitCount digits.
 * Returns it by reference in the "number" parameter. */
void PrimeGenerator::MakeRandom(BigInt &number, unsigned long int digitCount)
{
	//the new number will be created using a string object (newNum), 
	//and later converted into a BigInt
	std::string newNum;
	newNum.resize(digitCount);
	unsigned long int tempDigitCount(0);

	//generate random digits
	while (tempDigitCount < digitCount)
	{
		unsigned long int newRand(std::rand());

		//10 is chosen to skip the first digit, because it might be 
		//statistically <= n, where n is the first digit of RAND_MAX
		while (newRand >= 10)
		{
			newNum[tempDigitCount++] = (newRand % 10) + '0';
			newRand /= 10;
			if (tempDigitCount == digitCount)
				break;
		}
	}

	//make sure the leading digit is not zero
	if (newNum[0] == '0')
		newNum[0] = (std::rand() % 9) + 1 + '0';
	number = newNum;
}

/* Generates a random number such as 1 <= number < 'top'.
 * Returns it by reference in the 'number' parameter. */
void PrimeGenerator::makeRandom(BigInt &number, const BigInt &top)
{
	//randomly select the number of digits for the random number
	unsigned long int newDigitCount = (rand() % top.Length()) + 1;
	MakeRandom(number, newDigitCount);
	//make sure number < top
	while (number >= top)
		MakeRandom(number, newDigitCount);
}

/* Creates an odd BigInt with the specified number of digits. 
 * Returns it by reference in the "number" parameter. */
void PrimeGenerator::makePrimeCandidate(BigInt &number,
										unsigned long int digitCount)
{
	PrimeGenerator::MakeRandom(number, digitCount);
	//make the number odd
	if (!number.IsOdd())
		number.SetDigit(0, number.GetDigit(0) + 1);
	//make sure the leading digit is not a zero
	if (number.GetDigit(number.Length() - 1) == 0)
		number.SetDigit(number.Length() - 1, (std::rand() % 9) + 1);
}

/* Tests the primality of the given _odd_ number using the 
 * Miller-Rabin probabilistic primality test. Returns true if 
 * the tested argument "number" is a probable prime with a 
 * probability of at least 1 - 4^(-k), otherwise false. */
bool PrimeGenerator::isProbablePrime(	const BigInt &number, 
										unsigned long int k)
{
	//first we need to calculate such a and b, that
	//number - 1 = 2^a * b, a and b are integers, b is odd
	BigInt numberMinusOne(number - BigIntOne);
	unsigned long int a(0);
	BigInt temp(numberMinusOne);
	BigInt b, quotient;
	static const BigInt two(BigIntOne + BigIntOne);

	while (b.EqualsZero())
	{
		//temp = quotient * 2 + remainder
		
		//PrimeGenerator used to be a friend of BigInt, so the following 
		//statement produced the result in one call to BigInt::divide()
//		BigInt::divide(temp, two, quotient, b);
		//That doesn't work any more, so we have to use two calls
		quotient = temp / two;
		b = temp % two;
		temp = quotient;
		a++;
	}
	b = temp * two + b;
	a--;

	//test with k different possible witnesses to ensure that the probability
	//that "number" is prime is at least 1 - 4^(-k)
	for (unsigned long int i = 0; i < k; i++)
	{
		PrimeGenerator::makeRandom(temp, number);
		
		if (isWitness(temp, number, b, a, numberMinusOne))
			return false; //definitely a composite number
	}
	return true; //a probable prime
}

/* Returns true if "candidate" is a witness for the compositeness
 * of "number", false if "candidate" is a strong liar. "exponent" 
 * and "squareCount" are used for computation */
bool PrimeGenerator::isWitness(	BigInt candidate, 
								const BigInt &number, 
								const BigInt &exponent, 
								unsigned long int squareCount, 
								const BigInt &numberMinusOne)
{
	//calculate candidate = (candidate to the power of exponent) mod number
	candidate.SetPowerMod(exponent, number);
	//temporary variable, used to call the divide function
	BigInt quotient;

	for (unsigned long int i = 0; i < squareCount; i++)
	{
		bool maybeWitness(false);
		if (candidate != BigIntOne && candidate != numberMinusOne)
			maybeWitness = true;

		//PrimeGenerator used to be a friend of BigInt, so the following 
		//statement produced the result in one call to BigInt::divide()
//		BigInt::divide(candidate * candidate, number, quotient, candidate);
		//That doesn't work any more, so we have to use two calls
		candidate = candidate * candidate;
		quotient = (candidate) / number;
		candidate = (candidate) % number;
		if (maybeWitness && candidate == BigIntOne)
			return true; //definitely a composite number
	}

	if (candidate != BigIntOne)
		return true; //definitely a composite number

	return false; //probable prime
}

/* Returns a probable prime number "digitCount" digits long, 
 * with a probability of at least 1 - 4^(-k) that it is prime. */
BigInt PrimeGenerator::Generate(unsigned long int digitCount, 
								unsigned long int k)
{
	if (digitCount < 3)
		throw "Error PRIMEGENERATOR00: Primes less than 3 digits long "
				"not supported.";
	
	BigInt primeCandidate;
	PrimeGenerator::makePrimeCandidate(primeCandidate, digitCount);
	while (!isProbablePrime(primeCandidate, k))
	{
		//select the next odd number and try again
		primeCandidate = primeCandidate + 2;
		if (primeCandidate.Length() != digitCount)
		PrimeGenerator::makePrimeCandidate(primeCandidate, digitCount);
	}
	return primeCandidate;
}
