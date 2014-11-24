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
 * 				PrimeGenerator.h
 * 
 * A class used to generate large prime or random numbers. 
 * 
 * Author: Nedim Srndic
 * Release date: 14th of March 2008
 * 
 * ****************************************************************************
 */

#ifndef PRIMEGENERATOR_H_
#define PRIMEGENERATOR_H_

#include "BigInt.h"

class PrimeGenerator
{
	private:
		/* Generates a random "number" such as 1 <= "number" < "top".
		 * Returns it by reference in the "number" parameter. */
		static void makeRandom(	BigInt &number, 
								const BigInt &top);
		/* Creates an odd BigInt with the specified number of digits. 
		* Returns it by reference in the "number" parameter. */
		static void makePrimeCandidate(	BigInt &number, 
										unsigned long int digitCount);
		/* Tests the primality of the given _odd_ number using the 
		 * Miller-Rabin probabilistic primality test. Returns true if 
		 * the tested argument "number" is a probable prime with a 
		 * probability of at least 1 - 4^(-k), otherwise false.  */
		static bool isProbablePrime(const BigInt &number, 
									unsigned long int k);
		/* Returns true if "candidate" is a witness for the compositeness
		 * of "number", false if "candidate" is a strong liar. "exponent" 
		 * and "squareCount" are used for computation */
		static bool isWitness(	BigInt candidate, 
								const BigInt &number, 
								const BigInt &exponent, 
								unsigned long int squareCount, 
								const BigInt &numberMinusOne);
	public:
		/* Generates a random number with digitCount digits.
		 * Returns it by reference in the "number" parameter. */
		static void MakeRandom(	BigInt &number, 
								unsigned long int digitCount);
		/* Returns a probable prime number "digitCount" digits long, 
		 * with a probability of at least 1 - 4^(-k) that it is prime. */
		static BigInt Generate(	unsigned long int digitCount, 
								unsigned long int k = 3);
};

#endif /*PRIMEGENERATOR_H_*/
