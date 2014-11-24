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
