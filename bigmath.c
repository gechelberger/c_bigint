#include "bigmath.h"

//************* WARNING ***************
// * If you do not allocate sufficient *
// * digits for an operation, the      *
// * function will segfault.           *
// *                                   *
// * There are no checks               *
// *************************************/

bigint* alloc_bigint(uint64_t digits) {
  return alloc_bigint_base(digits, 10);
}

/**
 * Allocate a new bigint with at least enough bits to contain
 * a number of digits given a particular base
 */
bigint* alloc_bigint_base(uint64_t digits, byte base) {
  static const float l2 = log(2);
  float ratio = log(base) / l2; 
  bigint* value = malloc(sizeof(bigint));
  value->length = (uint64_t) ceil(ratio * digits / (sizeof(uint64_t)*8));
  size_t size = value->length * sizeof(uint64_t);
  value->data = malloc(size);
  memset(value->data, 0, size);
  return value;
}

bigint* create_bigint(uint64_t* segments, uint64_t length) {
  bigint* value = malloc(sizeof(bigint));
  value->data = segments;
  value->length = length;
  return value;
}

void free_bigint(bigint* value) {
  free(value->data);
  free(value);
}

///
///
///

uint64_t* shl_segments(uint64_t* dest, uint64_t length, byte offset) {
  if(offset > sizeof(uint64_t) * 8) {
    return NULL;
  }

  const byte carry_shift = sizeof(uint64_t) * 8 - offset;
  uint64_t i, orig, overflow = 0;
  for(i = 0; i < length; i++) {
    orig = dest[i];
    dest[i] = orig << offset | overflow;
    overflow = orig >> carry_shift;
  }
  return dest;
}

uint64_t* shr_segments(uint64_t* dest, uint64_t length, byte offset) {
  if(offset > sizeof(uint64_t) * 8 || length == 0) {
    return NULL;
  }

  const byte carry_shift = sizeof(uint64_t) * 8 - offset;
  uint64_t i;
  for(i = 0; i < length-1; i++) {
    dest[i] = (dest[i] >> offset) | (dest[i+1] << carry_shift);
  }
  dest[length-1] = dest[length-1] >> offset;
  return dest;
}

/**
 * Requires both dest and incr to have the same length
 */
uint64_t* add_segments(uint64_t* dest, uint64_t* incr, uint64_t length) {
  unsigned long preserve_carry_bool = FALSE;
  unsigned long index = 0;
  asm __volatile__(
      "movq $1, %%r10\n\t"
      "1:movq (%0,%2,8), %%r11\n\t"
      "movq (%1,%2,8), %%r12\n\t"
      "cmp $0x0, %4\n\t"
      "jne 2f\n\t"
      "addq %%r11, %%r12\n\t"
      "cmovc %%r10, %4\n\t" //preserve with carry is already 0 
      "jmp 3f\n\t"
      "2: stc\n\t"
      "movq $0, %4\n\t"
      "adcq %%r11, %%r12\n\t"
      "cmovc %%r10, %4\n\t"
      "3: movq %%r12, (%0,%2,8)\n\t"
      "inc %2\n\t"
      "cmp %2, %3\n\t"
      "jne 1b"
      : /* no outputs */
      : "r" (dest), 
	"r" (incr), 
	"r" (index),
	"r" (length),
	"r" (preserve_carry_bool)
      : "r10", "r11", "r12", "memory"
		   );
  return dest;
}

/**
 * Requires both dest and decr to have the same length
 */
