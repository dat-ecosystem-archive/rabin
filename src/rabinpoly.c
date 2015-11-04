// $Id$

/*
 *
 * Copyright (C) 2004 Hyang-Ah Kim (hakim@cs.cmu.edu)
 * Copyright (C) 1999 David Mazieres (dm@uun.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

#include "rabinpoly.h"
#include "msb.h"
#define INT64(n) n##LL
#define MSB64 INT64(0x8000000000000000)
                    
u_int64_t
polymod (u_int64_t nh, u_int64_t nl, u_int64_t d)
{
#ifdef _LPCOX_DEBUG_
  printf ("polymod (nh %llu nl %llu d %llu)\n", nh, nl, d);
#endif
  int k = fls64 (d) - 1;
#ifdef _LPCOX_DEBUG_
  printf ("polymod : k = %d\n", k);
#endif
  d <<= 63 - k;
#ifdef _LPCOX_DEBUG_
  printf ("polymod : d = %llu\n", d);
  printf ("polymod : MSB64 = %llu\n", MSB64);
#endif

  if (nh) {
    if (nh & MSB64)
      nh ^= d;
#ifdef _LPCOX_DEBUG_
    printf ("polymod : nh = %llu\n", nh);
#endif
    for (int i = 62; i >= 0; i--)
      if (nh & ((u_int64_t) 1) << i) {
	nh ^= d >> (63 - i);
	nl ^= d << (i + 1);
#ifdef _LPCOX_DEBUG_
	printf ("polymod : i = %d\n", i);
	printf ("polymod : shift1 = %llu\n", (d >> (63 - i)));
	printf ("polymod : shift2 = %llu\n", (d << (i + 1)));
	printf ("polymod : nh = %llu\n", nh);
	printf ("polymod : nl = %llu\n", nl);
#endif
      }
  }
  for (int i = 63; i >= k; i--)
  {  
    if (nl & INT64 (1) << i)
      nl ^= d >> 63 - i;
#ifdef _LPCOX_DEBUG_
    printf ("polymod : nl = %llu\n", nl);
#endif
  }
  
#ifdef _LPCOX_DEBUG_
  printf ("polymod : returning %llu\n", nl);
#endif
  return nl;
}

u_int64_t
polygcd (u_int64_t x, u_int64_t y)
{
  for (;;) {
    if (!y)
      return x;
    x = polymod (0, x, y);
    if (!x)
      return y;
    y = polymod (0, y, x);
  }
}

void
polymult (u_int64_t *php, u_int64_t *plp, u_int64_t x, u_int64_t y)
{
#ifdef _LPCOX_DEBUG_
  printf ("polymult (x %llu y %llu)\n", x, y);
#endif
  u_int64_t ph = 0, pl = 0;
  if (x & 1)
    pl = y;
  for (int i = 1; i < 64; i++)
    if (x & (INT64 (1) << i)) {
#ifdef _LPCOX_DEBUG_
      printf ("polymult : i = %d\n", i);
      printf ("polymult : ph = %llu\n", ph);
      printf ("polymult : pl = %llu\n", pl);
      printf ("polymult : y = %llu\n", y);
      printf ("polymult : ph ^ y >> (64-i) = %llu\n", (ph ^ y >> (64-i)));
      printf ("polymult : pl ^ y << i = %llu\n", (pl ^ y << i));
#endif
      ph ^= y >> (64 - i);
      pl ^= y << i;
#ifdef _LPCOX_DEBUG_
  printf ("polymult : ph %llu pl %llu\n", ph, pl);
#endif
    }
  if (php)
    *php = ph;
  if (plp)
    *plp = pl;
#ifdef _LPCOX_DEBUG_
  printf ("polymult : h %llu l %llu\n", ph, pl);
#endif
}

u_int64_t
polymmult (u_int64_t x, u_int64_t y, u_int64_t d)
{
#ifdef _LPCOX_DEBUG_
  printf ("polymmult (x %llu y %llu d %llu)\n", x, y, d);
#endif
  u_int64_t h, l;
  polymult (&h, &l, x, y);
  return polymod (h, l, d);
}

bool
polyirreducible (u_int64_t f)
{
  u_int64_t u = 2;
  int m = (fls64 (f) - 1) >> 1;
  for (int i = 0; i < m; i++) {
    u = polymmult (u, u, f);
    if (polygcd (f, u ^ 2) != 1)
      return false;
  }
  return true;
}

void
rabinpoly::calcT ()
{
#ifdef _LPCOX_DEBUG_
  printf ("rabinpoly::calcT ()\n");
#endif
//  assert (poly >= 0x100);
  int xshift = fls64 (poly) - 1;
  shift = xshift - 8;
  u_int64_t T1 = polymod (0, INT64 (1) << xshift, poly);
  for (int j = 0; j < 256; j++)
  {
    T[j] = polymmult (j, T1, poly) | ((u_int64_t) j << xshift);
#ifdef _LPCOX_DEBUG_
    printf ("rabinpoly::calcT tmp = %llu\n", polymmult (j, T1, poly));
    printf ("rabinpoly::calcT shift = %llu\n", ((u_int64_t) j <<
						xshift));
    printf ("rabinpoly::calcT xshift = %d\n", xshift);
    printf ("rabinpoly::calcT T[%d] = %llu\n", j, T[j]);
#endif
  }
#ifdef _LPCOX_DEBUG_
  printf ("rabinpoly::calcT xshift = %d\n", xshift);
  printf ("rabinpoly::calcT T1 = %llu\n", T1);
  printf ("rabinpoly::calcT T = {");
  for (int i=0; i< 256; i++)
    printf ("\t%llu \n", T[i]);
  printf ("}\n");
#endif
}

rabinpoly::rabinpoly (u_int64_t p)
  : poly (p)
{
  calcT ();
}

window::window (u_int64_t poly, unsigned int winsz)
  : rabinpoly (poly), size(winsz), fingerprint (0), bufpos (-1)
{
  u_int64_t sizeshift = 1;
  for (int i = 1; i < size; i++)
    sizeshift = append8 (sizeshift, 0);
  for (int i = 0; i < 256; i++)
    U[i] = polymmult (i, sizeshift, poly);
  buf = new unsigned char[winsz];
  bzero ((char*) buf, winsz*sizeof (u_char));
}

// XXX: previously deallocating buf was missing.- hakim
window::~window() 
{
	delete[] buf;
}
