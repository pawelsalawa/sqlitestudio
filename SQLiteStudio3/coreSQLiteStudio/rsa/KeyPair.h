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
 * 				KeyPair.h
 * 
 * Author: Nedim Srndic
 * Release date: 17th of June 2008
 * 
 * A class representing a public/private RSA keypair. 
 * 
 * A keypair consists of a public key and a matching private key. 
 * 
 * ****************************************************************************
 */

#ifndef KEYPAIR_H_
#define KEYPAIR_H_

#include "Key.h"
#include <iostream>

class KeyPair
{
	private:
		const Key privateKey;
		const Key publicKey;
	public:
		KeyPair(Key privateKey, Key publicKey): 
			privateKey(privateKey), publicKey(publicKey)
		{}
		const Key &GetPrivateKey() const
		{
			return privateKey;
		}
		const Key &GetPublicKey() const
		{
			return publicKey;
		}
        friend std::ostream &operator <<(std::ostream &, const KeyPair &k);
};

#endif /*KEYPAIR_H_*/
