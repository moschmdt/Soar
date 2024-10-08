/**
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * AND BELOW FOR LICENSE AND COPYRIGHT INFORMATION.
 */

/**
 *
 *  file:  soar_rand.h
 *
 * =======================================================================
 *  The ANSI rand and srand functions (at least the implementations
 *  provided with Visual Studio) cause problems for us, mostly because
 *  of threading issues (see bug 595).
 *  This is a replacement random number generator that doesn't suffer
 *  from those issues. It also automatically seeds itself, so we don't
 *  have to do that.
 *  Usage: SoarRand() will return a double in [0,1].
 *         SoarRand.RandInt(n) will return an integer in [0,n].
 *         See implementation for complete list of available functions.
 * =======================================================================
 */

#ifndef SOAR_RAND_H
#define SOAR_RAND_H

// Not thread safe (unless auto-initialization is avoided and each thread has
// its own MTRand object)
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include <iostream>

#include "Export.h"
#include "kernel.h"

// Mersenne Twister random number generator -- a C++ class MTRand
// Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
// Richard J. Wagner  v1.0  15 May 2003  rjwagner@writeme.com

// The Mersenne Twister is an algorithm for generating random numbers.  It
// was designed with consideration of the flaws in various other generators.
// The period, 2^19937-1, and the order of equidistribution, 623 dimensions,
// are far greater.  The generator is also fast; it avoids multiplication and
// division, and it benefits from caches and pipelines.  For more information
// see the inventors' web page at http://www.math.keio.ac.jp/~matumoto/emt.html

// Reference
// M. Matsumoto and T. Nishimura, "Mersenne Twister: A 623-Dimensionally
// Equidistributed Uniform Pseudo-Random Number Generator", ACM Transactions on
// Modeling and Computer Simulation, Vol. 8, No. 1, January 1998, pp 3-30.

// Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// Copyright (C) 2000 - 2003, Richard J. Wagner
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//   3. The names of its contributors may not be used to endorse or promote
//      products derived from this software without specific prior written
//      permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// The original code included the following notice:
//
//     When you use this, send an email to: matumoto@math.keio.ac.jp
//     with an appropriate reference to your work.
//
// It would be nice to CC: rjwagner@writeme.com and Cokus@math.washington.edu
// when you write.

class MTRand {
  // Data
 public:
  enum { N = 624 };       // length of state vector
  enum { SAVE = N + 1 };  // length of array for save()

 protected:
  enum { M = 397 };  // period parameter

  uint32_t state[N];  // internal state
  uint32_t* pNext;    // next value to get from state
  int left;           // number of values left before reload needed

  // Methods
 public:
  MTRand(const uint32_t& oneSeed);  // initialize with a simple uint32
  MTRand(uint32_t* const bigSeed,
         uint32_t const seedLength = N);  // or an array
  MTRand();  // auto-initialize with /dev/urandom or time() and clock()

  // Do NOT use for CRYPTOGRAPHY without securely hashing several returned
  // values together, otherwise the generator state can be learned after
  // reading 624 consecutive values.

  // Access to 32-bit random numbers
  double rand();                        // real number in [0,1]
  double rand(const double& n);         // real number in [0,n]
  double randExc();                     // real number in [0,1)
  double randExc(const double& n);      // real number in [0,n)
  double randDblExc();                  // real number in (0,1)
  double randDblExc(const double& n);   // real number in (0,n)
  uint32_t randInt();                   // integer in [0,2^32-1]
  uint32_t randInt(const uint32_t& n);  // integer in [0,n] for n < 2^32
  double operator()() {
    return rand();  // same as rand()
  }

  // Access to 53-bit random numbers (capacity of IEEE double precision)
  double rand53();  // real number in [0,1)

  // Access to nonuniform random number distributions
  double randNorm(const double& mean = 0.0, const double& stddeviation = 0.0);

  // Re-seeding functions with same behavior as initializers
  void seed(const uint32_t oneSeed);
  void seed(uint32_t* const bigSeed, const uint32_t seedLength = N);
  void seed();

  // Saving and loading generator state
  void save(uint32_t* saveArray) const;  // to array of size SAVE
  void load(uint32_t* const loadArray);  // from such array
  friend std::ostream& operator<<(std::ostream& os, const MTRand& mtrand);
  friend std::istream& operator>>(std::istream& is, MTRand& mtrand);

 protected:
  void initialize(const uint32_t oneSeed);
  void reload();
  uint32_t hiBit(const uint32_t& u) const { return u & 0x80000000U; }
  uint32_t loBit(const uint32_t& u) const { return u & 0x00000001U; }
  uint32_t loBits(const uint32_t& u) const { return u & 0x7fffffffU; }
  uint32_t mixBits(const uint32_t& u, const uint32_t& v) const {
    return hiBit(u) | loBits(v);
  }
#ifdef _MSC_VER
#pragma warning(push)            // save current warning settings
#pragma warning(disable : 4146)  // warning C4146: unary minus operator applied
                                 // to unsigned type, result still unsigned
#endif
  uint32_t twist(const uint32_t& m, const uint32_t& s0,
                 const uint32_t& s1) const {
    return m ^ (mixBits(s0, s1) >> 1) ^
           (-loBit(s1) &
            0x9908b0dfU);  // RPM 1/06 this line causes Visual Studio warning
                           // C4146, but is actually safe
  }
#ifdef _MSC_VER
#pragma warning(pop)  // return warning settings to what they were
#endif

