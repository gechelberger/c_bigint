#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TRUE  1
#define FALSE 0

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned long uint64_t;

typedef struct {
  uint64_t* data;
  uint64_t length;
} bigint;

bigint* create_bigint(uint64_t* segments, uint64_t length);
bigint* alloc_bigint(uint64_t digits);
bigint* alloc_bigint_base(uint64_t digits, byte base);
void free_bigint(bigint* bigint);

uint64_t* shl_segments(uint64_t* dest, uint64_t length, byte offset);
uint64_t* shr_segments(uint64_t* dest, uint64_t length, byte offset);
uint64_t* add_segments(uint64_t* dest, uint64_t* incr, uint64_t length);
uint64_t* sub_segments(uint64_t* dest, uint64_t* decr, uint64_t length);
uint64_t* mul_segments(uint64_t* dest, uint64_t* scale, uint64_t length);
uint64_t* div_segments(uint64_t* dest, uint64_t* divisor, uint64_t length);

bool gt(uint64_t* seg1, uint64_t* seg2, uint64_t length);
bool gte(uint64_t* seg1, uint64_t* seg2, uint64_t length);
bool _gt(uint64_t* seg1, uint64_t* seg2, uint64_t length, bool or_equal);

bool lt(uint64_t* seg1, uint64_t* seg2, uint64_t length);
bool lte(uint64_t* seg1, uint64_t* seg2, uint64_t length);
bool _lt(uint64_t* seg1, uint64_t* seg2, uint64_t length, bool or_equal);

uint64_t _msb(uint64_t* segments, uint64_t length);
byte _log2(uint64_t segment);

///

char* bigint_to_new_str(bigint* bigint);
char* bigint_to_new_str_base(bigint* bigint, byte base);
char* bigint_to_new_str_hex(bigint* bigint);

void print_bigint(bigint* bigint);
void print_bigint_base(bigint* bigint, byte base);
void print_bigint_hex(bigint* bigint);

///

bigint* shl_bigint(bigint* dest, byte offset);
bigint* shr_bigint(bigint* dest, byte offset);

bigint* add_bigint(bigint* dest, bigint* offset);
bigint* sub_bigint(bigint* dest, bigint* offset);

bigint* mul_bigint(bigint* dest, bigint* scale);
bigint* mul_bigint_nat(bigint* dest, uint64_t scale);

bigint* div_bigint(bigint* dest, bigint* divisor);
bigint* div_bigint_nat(bigint* dest, uint64_t divisor);





