/* ****************************************************************************
 *
 * Copyright 2013 Nedim Srndic
 * 
 * This file is part of rsa - the RSA implementation in C++.
 *
 * 				KeyPair.cpp
 * 
 * Author: Nedim Srndic
 * Release date: 22th of July 2008
 * 
 * This file contains the implementation for the KeyPair class. 
 * 
 * ****************************************************************************
 */

#include "KeyPair.h"

std::ostream &operator <<(std::ostream &, const KeyPair &k)
{
	return std::cout 
	<< "Private key:" << std::endl << k.GetPrivateKey() << std::endl
	<< "Public key:" << std::endl << k.GetPublicKey();
}