uint64_t* sub_segments(uint64_t* dest, uint64_t* decr, uint64_t length) {
  //check gt/e etc
  unsigned long preserve_carry_bool = FALSE;
  unsigned long index = 0;
  asm __volatile__(
      "movq $1, %%r10\n\t"
      "1:movq (%0,%2,8), %%r12\n\t"
      "movq (%1,%2,8), %%r11\n\t"
      "cmp $0x0, %4\n\t"
      "jne 2f\n\t"
      "subq %%r11, %%r12\n\t"
      "cmovc %%r10, %4\n\t" //preserve with carry is already 0 
      "jmp 3f\n\t"
      "2: stc\n\t"
      "movq $0, %4\n\t"
      "sbbq %%r11, %%r12\n\t"
      "cmovc %%r10, %4\n\t"
      "3: movq %%r12, (%0,%2,8)\n\t"
      "inc %2\n\t"
      "cmp %2, %3\n\t"
      "jne 1b"
      : /* no outputs */
      : "r" (dest), 
	"r" (decr), 
	"r" (index),
	"r" (length),
	"r" (preserve_carry_bool)
      : "r10", "r11", "r12", "memory"
		   );
  return dest;
}

uint64_t* div_segments(uint64_t* dest, uint64_t *divisor, uint64_t length) {
  uint64_t msb_dest = _msb(dest, length);
  uint64_t msb_divisor = _msb(divisor, length);
  int diff;
  if(msb_divisor > msb_dest) {
    memset(dest, 0, length);
    return dest;
  } else if(msb_divisor < msb_dest) {
    diff = msb_dest - msb_divisor;
    while(diff > 32) {
      shl_segments(divisor, length, 32);
      diff -= 32;
    }
    shl_segments(divisor, length, diff);
    //print div and dest to assure alignment
  } //else already aligned

  uint64_t iterations = msb_dest - msb_divisor; //reset

  size_t scratch_size = length * sizeof(uint64_t);
  uint64_t *scratch = malloc(scratch_size);
  memcpy(scratch, dest, scratch_size);
  memset(dest, 0, scratch_size);
  int i;
  for(i = 0; i < iterations; i++) {
    if(gte(scratch, divisor, length)) {
      sub_segments(scratch, divisor, length);
      dest[0] = dest[0] | 1;
    }
    shl_segments(dest, length, 1);
    shl_segments(scratch, length, 1);
  }
  
  free(scratch);

  return dest;
    
  /**
     //align MSB of N and D
     t = N - D;
     if t >= 0 then Q |= 0x1 and N = t
     N <<= 1
     Q <<= 1
     goto t = N-D
   */  
}

bool gt(uint64_t* seg1, uint64_t* seg2, uint64_t length) {
  return _gt(seg1, seg2, length, FALSE);
}

bool gte(uint64_t* seg1, uint64_t* seg2, uint64_t length) {
  return _gt(seg1, seg2, length, TRUE);
}

bool _gt(uint64_t* seg1, uint64_t* seg2, uint64_t length, bool or_equal) {
  int i;
  for(i = length-1; i >= 0; i--) {
    if(seg1[i] == 0 && seg2[i] == 0)
      continue;
    return or_equal ? seg1[i] > seg2[i] : seg1[i] >= seg2[i];
  }
  return or_equal;
}

bool lt(uint64_t* seg1, uint64_t* seg2, uint64_t length) {
  return _lt(seg1, seg2, length, FALSE);
}

bool lte(uint64_t* seg1, uint64_t* seg2, uint64_t length) {
  return _lt(seg1, seg2, length, TRUE);
}

bool _lt(uint64_t* seg1, uint64_t* seg2, uint64_t length, bool or_equal) {
  int i;
  for(i = length-1; i >= 0; i--) {
    if(seg1[i] == 0 && seg2[i] == 0)
      continue;
    return or_equal ? seg1[i] < seg2[i] : seg1[i] <= seg2[i];
  }
  return or_equal;
}


uint64_t _msb(uint64_t* segments, uint64_t length) {
  int i = length - 1;
  byte segment_size_bits = sizeof(uint64_t) * 8;
  uint64_t value;
  for(; i >= 0; i--) {
    value = _log2(segments[i]);
    if(value) {
      return (i * segment_size_bits) + value;
    }
  }
  return 0;
}

