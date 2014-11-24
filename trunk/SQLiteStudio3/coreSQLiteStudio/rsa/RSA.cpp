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
 * 				RSA.cpp
 * 
 * Author: Nedim Srndic
 * Release date: 16th of June 2008
 * 
 * This file contains the implementation for the RSA class.
 * 
 * ****************************************************************************
 */

#include "RSA.h"
#include "Key.h"	//Key
#include "KeyPair.h"	//KeyPair
#include "PrimeGenerator.h"	//Generate()
#include <string>	//string
#include <fstream>	//ifstream, ofstream

using std::string;

/* Returns the greatest common divisor of the two arguments 
 * "a" and "b", using the Euclidean algorithm. */
BigInt RSA::GCD(const BigInt &a, const BigInt &b)
{
	if (b.EqualsZero())
		return a;
	else
		return RSA::GCD(b, a % b);
}

/* Solves the equation 
 * 			d = ax + by 
 * given a and b, and returns d, x and y by reference. 
 * It uses the Extended Euclidean Algorithm */
void RSA::extendedEuclideanAlgorithm(	const BigInt &a, const BigInt &b, 
										BigInt &d, BigInt &x, BigInt &y)
{
	if (b.EqualsZero())
	{
		d = a;
		x = BigIntOne;
		y = BigIntZero;
		return;
	}
	RSA::extendedEuclideanAlgorithm(b, a % b, d, x, y);
	BigInt temp(x);
	x = y;
	y = temp - a / b * y;
}

/* Solves the equation 
 * 			ax is congruent to b (mod n), 
 * given a, b and n finds x. */
BigInt RSA::solveModularLinearEquation(	const BigInt &a, 
										const BigInt &b, 
										const BigInt &n)
{
	BigInt p, q, r;
	RSA::extendedEuclideanAlgorithm(a, n, p, q, r);
	if ((b % p).EqualsZero())	// This has to evaluate to 'true'.
		return (q * (b / p)) % n;
	else
		throw "Error RSA00: Error in key generation."; // Detect mistakes.
}

/* Throws an exception if "key" is too short to be used. */
void RSA::checkKeyLength(const Key &key)
{
	// Minimum required key length is around 24 bits. (In-house requirement)
	if (key.GetModulus().Length() < 8)
			throw "Error RSA01: Keys must be at least 8 digits long.";
}

/* Transforms a std::string message into a BigInt message. 
 * Every ASCII character of the original message is replaced by it's 
 * ASCII value and appended to the end of the newly created BigInt object
 * 'decoded' as a three-digit number, from left to right. */
BigInt RSA::encode(const string &message)
{
	// The new number will be created using a string object (encoded), 
	// and converted into a BigInt on return.
	string encoded;
	encoded.resize(message.length() * 3 + 1);
	unsigned long int index = message.length() * 3;
	for (unsigned long int i(0); i < message.length(); i++)
	{
		// Encode the characters using their ASCII values' digits as
		// BigInt digits. 
		unsigned char ASCII = message[i];
		encoded[index - 2] = (ASCII % 10) + '0';
		ASCII /= 10;
		encoded[index - 1] = (ASCII % 10) + '0';
		encoded[index] = (ASCII / 10) + '0';
		index -= 3;
	}
	// We add an special symbol '1' to the beginning of the string 'encoded' 
	// to make sure that the returned BigInt doesn't begin with a zero. We also
	// need to make sure we remove that '1' when decoding (see RSA::decode()). 
	encoded[0] = '1';
	return encoded;
}

/* Transforms a BigInt cyphertext into a std::string cyphertext. */
string RSA::decode(const BigInt &message)
{
	string decoded;
	// The special symbol '1' we added to the beginning of the encoded message 
	// will now be positioned at message[message.Length() - 1], and 
	// message.Length() -1 must be divisible by 3 without remainder. Thus we 
	// can ignore the special symbol by only using digits in the range 
	// from message[0] to message[message.Length() - 2]. 
	for (unsigned long int i(0); i < message.Length() / 3; i++)
	{
		// Decode the characters using the ASCII values in the BigInt digits. 
		char ASCII = 100 * char(message.GetDigit(i * 3));
		ASCII += 10 * char(message.GetDigit(i * 3 + 1));
		decoded.push_back(ASCII + char(message.GetDigit(i * 3 + 2)));
	}
	return decoded;
}

