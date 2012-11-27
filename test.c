#include "bigmath.h"

void run_test(bool (*func)(void), char*);

bool test_print(void);
bool test_print_hex(void);
bool test_shr_segments(void);
bool test_shl_segments(void);
bool test_add_segments(void);
bool test_sub_segments(void);

/*
bool test_shl(void);
bool test_shr(void);
bool test_add(void);
bool test_sub(void);
bool test_mul(void);
bool test_mult_nat(void);
bool test_div(void);
bool test_div_nat(void);
*/


//TODO:
bool test_mul_segments(void);
bool test_div_segments(void);

bool test_pow(void);

bool test_gt(void);
bool test_gte(void);
bool test_lt(void);
bool test_lte(void);
bool test_eq(void);

bool test_msb(void);
bool test_log2(void);

//END TODO


bigint* get_ones(uint64_t size);
bigint* get_zeros(uint64_t size);
bigint* get_fill(uint64_t size, byte fill);

void assert(bool* accum, bool test);

int main() {
  setbuf(stdout, NULL);
  run_test(&test_print_hex, "hex_print");
  run_test(&test_shr_segments, "shr_segments");
  run_test(&test_shl_segments, "shl_segments");
  run_test(&test_add_segments, "add_segments");
  run_test(&test_sub_segments, "sub_segments");
  run_test(&test_msb, "most significant bit");
  run_test(&test_log2, "integer log2 of uint64_t");
  run_test(&test_gt, "greater than");
  run_test(&test_gte, "greater or equal");
  run_test(&test_lt, "less than");
  run_test(&test_lte, "less or equal");
  run_test(&test_eq, "equals");
  run_test(&test_mul_segments, "mul_segments");
  run_test(&test_div_segments, "div_segments");
  run_test(&test_print, "print_decimal");
  run_test(&test_pow, "pow_segments");
  return 0;
}

void run_test(bool (*func)(void), char* message) {
  printf("==================================\n");
  printf("Running Test: %s\n", message);
  printf("-----------test-output------------\n");
  bool test = (*func)();
  printf("\n----------------------------------\n");
  printf("Result: %s\n", test ? "PASS" : "FAILURE");
  printf("==================================\n\n");
}

bool test_print() {
  bool test = TRUE;
  bigint* val = get_zeros(5);
  val->data[0] = 15;

  char* repr = bigint_to_new_str_base(val, 10);
  printf("%s\n", repr);

  assert(&test, strcmp(repr, "15") == 0);
  free(repr);
  free(val);

  val = get_ones(1000);
  repr = bigint_to_new_str_base(val, 10);
  printf("%s\n", repr);
  free(repr);
  free(val);
  return test;
}


bool test_shr_segments() {
  static int size = 5;
  bool test = TRUE;
  int i;

  uint64_t *segments = malloc(size * sizeof(uint64_t));
  bigint* for_printing = create_bigint(segments, size);

  for(i = 0; i < size; i++) {
    segments[i] = 0x4000000000000000;
  }

  
  print_bigint_hex(for_printing);
  printf("\nShift by 2:\n");

  //test normal shift
  shr_segments(segments, size, 2);

  for(i = 0; i < size; i++) {
    test = test && segments[i] == 0x1000000000000000;
  }

  print_bigint_hex(for_printing);
  printf("\nExpected: 0x10000000000000000 x %d\n", size);

  printf("Shift by 60:\n");
  shr_segments(segments, size, 60);
  
  for(i = 0; i < size; i++) {
    test = test && segments[i] == 0x1;
  }

  print_bigint_hex(for_printing);
  printf("\nExpected: 0x01 x %d\n", size);

  printf("Shift by 1 (to next qword):\n");
  shr_segments(segments, size, 1);
  for(i = 0; i < size-1; i++) {
    test = test && segments[i] == 0x8000000000000000;
  }
  test = test && segments[size-1] == 0x0;
  print_bigint_hex(for_printing);
  printf("\nExpected: 0x00 0x8000000000000000 x %d\n", size-1);
  //free(segments);
  free_bigint(for_printing);
  return test;
}