byte _log2(uint64_t segment) {
  const uint64_t mask[] = {
    0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000, 0xFFFFFFFF00000000
  };
  const byte offset[] = {
    1, 2, 4, 8, 16, 32
  };
  int i;
  register uint64_t l2;
  for(i = 5; i >= 0; i--) {
    if(segment & mask[i]) {
      segment >>= offset[i];
      l2 = l2 | offset[i];
    }
  }
  return l2;
}

///
///
///

void print_bigint(bigint* bigint) {
  print_bigint_base(bigint, 10);
}

void print_bigint_base(bigint* bigint, byte base) {
  if(base == 16) {
    print_bigint_hex(bigint);
  } else {
    //print eulers bs
  }
}

void print_bigint_hex(bigint* value) {
  size_t size;
  char* buffer, *output;
  uint64_t temp, i, j;
  
  size = sizeof(char) * value->length * sizeof(uint64_t) + 1;
  buffer = malloc(size);
  output = malloc(size);
  output[0] = '\0';
  
  for(i = 0; i < value->length; i++) {
    temp = value->data[i];
    for(j = 0; j < 2*sizeof(uint64_t); j++) {
      strcpy(buffer, output);
      sprintf(output, "%x%s", (byte) temp & 0xf, buffer);
      temp >>= 4;
    }
    //do we add spaces every qword?
  }

  printf("%s", output);
  free(buffer);
  free(output);
}

///
///
///

bigint* shl_bigint(bigint* dest, byte offset) {
  if(offset > sizeof(uint64_t) * 8) {
    return NULL;
  }
  
  if(shl_segments(dest->data, dest->length, offset) == NULL) {
    return NULL;
  }

  return dest;
}

bigint* shr_bigint(bigint* dest, byte offset) {
  if(offset > sizeof(uint64_t) * 8) {
    return NULL;
  }

  if(shr_segments(dest->data, dest->length, offset) == NULL) {
    return NULL;
  }
  return dest;
}

bigint* add_bigint(bigint* dest, bigint* offset) {
  if(dest->length != offset->length) {
    return NULL;
  }

  if(add_segments(dest->data, offset->data, dest->length) == NULL) {
    return NULL;
  }

  return dest;
}

bigint* sub_bigint(bigint* dest, bigint* offset) {
  if(dest->length != offset->length) {
    return NULL;
  }
  if(sub_segments(dest->data, offset->data, dest->length) == NULL) {
    return NULL;
  }
  return dest;
}

bigint* mul_bigint_nat(bigint* dest, uint64_t scale) {
  //todo: opt native
  bigint* multiplier;
  uint64_t* temp = malloc(sizeof(uint64_t)); //later 
  multiplier = create_bigint(temp, 1);
  mul_bigint(dest, multiplier);
  free_bigint(multiplier);
  return dest;
}


bigint* mul_bigint(bigint* dest, bigint* scale) {
  size_t scratch_size = sizeof(uint64_t) * dest->length;
  uint64_t* scratch = malloc(scratch_size);
  memset(scratch, 0, scratch_size);

  int i, j=0;
  uint64_t multiplier;
  for(i = 0; i < scale->length; i++) {
    multiplier = scale->data[i];
    for(j=0; multiplier; j++) {
      if(multiplier & 0x1) {
	add_segments(scratch, dest->data, dest->length);
      }
      shl_segments(dest->data, dest->length, 1);
      multiplier >>= 1;
    }
    shl_segments(dest->data, dest->length, sizeof(uint64_t) * 8 - j);
  }

  memcpy(dest->data, scratch, scratch_size);
  free(scratch);
  return dest;
}

bigint* div_bigint_nat(bigint* dest, uint64_t divisor) {
  uint64_t* segments;
  bigint* scratch;
  
  segments = malloc(sizeof(uint64_t) * dest->length);
  memset(segments, 0, sizeof(uint64_t) * dest->length);
  segments[0] = divisor;
  scratch = create_bigint(segments, dest->length);
  
  div_bigint(dest, scratch);
  free_bigint(scratch);
  return dest;
}

bigint* div_bigint(bigint* dest, bigint* divisor) {
  //divide

  return NULL;
}