/* Encrypts a "chunk" (a small part of a message) using "key" */
string RSA::encryptChunk(const string &chunk, const Key &key)
{
	// First encode the chunk, to make sure it is represented as an integer. 
	BigInt a = RSA::encode(chunk);
	// The RSA encryption algorithm is a congruence equation. 
	a.SetPowerMod(key.GetExponent(), key.GetModulus());
	return a.ToString();
}

/* Decrypts a "chunk" (a small part of a message) using "key" */
string RSA::decryptChunk(const BigInt &chunk, const Key &key)
{
	BigInt a = chunk;
	// The RSA decryption algorithm is a congruence equation. 
	a.SetPowerMod(key.GetExponent(), key.GetModulus());
	// Decode the message to a readable form. 
	return RSA::decode(a);
}

/* Encrypts a string "message" using "key". */
std::string RSA::encryptString(const std::string &message, const Key &key)
{
	//partition the message into biggest possible encryptable chunks
	const unsigned long int chunkSize(((key.GetModulus().Length() - 2) / 3));
	const unsigned long int chunkCount = message.length() / chunkSize;
	
	string cypherText;
	for (unsigned long int i(0); i < chunkCount; i++)
	{
		// Get the next chunk.
		string chunk(message.substr(i * chunkSize, chunkSize));
		chunk = RSA::encryptChunk(chunk, key);
		// Put a ' ' between the chunks so that we can separate them later. 
		cypherText.append(chunk.append(" "));
	}
	// If the last chunk has the same size as the others, we are finished. 
	if (chunkSize * chunkCount == message.length())
		return cypherText;
	
	// Handle the last chunk. It is smaller than the others. 
	const unsigned long int lastChunkSize = message.length() % chunkSize;
	string lastChunk(message.substr(chunkCount * chunkSize, lastChunkSize));
	lastChunk = RSA::encryptChunk(lastChunk, key);
	return cypherText.append(lastChunk.append(" "));
}

/* Decrypts a string "message" using "key". */
std::string RSA::decryptString(const std::string &cypherText, const Key &key)
{
	// Partition the cypherText into chunks. They are seperated by ' '. 
	string message;
	long int i(0), j(0);
	while ((j = cypherText.find(' ', i)) != -1)
	{
		// Get the chunk. 
		BigInt chunk(cypherText.substr(i, j - i));
		if (chunk >= key.GetModulus())
			throw "Error RSA02: Chunk too large.";
		
		// Decrypt the chunk and store the message. 
		string text = RSA::decryptChunk(chunk, key);
		message.append(text);
		i = j + 1;
	}
	return message;
}

/* Tests the file for 'eof', 'bad ' errors and throws an exception. */
void RSA::fileError(bool eof, bool bad)
{
	if (eof)
		throw "Error RSA03: Unexpected end of file.";
	else if (bad)
		throw "Error RSA04: Bad file?";
	else
		throw "Error RSA05: File contains unexpected data.";
}

/* Returns the string "message" RSA-encrypted using the key "key". */
string RSA::Encrypt(const string &message, const Key &key)
{
	RSA::checkKeyLength(key);
	
	return RSA::encryptString(message, key);
}

/* Encrypts the file "sourceFile" using the key "key" and saves 
 * the result into the file "destFile". */
void RSA::Encrypt(	const char *sourceFile, const char *destFile, 
					const Key &key)
{
	RSA::checkKeyLength(key);
	
	//open the input and output files
	std::ifstream source(sourceFile, std::ios::in | std::ios::binary);
	if (!source)
		throw "Error RSA06: Opening file \"sourceFile\" failed.";
	std::ofstream dest(destFile, std::ios::out | std::ios::binary);
	if (!dest)
		throw "Error RSA07: Creating file \"destFile\" failed.";
	
	//find the source file length
	source.seekg(0, std::ios::end);
	const unsigned long int fileSize = source.tellg();
	source.seekg(0, std::ios::beg);
	
	//create an input buffer
	const unsigned long int bufferSize = 4096;
	char buffer[bufferSize];
	
	//encrypt file chunks
	const unsigned long int chunkCount = fileSize / bufferSize;
	for (unsigned long int i(0); i <= chunkCount; i++)
	{
		unsigned long int readLength; 
		//read the chunk
		if (i == chunkCount)	//if it's the last one
			readLength = fileSize % bufferSize;
		else
			readLength = sizeof buffer;
		source.read(buffer, readLength);
		if (!source)
			RSA::fileError(source.eof(), source.bad());
		
		//encrypt the chunk
		std::string chunk(buffer, readLength);
		chunk = RSA::encryptString(chunk, key);
		//write the chunk
		dest.write(chunk.c_str(), chunk.length());
		if (!dest)
			RSA::fileError(dest.eof(), dest.bad());
	}
	
	source.close();
	dest.close();
}