  static uint32_t hash(time_t t, clock_t c);
};

inline MTRand::MTRand(const uint32_t& oneSeed) { seed(oneSeed); }

inline MTRand::MTRand(uint32_t* const bigSeed, const uint32_t seedLength) {
  seed(bigSeed, seedLength);
}

inline MTRand::MTRand() { seed(); }

inline double MTRand::rand() {
  return double(randInt()) * (1.0 / 4294967295.0);
}

inline double MTRand::rand(const double& n) { return rand() * n; }

inline double MTRand::randExc() {
  return double(randInt()) * (1.0 / 4294967296.0);
}

inline double MTRand::randExc(const double& n) { return randExc() * n; }

inline double MTRand::randDblExc() {
  return (double(randInt()) + 0.5) * (1.0 / 4294967296.0);
}

inline double MTRand::randDblExc(const double& n) { return randDblExc() * n; }

inline double MTRand::rand53() {
  uint32_t a = randInt() >> 5, b = randInt() >> 6;
  return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);  // by Isaku Wada
}

inline double MTRand::randNorm(const double& mean, const double& stddeviation) {
  // Return a real number from a normal (Gaussian) distribution with given
  // mean and variance by Box-Muller method
  double r = sqrt(-2.0 * log(1.0 - randDblExc())) * stddeviation;
  double phi = 2.0 * 3.14159265358979323846264338328 * randExc();
  return mean + r * cos(phi);
}

inline uint32_t MTRand::randInt() {
  // Pull a 32-bit integer from the generator state
  // Every other access function simply transforms the numbers extracted here

  if (left == 0) {
    reload();
  }
  --left;

  uint32_t s1;
  s1 = *pNext++;
  s1 ^= (s1 >> 11);
  s1 ^= (s1 << 7) & 0x9d2c5680U;
  s1 ^= (s1 << 15) & 0xefc60000U;
  return (s1 ^ (s1 >> 18));
}

inline uint32_t MTRand::randInt(const uint32_t& n) {
  // Find which bits are used in n
  // Optimized by Magnus Jonsson (magnus@smartelectronix.com)
  uint32_t used = n;
  used |= used >> 1;
  used |= used >> 2;
  used |= used >> 4;
  used |= used >> 8;
  used |= used >> 16;

  // Draw numbers until one is found in [0,n]
  uint32_t i;
  do {
    i = randInt() & used;  // toss unused bits to shorten search
  } while (i > n);
  return i;
}

inline void MTRand::seed(const uint32_t oneSeed) {
  // Seed the generator with a simple uint32
  initialize(oneSeed);
  reload();
}

inline void MTRand::seed(uint32_t* const bigSeed, const uint32_t seedLength) {
  // Seed the generator with an array of uint32's
  // There are 2^19937-1 possible initial states.  This function allows
  // all of those to be accessed by providing at least 19937 bits (with a
  // default seed length of N = 624 uint32's).  Any bits above the lower 32
  // in each element are discarded.
  // Just call seed() if you want to get array from /dev/urandom
  initialize(19650218U);
  int i = 1;
  uint32_t j = 0;
  int k = (N > seedLength ? N : seedLength);
  for (; k; --k) {
    state[i] = state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1664525U);
    state[i] += (bigSeed[j] & 0xffffffffU) + j;
    state[i] &= 0xffffffffU;
    ++i;
    ++j;
    if (i >= N) {
      state[0] = state[N - 1];
      i = 1;
    }
    if (j >= seedLength) {
      j = 0;
    }
  }
  for (k = N - 1; k; --k) {
    state[i] = state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1566083941U);
    state[i] -= i;
    state[i] &= 0xffffffffU;
    ++i;
    if (i >= N) {
      state[0] = state[N - 1];
      i = 1;
    }
  }
  state[0] = 0x80000000U;  // MSB is 1, assuring non-zero initial array
  reload();
}

inline void MTRand::seed() {
  // Seed the generator with an array from /dev/urandom if available
  // Otherwise use a hash of time() and clock() values

  // First try getting an array from /dev/urandom
  FILE* urandom = fopen("/dev/urandom", "rb");
  if (urandom) {
    uint32_t bigSeed[N];
    uint32_t* s = bigSeed;
    int i = N;
    bool success = true;
    while (success && i--)
    // success = fread( s++, sizeof(uint32_t), 1, urandom );
    {
      success =
          (fread(s++, sizeof(uint32_t), 1, urandom) ==
           0);  // RPM 1/06 modified to eliminate Visual Studio warning C4800
    }
    fclose(urandom);
    if (success) {
      seed(bigSeed, N);
      return;
    }
  }

  // Was not successful, so use time() and clock() instead
  seed(hash(time(NULL), clock()));
}

