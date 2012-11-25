#include "bigmath.h"

void run_test(bool (*func)(void), char*);

bool test_print_hex(void);
bool test_shr_segments(void);
bool test_shl_segments(void);
bool test_add_segments(void);
bool test_sub_segments(void);

bigint* get_ones(uint64_t size);
bigint* get_zeros(uint64_t size);
bigint* get_fill(uint64_t size, byte fill);

void assert(bool* accum, bool test);

int main() {
  run_test(&test_print_hex, "hex_print");
  run_test(&test_shr_segments, "shr_segments");
  run_test(&test_shl_segments, "shl_segments");
  run_test(&test_add_segments, "add_segments");
  return 0;
}

void run_test(bool (*func)(void), char* message) {
  printf("==================================\n");
  printf("Running Test: %s\n", message);
  printf("-----------test-output------------\n");
  bool test = (*func)();
  printf("\n----------------------------------\n");
  printf("Result: %s\n", test ? "passed" : "failed");
  printf("==================================\n\n");
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

  printf("max shift (1,64) \n");
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