/* Returns the string "cypherText" RSA-decrypted using the key "key". */
string RSA::Decrypt(const string &cypherText, const Key &key)
{
	RSA::checkKeyLength(key);
	
	return RSA::decryptString(cypherText, key);
}

/* Decrypts the file "sourceFile" using the key "key" and saves 
 * the result into the file "destFile". */
void RSA::Decrypt(	const char *sourceFile, const char *destFile, 
					const Key &key)
{
	RSA::checkKeyLength(key);
		
	//open the input and output files
	std::ifstream source(sourceFile, std::ios::in | std::ios::binary);
	if (!source)
		throw "Error RSA08: Opening file \"sourceFile\" failed.";
	std::ofstream dest(destFile, std::ios::out | std::ios::binary);
	if (!dest)
		throw "Error RSA09: Creating file \"destFile\" failed.";
	
	//find the source file length
	source.seekg(0, std::ios::end);
	const unsigned long int fileSize = source.tellg();
	source.seekg(0, std::ios::beg);
	
	//create an input buffer
	const unsigned long int bufferSize = 8192;
	char buffer[bufferSize];
	unsigned long int readCount = 0;
	
	while (readCount < fileSize)
	{
		unsigned long int readLength; 
		//read new data
		if (fileSize - readCount >= bufferSize)	//if it's not the last one
			readLength = sizeof buffer;
		else
			readLength = fileSize - readCount;
		source.read(buffer, readLength);
		if (!source)
			RSA::fileError(source.eof(), source.bad());
		
		//find the next chunk
		std::string chunk(buffer, readLength);
		chunk = chunk.substr(0, chunk.find_last_of(' ', chunk.length()) + 1);
		readCount += chunk.length();
		source.seekg(readCount, std::ios::beg);
		//decrypt the chunk
		chunk = RSA::decryptString(chunk, key);
		//write the chunk
		dest.write(chunk.c_str(), chunk.length());
		if (!dest)
			RSA::fileError(dest.eof(), dest.bad());
	}
	
	source.close();
	dest.close();
}

/* Generates a public/private keypair. The keys are retured in a 
 * KeyPair. The generated keys are 'digitCount' or 
 * 'digitCount' + 1 digits long. */
KeyPair RSA::GenerateKeyPair(	unsigned long int digitCount, 
								unsigned long int k)
{
	if (digitCount < 8)
		throw "Error RSA10: Keys must be at least 8 digits long.";
	
	//generate two random numbers p and q
	BigInt p(PrimeGenerator::Generate(digitCount / 2 + 2, k));
	BigInt q(PrimeGenerator::Generate(digitCount / 2 - 1, k));
	
	//make sure they are different
	while (p == q)
	{
		p = PrimeGenerator::Generate(digitCount / 2 + 1, k);
	}
	
	//calculate the modulus of both the public and private keys, n
	BigInt n(p * q);
	
	//calculate the totient phi
	BigInt phi((p - BigIntOne) * (q - BigIntOne));
	
	//select a small odd integer e that is coprime with phi and e < phi
	//usually 65537 is used, and we will use it too if it fits
	//it is recommended that this be the least possible value for e
	BigInt e("65537");
	
	//make sure the requirements are met
	while (RSA::GCD(phi, e) != BigIntOne || e < "65537" || !e.IsOdd())
	{
		PrimeGenerator::MakeRandom(e, 5);
	}
	
	//now we have enough information to create the public key
	//e is the public key exponent, n is the modulus
	Key publicKey(n, e);
	
	//calculate d, d * e = 1 (mod phi)
	BigInt d(RSA::solveModularLinearEquation(e, BigIntOne, phi));
	
	//we need a positive private exponent
	if (!d.IsPositive())
		return RSA::GenerateKeyPair(digitCount, k);
	
	//we can create the private key
	//d is the private key exponent, n is the modulus
	Key privateKey(n, d);
	
	//finally, the keypair is created and returned
	KeyPair newKeyPair(privateKey, publicKey);
	return newKeyPair;
}
