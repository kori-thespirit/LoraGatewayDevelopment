#include "SPI_Stub.h"
#include "esp_err.h"
#include "esp_log.h"
#include "unity.h"
#include <assert.h>
#include <string.h>
static const char *tag = "SPI Test";
static const int SCK_PIN = GPIO_NUM_4;
static const int MOSI_PIN = GPIO_NUM_10;
static const int MISO_PIN = GPIO_NUM_9;
static const int CS_PIN = GPIO_NUM_20;
static const int SPI_MODE_USER = SPI2_HOST;
SPI_Stub_handle_t SPI_Handle;
void setUp(void) {

  SPI_config cfg;
  // Arrange
  ESP_LOGE(tag, "###\nPin Initsialize\n\n");
  cfg = (SPI_config){.SCK = SCK_PIN,
                     .MOSI = MOSI_PIN,
                     .MISO = MISO_PIN,
                     .CS = CS_PIN,
                     .host = SPI_MODE_USER};

  esp_err_t err = SPI_Stub_Init(&SPI_Handle, &cfg);
  assert(err == ESP_OK);
}

void tearDown(void) { SPI_Stub_Deinit(&SPI_Handle); }

TEST_CASE("Slave Unit Test Case 1", "[SPI]") {
  ESP_LOGE(tag, "ReInitsialized SPI....");
  SPI_config cfg;
  cfg = (SPI_config){.SCK = SCK_PIN,
                     .MOSI = MOSI_PIN,
                     .MISO = MISO_PIN,
                     .CS = CS_PIN,
                     .host = SPI_MODE_USER};
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Init(&SPI_Handle, &cfg),
                          ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 2", "[SPI]") {
  SPI_config cfg;
  cfg = (SPI_config){.SCK = SCK_PIN,
                     .MOSI = MOSI_PIN,
                     .MISO = MISO_PIN,
                     .CS = CS_PIN,
                     .host = SPI_MODE_USER};

  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Init(NULL, &cfg), ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 3", "[SPI]") {
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Init(&SPI_Handle, NULL),
                          ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 4", "[SPI]") {
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Deinit(&SPI_Handle), ESP_OK);
}

TEST_CASE("Slave Unit Test Case 5", "[SPI]") {
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Begin_Get_Info(NULL), ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 6", "[SPI]") {
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Begin_Get_Info(NULL), ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 7", "[SPI]") {
  TEST_ASSERT_EQUAL_UINT8(SPI_Stub_Begin_Get_Info(&SPI_Handle), ESP_OK);
}
TEST_CASE("Slave Unit Test Case 8", "[SPI]") {
  static uint8_t Data[4] = {0};
  TEST_ASSERT_EQUAL_UINT8(
      Send_SPI_Stub_Register(&SPI_Handle, Data, sizeof(Data)), ESP_OK);
}
TEST_CASE("Slave Unit Test Case 9", "[SPI]") {
  static uint8_t Data[4] = {0};
  TEST_ASSERT_EQUAL_UINT8(
      Send_SPI_Stub_Register(&SPI_Handle, Data, sizeof(Data) + 10),
      ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 10", "[SPI]") {
  static uint8_t Data[4] = {0};
  TEST_ASSERT_EQUAL_UINT8(Send_SPI_Stub_Register(NULL, Data, sizeof(Data)),
                          ESP_ERR_INVALID_ARG);
}
TEST_CASE("Slave Unit Test Case 11", "[SPI]") {
  static uint8_t *Data = NULL;
  TEST_ASSERT_EQUAL_UINT8(Send_SPI_Stub_Register(NULL, Data, sizeof(Data)),
                          ESP_ERR_INVALID_ARG);
}
// TEST_CASE("Slave Unit Test Case 12", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 13", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 14",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  15", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 16", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 17", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 18", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 19", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 20", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 21", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 22", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 23",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  24", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 25", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 26", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 27", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 28", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 29", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 30", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 31", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 32",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  33", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 34", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 35", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 36", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 37", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 38", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 39", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 40", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 41",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  42", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 43", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 44", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 45", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 46", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 47", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 48", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 49", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 50",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  51", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 52", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 53", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 54", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 55", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 56", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 57", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 58", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 59",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  60", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 61", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 62", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 63", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 64", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 65", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 66", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 67", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 68",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  69", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 70", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 71", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 72", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 73", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 74", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 75", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 76", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 77",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  78", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 79", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 80", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 81", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 82", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 83", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 84", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 85", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 86",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  87", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 88", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 89", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 90", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 91", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, );
//  } TEST_CASE("Slave Unit Test Case 92", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 93", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 94", "[SPI]")
//  { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 95",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  96", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 97", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 98", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 99", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 100", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 101", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 102", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 103",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  104",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  105", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 106", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 107", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 108", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 109", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 110", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 111", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 112",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  113",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  114", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 115", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 116", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 117", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 118", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 119", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(,
//  ); } TEST_CASE("Slave Unit Test Case 120", "[SPI]") {
//  TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case 121",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  122",
//  "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test Case
//  123", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit Test
//  Case 124", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave Unit
//  Test Case 125", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); } TEST_CASE("Slave
//  Unit Test Case 126", "[SPI]") { TEST_ASSERT_EQUAL_UINT8(, ); }
//  TEST_CASE("Slave Unit Test Case 127", "[SPI]") {
//    TEST_ASSERT_EQUAL_UINT8(, );
//  } //  TEST_CASE("SPI_Stub Begin Get Info ", "[SPI]") { err =
//  SPI_Stub_Begin_Get_Info(
//   &SPI_Handle); //Get INFO Register from SPI_Stub Slave
//   TEST_ASSERT_EQUAL_UINT8(ESP_OK, err); } TEST_CASE("SPI Add Device ",
//   "[SPI]") { SPI_Stub_handle_t SPI_Handle; // Arrange SPI_config cfg =
//   {.SCK = SCK_PIN, .MOSI = MOSI_PIN, .MISO = MISO_PIN, .CS = CS_PIN, .host =
//   SPI_MODE_USER}; esp_err_t err = SPI_add_device(&SPI_Handle, &cfg); //
//   Assert TEST_ASSERT_EQUAL_UINT8(ESP_OK, err); }
