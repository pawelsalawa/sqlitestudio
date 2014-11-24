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
 * 				Key.cpp
 * 
 * Author: Nedim Srndic
 * Release date: 5th of September 2008
 * 
 * This file contains the implementation for the Key class. 
 * 
 * ****************************************************************************
 */

#include "Key.h"

std::ostream &operator<<(std::ostream &, const Key &key)
{
	return std::cout 
	<< "Modulus: " << key.GetModulus() << std::endl 
	<< "Exponent: " << key.GetExponent();
}