inline void MTRand::initialize(const uint32_t seed) {
  // Initialize generator state with seed
  // See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
  // In previous versions, most significant bits (MSBs) of the seed affect
  // only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
  uint32_t* s = state;
  uint32_t* r = state;
  int i = 1;
  *s++ = seed & 0xffffffffU;
  for (; i < N; ++i) {
    *s++ = (1812433253U * (*r ^ (*r >> 30)) + i) & 0xffffffffU;
    r++;
  }
}

inline void MTRand::reload() {
  // Generate N new values in state
  // Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
  uint32_t* p = state;
  int i;
  for (i = N - M; i--; ++p) {
    *p = twist(p[M], p[0], p[1]);
  }
  for (i = M; --i; ++p) {
    *p = twist(p[M - N], p[0], p[1]);
  }
  *p = twist(p[M - N], p[0], state[0]);

  left = N, pNext = state;
}

inline uint32_t MTRand::hash(time_t t, clock_t c) {
  // Get a uint32 from t and c
  // Better than uint32(x) in case x is floating point in [0,1]
  // Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

  static uint32_t differ = 0;  // guarantee time-based seeds will change

  uint32_t h1 = 0;
  unsigned char* p = (unsigned char*)&t;
  for (size_t i = 0; i < sizeof(t); ++i) {
    h1 *= UCHAR_MAX + 2U;
    h1 += p[i];
  }
  uint32_t h2 = 0;
  p = (unsigned char*)&c;
  for (size_t j = 0; j < sizeof(c); ++j) {
    h2 *= UCHAR_MAX + 2U;
    h2 += p[j];
  }
  return (h1 + differ++) ^ h2;
}

inline void MTRand::save(uint32_t* saveArray) const {
  uint32_t* sa = saveArray;
  const uint32_t* s = state;
  int i = N;
  for (; i--; *sa++ = *s++) {
  }
  *sa = left;
}

inline void MTRand::load(uint32_t* const loadArray) {
  uint32_t* s = state;
  uint32_t* la = loadArray;
  int i = N;
  for (; i--; *s++ = *la++) {
  }
  left = *la;
  pNext = &state[N - left];
}

inline std::ostream& operator<<(std::ostream& os, const MTRand& mtrand) {
  const uint32_t* s = mtrand.state;
  int i = mtrand.N;
  for (; i--; os << *s++ << "\t") {
  }
  return os << mtrand.left;
}

inline std::istream& operator>>(std::istream& is, MTRand& mtrand) {
  uint32_t* s = mtrand.state;
  int i = mtrand.N;
  for (; i--; is >> *s++) {
  }
  is >> mtrand.left;
  mtrand.pNext = &mtrand.state[mtrand.N - mtrand.left];
  return is;
}

// real number in [0,1]
EXPORT double SoarRand();

// real number in [0,n]
EXPORT double SoarRand(const double& max);

// integer in [0,2^32-1]
EXPORT uint32_t SoarRandInt();

// integer in [0,n] for n < 2^32
EXPORT uint32_t SoarRandInt(const uint32_t& max);

// automatically seed with a value based on the time or /dev/urandom
EXPORT void SoarSeedRNG();

// seed with a provided value
EXPORT void SoarSeedRNG(const uint32_t seed);

#endif  // SOAR_RAND_H

// Change log:
//
// v0.1 - First release on 15 May 2000
//      - Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
//      - Translated from C to C++
//      - Made completely ANSI compliant
//      - Designed convenient interface for initialization, seeding, and
//        obtaining numbers in default or user-defined ranges
//      - Added automatic seeding from /dev/urandom or time() and clock()
//      - Provided functions for saving and loading generator state
//
// v0.2 - Fixed bug which reloaded generator one step too late
//
// v0.3 - Switched to clearer, faster reload() code from Matthew Bellew
//
// v0.4 - Removed trailing newline in saved generator format to be consistent
//        with output format of built-in types
//
// v0.5 - Improved portability by replacing static const int's with enum's and
//        clarifying return values in seed(); suggested by Eric Heimburg
//      - Removed MAXINT constant; use 0xffffffffUL instead
//
// v0.6 - Eliminated seed overflow when uint32 is larger than 32 bits
//      - Changed integer [0,n] generator to give better uniformity
//
// v0.7 - Fixed operator precedence ambiguity in reload()
//      - Added access for real numbers in (0,1) and (0,n)
//
// v0.8 - Included time.h header to properly support time_t and clock_t
//
// v1.0 - Revised seeding to match 26 Jan 2002 update of Nishimura and Matsumoto
//      - Allowed for seeding with arrays of any length
//      - Added access for real numbers in [0,1) with 53-bit resolution
//      - Added access for real numbers from normal (Gaussian) distributions
//      - Increased overall speed by optimizing twist()
//      - Doubled speed of integer [0,n] generation
//      - Fixed out-of-range number generation on 64-bit machines
//      - Improved portability by substituting literal constants for long enum's
//      - Changed license from GNU LGPL to BSD
