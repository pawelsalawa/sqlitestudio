/* ****************************************************************************
 *
 * Copyright 2013 Nedim Srndic
 * 
 * This file is part of rsa - the RSA implementation in C++.
 *
 * 				Key.h
 * 
 * Author: Nedim Srndic
 * Release date: 16th of June 2008
 * 
 * A class representing a public or private RSA key. 
 * 
 * A public or private RSA key consists of a modulus and an exponent. In this 
 * implementation an object of type BigInt is used to store those values. 
 * 
 * ****************************************************************************
 */

#ifndef KEY_H_
#define KEY_H_

#include "BigInt.h"
#include "coreSQLiteStudio_global.h"
#include <iostream>

class API_EXPORT Key
{
	private:
		BigInt modulus;
		BigInt exponent;
	public:
		Key(const BigInt &modulus, const BigInt &exponent) :
			modulus(modulus), exponent(exponent)
		{}
		const BigInt &GetModulus() const
		{
			return modulus;
		}
		const BigInt &GetExponent() const
		{
			return exponent;
		}
        friend std::ostream &operator<<(std::ostream &, const Key &key);
};

#endif /*KEY_H_*/