bool test_shl_segments() {
  static int size = 3;
  bool test = TRUE;
  int i;
  uint64_t *segments = malloc(sizeof(uint64_t) * size);
  bigint* for_printing = create_bigint(segments, size);
  for(i = 0; i < size; i++) {
    segments[i] = 0x1;
  }
  
  print_bigint_hex(for_printing);
  printf("\nShifting by 60:\n");
  shl_segments(segments, size, 63);
  for(i = 0; i < size; i++) {
    test = test && segments[i] == 0x8000000000000000;
  }
  print_bigint_hex(for_printing);
  printf("\nExpecting: 0x8000000000000000 x 3\n");

  printf("max shift (2,63) \n");
  shl_segments(segments, size, 2);
  shl_segments(segments, size, 63);
  print_bigint_hex(for_printing);
  for(i = size-1; i >= 2; i--) {
    test = test && segments[i] == 0x1;
  }
  test = test && segments[0] == 0x0 && segments[1] == 0x0;
  printf("\nExpecting: 0x1 0x0 0x0\n");
  

  free_bigint(for_printing);
  return test;
}

bool test_print_hex() {
  static int digits = 50;
  bool test = TRUE;

  bigint* value = alloc_bigint(digits);
  printf("%d digits: %lu fields\n", digits, value->length);
  value->data[0] = 0xFF00F0F0;
  value->data[1] = 0xFFFF0000FFFF0000;
  value->data[2] = 0xFFFFFFFF00000000;

  char* output = bigint_to_new_str_hex(value);
  test = test && 0 == strcmp(output, "ffffffff00000000ffff0000ffff000000000000ff00f0f0");
  printf("%s\n", output);
  free(output);

  return test;
}

bool test_add_segments() {
  bool test = TRUE;
  int i;

  assert(&test, TRUE == 0x1);

  bigint* base = get_ones(5);
  base->data[4] = 0;
  printf("   ");
  print_bigint_hex(base);
  printf("\n");

  bigint* incr = get_ones(5);
  incr->data[4] = 0;
  printf(" + ");
  print_bigint_hex(incr);
  printf("\n");

  uint64_t *result = add_segments(base->data, incr->data, 5);

  for(i = 0; i < 5; i++) {
    printf("----------------");
  }
  printf("---\n   ");
  print_bigint_hex(base);
  printf("\nExpecting 0x1 0xFFFFFFFFFFFFFFFF x3 0xFFFFFFFFFFFFFFFE\nIncr:\n");

  assert(&test, base->data[0] == 0xFFFFFFFFFFFFFFFE);
  for(i=1; i < 4; i++) {
    assert(&test, base->data[i] == 0xFFFFFFFFFFFFFFFF);
  }
  assert(&test, base->data[4] == 0x1);

  print_bigint_hex(incr);
  printf("\nExpecting: 0x0 0xFFFFFFFFFFFFFFFF x4\n");
  assert(&test, incr->data[4] == 0);
  for(i = 0; i < 4; i++) {
    assert(&test, incr->data[i] == 0xFFFFFFFFFFFFFFFF);
  }

  assert(&test, base->data == result);

  free_bigint(base);
  free_bigint(incr);
  

  return test;
}

bool test_sub_segments() {
  bigint* major = get_ones(5);
  bigint* minor = get_ones(5);
  major->data[0] = 0;
  minor->data[4] = 0;
  
  printf("   ");
  print_bigint_hex(major);
  printf("\n - ");
  print_bigint_hex(minor);
  int i;
  printf("\n");
  for(i=0; i < 5;i++) {
    printf("----------------");
  }
  printf("---\n = ");

  uint64_t* result = sub_segments(major->data, minor->data, 5);

  print_bigint_hex(major);

  bool test = TRUE;
  assert(&test, major->data[0] == 0x1);
  for(i = 1; i < 4; i++) {
    assert(&test, major->data[i] == 0xFFFFFFFFFFFFFFFF);
  }
  assert(&test, major->data[4] == 0xFFFFFFFFFFFFFFFE);

  assert(&test, result == major->data);

  for(i = 0; i < 4; i++) {
    assert(&test, minor->data[i] == 0xFFFFFFFFFFFFFFFF);
  }
  assert(&test, minor->data[4] == 0x0);

  return test;
}

