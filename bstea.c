#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "bstea.h"

/* a key schedule constant - 32/golden-ratio */
static const uint32_t delta = 0x9e3779b9;

/* v points to the wordsize-way vectorized plaintext, 
 * k to the vectorized key */
/* input quantities are disposed in the following way:
     v0 <- v[0..31]      k0 <- k[0..31]      k2 <- k[64..95]
     v1 <- v[32..63]     k1 <- k[32..63]     k3 <- k[96..127]
 */
void encrypt(parallel_blocks_t v, const parallel_keys_t k, unsigned int r)
{
   /* Stride 32 between consecutive words in input quantities */
#  define offset_v0 0
#  define offset_v1 32
#  define offset_k0 0
#  define offset_k1 32
#  define offset_k2 64
#  define offset_k3 96

   vector_width_t carry;
   vector_width_t axorb;
   vector_width_t aandb;
   vector_width_t ai;
   vector_width_t bi;
   vector_width_t borrow;
   vector_width_t notaandb;

   vector_width_t v1_lshift_4[32];
   vector_width_t v1_plus_sum[32]; /* term two */
   vector_width_t v1_rshift_5[32];
   vector_width_t v1_lshift_4_plus_k0[32]; /* term one */
   vector_width_t v1_rshift_5_plus_k1[32]; /* term three */

   vector_width_t v0_lshift_4[32];
   vector_width_t v0_plus_sum[32]; /* term  two */
   vector_width_t v0_rshift_5[32]; 
   vector_width_t v0_lshift_4_plus_k2[32]; /* term one */
   vector_width_t v0_rshift_5_plus_k3[32]; /* term three */

   vector_width_t shift;

   int i;


   /* setup */
   uint32_t sum   = 0;
   for (i = 0; i < 32; ++i) 
     v1_lshift_4[i] = v1_plus_sum[i] = v1_rshift_5[i] = \
     v1_lshift_4_plus_k0[i] = v1_rshift_5_plus_k1[i] =  \
     v0_lshift_4[i] = v0_plus_sum[i] = v0_rshift_5[i] = \
     v0_lshift_4_plus_k2[i] = v0_rshift_5_plus_k3[i] = 0;


   while (r > 0) {
     sum += delta;

     /* lshift v1 by 4 */
     shift = 4;
     for (i = 31; i >= 0; i--)
        v1_lshift_4[i] = (i >= shift) ? v[offset_v1 + i - shift] : 0;

     /* add k0 to v1_lshift_4 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v1_lshift_4[i];
       bi = k[offset_k0 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_lshift_4_plus_k0[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* add delta sum to v1 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       /* VECTOR_AT_ONE where the ith bit of the sum is set */
       /* 
        * Each iteration follows the first 32 elements 
        * in the expansion of multiples of 32/golden-ratio, 
        * or 32/(1+sqrt(5)/2
        */
       ai = (sum & (1<<i)) ? VECTOR_AT_ONE : VECTOR_AT_ZERO;
       bi = v[offset_v1 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_plus_sum[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* rshift v1 by 5 */
     shift = 5;
     for (i = 0; i < 32; ++i) 
       v1_rshift_5[i] = (i < (32 - shift)) ? v[offset_v1 + i + shift] : 0;

     /* add k1 to v1_rshift_5 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v1_rshift_5[i];
       bi = k[offset_k1 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_rshift_5_plus_k1[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* xor the three terms and increment v0 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v1_lshift_4_plus_k0[i] ^ v1_plus_sum[i] ^ v1_rshift_5_plus_k1[i];
       bi = v[offset_v0 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v[offset_v0 + i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }



     /* lshift v0 by 4 */
     shift = 4;
     for (i = 31; i >= 0; i--)
        v0_lshift_4[i] = (i >= shift) ? v[offset_v0 + i - shift] : 0;

     /* add k2 and v0_lshift_4 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v0_lshift_4[i];
       bi = k[offset_k2 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_lshift_4_plus_k2[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* add delta sum to v0 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       /* VECTOR_AT_ONE where the ith bit of the sum is set */
       ai = (sum & (1<<i)) ? VECTOR_AT_ONE : VECTOR_AT_ZERO;
       bi = v[offset_v0 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_plus_sum[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }


     /* rshift v0 by 5 */
     shift = 5;
     for (i = 0; i < 32; ++i) 
       v0_rshift_5[i] = (i < (32 - shift)) ? v[offset_v0 + i + shift] : 0;

     /* add k3 to v0_rshift_5 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v0_rshift_5[i];
       bi = k[offset_k3 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_rshift_5_plus_k3[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* xor the three terms and increment v1 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v0_lshift_4_plus_k2[i] ^ v0_plus_sum[i] ^ v0_rshift_5_plus_k3[i];
       bi = v[offset_v1 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v[offset_v1 + i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }


     --r;
   }
}

/* v points to the wordsize-way vectorized ciphertext,
 * k to the vectorized key */
/* input quantities are disposed in the following way:
     v0 <- v[0..31]      k0 <- k[0..31]      k2 <- k[64..95]
     v1 <- v[32..63]     k1 <- k[32..63]     k3 <- k[96..127]
 */
void decrypt(parallel_blocks_t v, const parallel_keys_t k, unsigned int r)
{
#  define offset_v0 0
#  define offset_v1 32
#  define offset_k0 0
#  define offset_k1 32
#  define offset_k2 64
#  define offset_k3 96

   vector_width_t carry;
   vector_width_t axorb;
   vector_width_t aandb;
   vector_width_t ai;
   vector_width_t bi;
   vector_width_t borrow;
   vector_width_t notaandb;
  
   vector_width_t v1_lshift_4[32];
   vector_width_t v1_plus_sum[32]; /* term two */
   vector_width_t v1_rshift_5[32];
   vector_width_t v1_lshift_4_plus_k0[32]; /* term one */
   vector_width_t v1_rshift_5_plus_k1[32]; /* term three */

   vector_width_t v0_lshift_4[32];
   vector_width_t v0_plus_sum[32]; /* term  two */
   vector_width_t v0_rshift_5[32]; 
   vector_width_t v0_lshift_4_plus_k2[32]; /* term one */
   vector_width_t v0_rshift_5_plus_k3[32]; /* term three */

   vector_width_t shift;

   int i;

   /* setup */
   uint32_t sum = delta * r;

   for (i = 0; i < 32; ++i) 
     v1_lshift_4[i] = v1_plus_sum[i] = v1_rshift_5[i] = \
     v1_lshift_4_plus_k0[i] = v1_rshift_5_plus_k1[i] =  \
     v0_lshift_4[i] = v0_plus_sum[i] = v0_rshift_5[i] = \
     v0_lshift_4_plus_k2[i] = v0_rshift_5_plus_k3[i] = 0;

   while (r > 0) {
     /* lshift v0 by 4 */
     shift = 4;
     for (i = 31; i >= 0; i--)
        v0_lshift_4[i] = (i >= shift) ? v[offset_v0 + i - shift] : 0;

     /* add k2 and v0_lshift_4 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v0_lshift_4[i];
       bi = k[offset_k2 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_lshift_4_plus_k2[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* add delta sum to v0 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       /* VECTOR_AT_ONE where the ith bit of the sum is set */
       ai = (sum & (1<<i)) ? VECTOR_AT_ONE : VECTOR_AT_ZERO;
       bi = v[offset_v0 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_plus_sum[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }


     /* rshift v0 by 5 */
     shift = 5;
     for (i = 0; i < 32; ++i) 
       v0_rshift_5[i] = (i < (32 - shift)) ? v[offset_v0 + i + shift] : 0;

     /* add k3 to v0_rshift_5 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v0_rshift_5[i];
       bi = k[offset_k3 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v0_rshift_5_plus_k3[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* xor the three terms and decrement v1 */
     borrow = 0;
     for (i = 0;i < 32;++i) {
       ai = v[offset_v1 + i];
       bi = v0_lshift_4_plus_k2[i] ^ v0_plus_sum[i] ^ v0_rshift_5_plus_k3[i];
       notaandb = (ai ^ VECTOR_AT_ONE) & bi;
       axorb = ai ^ bi;
       v[offset_v1 + i] = axorb ^ borrow;
       borrow = notaandb | ((ai ^ VECTOR_AT_ONE) & borrow) | (bi & borrow);
     }




     /* lshift v1 by 4 */
     shift = 4;
     for (i = 31; i >= 0; i--)
        v1_lshift_4[i] = (i >= shift) ? v[offset_v1 + i - shift] : 0;

     /* add k0 to v1_lshift_4 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v1_lshift_4[i];
       bi = k[offset_k0 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_lshift_4_plus_k0[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* add delta sum to v1 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       /* VECTOR_AT_ONE where the ith bit of the sum is set */
       ai = (sum & (1<<i)) ? VECTOR_AT_ONE : VECTOR_AT_ZERO;
       bi = v[offset_v1 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_plus_sum[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* rshift v1 by 5 */
     shift = 5;
     for (i = 0; i < 32; ++i) 
       v1_rshift_5[i] = (i < (32 - shift)) ? v[offset_v1 + i + shift] : 0;

     /* add k1 to v1_rshift_5 */
     carry = 0;
     for (i = 0;i < 32;++i) {
       ai = v1_rshift_5[i];
       bi = k[offset_k1 + i];
       aandb = ai & bi;
       axorb = ai ^ bi;
       v1_rshift_5_plus_k1[i] = axorb ^ carry;
       carry &= axorb;
       carry |= aandb;
     }

     /* xor the three terms and decrement v0 */
     borrow = 0;
     for (i = 0;i < 32;++i) {
       ai = v[offset_v0 + i];
       bi = v1_lshift_4_plus_k0[i] ^ v1_plus_sum[i] ^ v1_rshift_5_plus_k1[i];
       notaandb = (ai ^ VECTOR_AT_ONE) & bi;
       axorb = ai ^ bi;
       v[offset_v0 + i] = axorb ^ borrow;
       borrow = notaandb | ((ai ^ VECTOR_AT_ONE) & borrow) | (bi & borrow);
     }


     sum -= delta;
     --r;
   }
}

