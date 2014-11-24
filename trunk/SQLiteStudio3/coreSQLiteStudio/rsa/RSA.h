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
 * 				RSA.h
 * 
 * Author: Nedim Srndic
 * Release date: 16th of June 2008
 * 
 * An implementation of the RSA public-key cryptography algorithm. 
 * 
 * RSA supports: 
 * 
 * 	- Message encryption (string and file) (Encrypt())
 * 	- Message decryption (string and file) (Decrypt())
 * 	- Public/private keypair generation (GenerateKeyPair())
 * 
 * NOTE: All methods are static. Instantiation, copying and assignment of 
 * 	objects of type RSA is forbidden. 
 * 
 * NOTE: it is highly recommended to call 
 * 		std::srand(time(NULL));
 * 	once when the program starts and before any use of methods provided by the 
 * 	RSA class. Calling the srand() function randomizes the standard C++ 
 * 	pseudorandom number generator, so that it provides different series of 
 * 	pseudorandom numbers every time the program is run. This greatly improves 
 * 	security. 
 * 
 * ****************************************************************************
 */

#ifndef RSA_H_
#define RSA_H_

#include <string>
#include <fstream>
#include "KeyPair.h"
#include "Key.h"
#include "BigInt.h"

class RSA
{
	private:
		/* Instantiation of objects of type RSA is forbidden. */
		RSA()
		{}
		/* Copying of objects of type RSA is forbidden. */
		RSA(const RSA &rsa);
		/* Assignment of objects of type RSA is forbidden. */
		RSA &operator=(const RSA &rsa);
		/* Returns the greatest common divisor of the two arguments 
		 * "a" and "b", using the Euclidean algorithm. */
		static BigInt GCD(const BigInt &a, const BigInt &b);
		/* Solves the equation 
		 * 			d = ax + by 
		 * given a and b, and returns d, x and y by reference. 
		 * It uses the Extended Euclidean Algorithm */
		static void extendedEuclideanAlgorithm(	const BigInt &a, 
												const BigInt &b, 
												BigInt &d, 
												BigInt &x, 
												BigInt &y);
		/* Solves the equation 
		 * 			ax is congruent to b (mod n), 
		 * given a, b and n finds x. */
		static BigInt solveModularLinearEquation(	const BigInt &a, 
													const BigInt &b, 
													const BigInt &n);
		/* Throws an exception if "key" is too short to be used. */
		static void checkKeyLength(const Key &key);
		/* Transforms a std::string message into a BigInt message. */
		static BigInt encode(const std::string &message);
		/* Transforms a BigInt cyphertext into a std::string cyphertext. */
		static std::string decode(const BigInt &message);
		/* Encrypts a "chunk" (a small part of a message) using "key" */
		static std::string encryptChunk(const std::string &chunk, 
										const Key &key);
		/* Decrypts a "chunk" (a small part of a message) using "key" */
		static std::string decryptChunk(const BigInt &chunk, 
										const Key &key);
		/* Encrypts a string "message" using "key". */
		static std::string encryptString(	const std::string &message, 
											const Key &key);
		/* Decrypts a string "message" using "key". */
		static std::string decryptString(	const std::string &cypherText, 
											const Key &key);
		/* Tests the file for 'eof', 'bad ' errors and throws an exception. */
		static void fileError(bool eof, bool bad);
	public:
		/* Returns the string "message" RSA-encrypted using the key "key". */
		static std::string Encrypt(	const std::string &message, 
									const Key &key);
		/* Encrypts the file "sourceFile" using the key "key" and saves 
		 * the result into the file "destFile". */
		static void Encrypt(const char *sourceFile, 
							const char *destFile, 
							const Key &key);
		/* Decrypts the file "sourceFile" using the key "key" and saves 
		 * the result into the file "destFile". */
		static void Decrypt(const char *sourceFile, 
							const char *destFile, 
							const Key &key);
		/* Returns the string "cypherText" RSA-decrypted 
		 * using the key "key". */
		static std::string Decrypt(	const std::string &cypherText, 
									const Key &key);
		/* Generates a public/private keypair. The keys are retured in a 
		 * KeyPair. The generated keys are 'digitCount' or 
		 * 'digitCount' + 1 digits long. */
		static KeyPair GenerateKeyPair(	unsigned long int digitCount, 
										unsigned long int k = 3);
};

#endif /*RSA_H_*/