bool test_mul_segments() {
  bool test = TRUE;
  int i;
  bigint* ones_dest  = get_ones(6);
  bigint* ones_scale = get_ones(6);
  for(i = 2; i < 6; i++) {
    ones_dest->data[i] = 0;
    ones_scale->data[i] = 0;
  }
  
  print_bigint_hex(ones_dest); 
  printf("\n");
  print_bigint_hex(ones_scale);
  printf("\n");

  mul_segments(ones_dest->data, ones_scale->data, 6);

  assert(&test, ones_dest->data[0] == 0x1);
  assert(&test, ones_dest->data[1] == 0x0);
  assert(&test, ones_dest->data[2] == 0xFFFFFFFFFFFFFFFE);
  assert(&test, ones_dest->data[3] == 0xFFFFFFFFFFFFFFFF);
  assert(&test, ones_dest->data[4] == 0x0);
  assert(&test, ones_dest->data[5] == 0x0);

  print_bigint_hex(ones_dest);

  free(ones_dest);
  free(ones_scale);

  return test;
}
bool test_div_segments() {
  return FALSE;
}

bool test_gt() {
  return FALSE;
}
bool test_gte() {
  return FALSE;
}
bool test_lt() {
  return FALSE;
}
bool test_lte() {
  return FALSE;
}
bool test_eq() {
  return FALSE;
}

bool test_msb() {
  return FALSE;
}
bool test_log2() {
  return FALSE;
}

bool test_pow() {
  bigint* raise = get_zeros(1);
  raise->data[0] = 0xf;

  pow_segments(raise->data, 0xf, 1);

  bool test = TRUE;
  assert(&test, raise->data[0] == 0x0613b62c597707ef);

  free(raise);
  raise = get_zeros(32);
  raise->data[0] = 0xff;
  pow_segments(raise->data, 0xff, 32);

  print_bigint_hex(raise);
  char* output = bigint_to_new_str_hex(raise);
  printf("\n0xff ^ 0xff = %s\n0xfff ^ 0xfff", output);
  assert(&test, strcmp(output, "005e5c8b0eb95ab08f9d37ef127fc01bd0e33de52647528396d78d5f8da31989e67814f6bba1fb0f0207010ff5f2347b19d5f6598fc91bf5a88f77daa3d7b382fec484f3d205c06a34445384c0e7ab0d883788c68c012cb433055edda746a48409444ea91147273b79fc3eabb70eca552af650c234bb01ed404427f17cdddd71d08e39ef9c3982e3ce44e670456aa8154c1fdbd9c35947f494636a425c69bf89e9c75ad3b7a0a559af0f5da9947c8deba64417310713b23e7ef4de50bb2a3e90bc2ac3da5201cca8d6e5dfea887c4f7a4e92175d9f88bd2779b57f9eb35be7528f965a06da0ac41dcb3a34f1d8ab7d8fee620a94faa42c395997756b007ffeff") == 0);
  free(raise);
  free(output);

  raise = get_zeros(768);
  raise->data[0] = 0xfff;
  pow_segments(raise->data, 0xfff, 768);
  printf(" = ");
  print_bigint_hex(raise);
  free(raise);

  /*  raise = get_zeros(16384);
  raise->data[0] = 0x1fff;
  printf("\n0x1fff ^ 0x1fff");
  pow_segments(raise->data, 0x1fff, 16384);
  printf(" = ");
  print_bigint_hex(raise);
  free(raise);*/

  return test;
}

///
///
///


bigint *get_ones(uint64_t size) {
  return get_fill(size, 0xff);
}

bigint *get_zeros(uint64_t size) {
  return get_fill(size, 0);
}

bigint *get_fill(uint64_t size, byte fill) {
  uint64_t *segments = malloc(sizeof(uint64_t) * size);
  bigint *new = create_bigint(segments, size);
  memset(segments, fill, size * sizeof(uint64_t));
  return new;
}

void assert(bool* accum, bool test) {
  *accum = *accum && test;
}
