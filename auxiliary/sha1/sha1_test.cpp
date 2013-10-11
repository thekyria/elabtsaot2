/*
 *  shatest.cpp
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: shatest.cpp 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This file will exercise the SHA1 class and perform the three
 *      tests documented in FIPS PUB 180-1.
 *
 *  Portability Issues:
 *      None.
 *
 */

#include <iostream>
#include "sha1.h"

using namespace std;

#define TESTA   "abc"
#define TESTB   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"

void DisplayMessageDigest(unsigned *message_digest);

int main() {

  SHA1        sha;
  unsigned    message_digest[5];

  // ----------------- TEST A -----------------
  cout << endl << "Test A: 'abc'" << endl;

  sha.Reset();
  sha << TESTA;

  if (!sha.Result(message_digest)) {
    cerr << "ERROR-- could not compute message digest" << endl;
  } else  {
    DisplayMessageDigest(message_digest);
    cout << "Should match:" << endl;
    cout << '\t' << "A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D" << endl;
  }

  // ----------------- TEST A -----------------
  cout << endl << "Test B: " << TESTB << endl;

  sha.Reset();
  sha << TESTB;

  if (!sha.Result(message_digest)) {
    cerr << "ERROR-- could not compute message digest" << endl;
  } else {
    DisplayMessageDigest(message_digest);
    cout << "Should match:" << endl;
    cout << '\t' << "84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1" << endl;
  }

  // ----------------- TEST C -----------------
  cout << endl << "Test C: One million 'a' characters" << endl;

  sha.Reset();
  for(int i = 1; i <= 1000000; i++) sha.Input('a');

  if (!sha.Result(message_digest)) {
    cerr << "ERROR-- could not compute message digest" << endl;
  } else {
    DisplayMessageDigest(message_digest);
    cout << "Should match:" << endl;
    cout << '\t' << "34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F" << endl;
  }

  return 0;
}

// Display Message Digest array
void DisplayMessageDigest(unsigned *message_digest) {

  ios::fmtflags   flags;

  cout << '\t';

  flags = cout.setf(ios::hex|ios::uppercase,ios::basefield);
  cout.setf(ios::uppercase);

  for(int i = 0; i < 5 ; i++)
  {
    cout << message_digest[i] << ' ';
  }

  cout << endl;

  cout.setf(flags);
}
