#include "unity.h"
#include "unity_test_runner.h"
#include <stdio.h>

void print_banner(const char *text);

void app_main(void) {
  UNITY_BEGIN();
  print_banner("Running Test SPI ");
  unity_run_all_tests();
  UNITY_END();
}
void print_banner(const char *text) { printf("\n### %s ###\n\n", text); }
