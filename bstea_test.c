#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "bstea.h"



/* pack and unpack a single value all over the data path */
static void pack(uint32_t *v, size_t len, vector_width_t *bv) {
   size_t i, p, offset = 0;

   for (i=0; i<len; ++i, offset += 32) 
     for (p = 0; p < 32; ++p) 
       bv[offset + p] = (v[i] & (1<<p)) ? VECTOR_AT_ONE : VECTOR_AT_ZERO;
}


static void unpack(vector_width_t *bv, int len, uint32_t *v) {
   int i;

   for (i=0; i<len; i++) 
     if (bv[i]) v[i>>5] |= 1<<(i%32);
}

/* pack and unpack one element at a time */
static void pack_elem(uint32_t *v, size_t len, vector_width_t *bv, int elem) {
   size_t i, p, offset = 0;

   for (i=0; i<len; ++i, offset += 32)
     for (p = 0; p < 32; ++p)  
       bv[offset + p] |= (v[i] & (1<<p)) ? (1<<(elem)) : 0;
}

static void unpack_elem(vector_width_t *bv, int len, uint32_t *v, int elem) {
   int i;

   for (i=0; i<len; i++)
     if (bv[i] & (1<<elem)) v[i>>5] |= 1<<(i%32);
}

typedef struct tvector_s {
  uint32_t ptext[TEA_BLOCK_SIZE >> 5];
  uint32_t ctext[TEA_BLOCK_SIZE >> 5];
  uint32_t   key[TEA_KEY_SIZE   >> 5];
} tvector_t;

static void test_vectors() {
  int i, j;
  parallel_blocks_t v;
  parallel_keys_t k;
  uint32_t ctext[TEA_BLOCK_SIZE >> 5];
  uint32_t ptext[TEA_BLOCK_SIZE >> 5];
  uint32_t key[TEA_KEY_SIZE >> 5];

  tvector_t testv [] =  { { {0x00000000, 0x00000000}, \
                            {0x41ea3a0a, 0x94baa940}, \
                            {0x00000000, 0x00000000,  \
                             0x00000000, 0x00000000} }, \
                          { {0x74736574, 0x2e656d20}, \
                            {0x6a2a5d77, 0x0992cef6}, \
                            {0x6805022b, 0x76491406,  \
                             0x260e5d77, 0x4378286c} }, \
                          { {0x94baa940, 0x00000000}, \
                            {0x4e8e7829, 0x7d8236d8}, \
                            {0x00000000, 0x00000000,  \
                             0x00000000, 0x41ea3a0a} }, \
                          { {0x7d8236d8, 0x00000000}, \
                            {0xc88ba95e, 0xe7edac02}, \
                            {0x00000000, 0x00000000,  \
                             0x41ea3a0a, 0x4e8e7829} } };

  for (i = 0; i < sizeof(testv)/sizeof(tvector_t); ++i) {
     for (j = 0;j < TEA_BLOCK_SIZE;++j) v[j] = 0;
     for (j = 0;j < TEA_KEY_SIZE;++j) k[j] = 0;

     (void) memset(&ctext, 0, 8);
     (void) memset(&ptext, 0, 8);
     (void) memset(&key, 0, 16);

     pack(testv[i].ptext, 2, v);
     pack(testv[i].key, 4, k);

     encrypt(v,k,TEA_ROUNDS);

     unpack(v, 64, ctext);

     decrypt(v,k,TEA_ROUNDS);

     unpack(v, 64, ptext);
     unpack(k, 128, key);

#if 0
  printf("key[0]: 0x%8x, key[1]: 0x%8x, key[2]: 0x%8x, key[3]: 0x%8x\n", key[0], key[1], key[2], key[3]);
  printf("ctext[0]: 0x%8x, ctext[1]: 0x%8x\n", ctext[0], ctext[1]);
  printf("ptext[0]: 0x%8x, ptext[1]: 0x%8x\n", ptext[0], ptext[1]);

  printf("t1_ctext[0]: 0x%8x, t1_ctext[1]: 0x%8x\n", testv[i].ctext[0], testv[i].ctext[1]);
  printf("t1_ptext[0]: 0x%8x, t1_ptext[1]: 0x%8x\n\n\n", testv[i].ptext[0], testv[i].ptext[1]);
#endif

     assert(testv[i].ctext[0] == ctext[0] \
         && testv[i].ctext[1] == ctext[1]);
     assert(testv[i].ptext[0] == ptext[0] \
         && testv[i].ptext[1] == ptext[1]);
     assert(testv[i].key[0] == key[0] \
        &&  testv[i].key[1] == key[1] \
        &&  testv[i].key[2] == key[2] \
        &&  testv[i].key[3] == key[3] );

     printf("test vector, %i,\t[PASSED]\n", i);
  }
}


#ifdef __BSTEA_MAIN_

int main(int argc, char *argv[]) {
  test_vectors();

  return 0;
}

#endif /* __BSTEA_MAIN_ */
