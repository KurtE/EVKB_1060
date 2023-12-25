#define TeensySketch 1
#define TEENSY_SDRAM 1
#ifndef TeensySketch
/*
   Copyright 2017-2020 NXP
   All rights reserved.

   SPDX-License-Identifier: BSD-3-Clause
*/
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_semc.h"
//#include "stdio.h"
#include "fsl_clock.h"

#include "system_MIMXRT1062.h"
/*******************************************************************************
   Definitions
 ******************************************************************************/
#define EXAMPLE_SEMC               SEMC
#define EXAMPLE_SEMC_START_ADDRESS (0x80000000U)
#define EXAMPLE_SEMC_CLK_FREQ      CLOCK_GetFreq(kCLOCK_SemcClk)

#define SEMC_EXAMPLE_DATALEN    12*1024 // (0x1000U)
// #define SEMC_EXAMPLE_WRITETIMES (1000U)

#define EXAMPLE_LED_GPIO     BOARD_USER_LED_GPIO
#define EXAMPLE_LED_GPIO_PIN BOARD_USER_LED_GPIO_PIN
/*
  GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 1U);
*/
/*******************************************************************************
   Prototypes
 ******************************************************************************/
extern status_t BOARD_InitSEMC(void);
static void SEMC_SDRAMReadWrite32BitAll(void);
static void SEMC_SDRAMReadWrite32Bit(void);
static void SEMC_SDRAMReadWrite16Bit(void);
static void SEMC_SDRAMReadWrite8Bit(void);

void main2();

/*******************************************************************************
   Variables
 ******************************************************************************/

uint32_t sdram_writeBuffer[SEMC_EXAMPLE_DATALEN];
uint32_t sdram_readBuffer[SEMC_EXAMPLE_DATALEN];


/*******************************************************************************
   Code
 ******************************************************************************/
status_t BOARD_InitSEMC(void)
{
  semc_config_t config;
  semc_sdram_config_t sdramconfig;
  uint32_t clockFrq = EXAMPLE_SEMC_CLK_FREQ;

  /* Initializes the MAC configure structure to zero. */
  memset(&config, 0, sizeof(semc_config_t));
  memset(&sdramconfig, 0, sizeof(semc_sdram_config_t));

  /* Initialize SEMC. */
  SEMC_GetDefaultConfig(&config);
  config.dqsMode = kSEMC_Loopbackdqspad; /* For more accurate timing. */
  SEMC_Init(SEMC, &config);

  /* Configure SDRAM. */
  sdramconfig.csxPinMux           = kSEMC_MUXCSX0;
  sdramconfig.address             = 0x80000000;
  sdramconfig.memsize_kbytes      = 32 * 1024; /* 32MB = 32*1024*1KBytes*/
  sdramconfig.portSize            = kSEMC_PortSize16Bit;
  sdramconfig.burstLen            = kSEMC_Sdram_BurstLen8;
  sdramconfig.columnAddrBitNum    = kSEMC_SdramColunm_9bit;
  sdramconfig.casLatency          = kSEMC_LatencyThree;
  sdramconfig.tPrecharge2Act_Ns   = 18; /* Trp 18ns */
  sdramconfig.tAct2ReadWrite_Ns   = 18; /* Trcd 18ns */
  sdramconfig.tRefreshRecovery_Ns = 67; /* Use the maximum of the (Trfc , Txsr). */
  sdramconfig.tWriteRecovery_Ns   = 12; /* 12ns */
  sdramconfig.tCkeOff_Ns =
    42; /* The minimum cycle of SDRAM CLK off state. CKE is off in self refresh at a minimum period tRAS.*/
  sdramconfig.tAct2Prechage_Ns       = 42; /* Tras 42ns */
  sdramconfig.tSelfRefRecovery_Ns    = 67;
  sdramconfig.tRefresh2Refresh_Ns    = 60;
  sdramconfig.tAct2Act_Ns            = 60;
  sdramconfig.tPrescalePeriod_Ns     = 160 * (1000000000 / clockFrq);
  sdramconfig.refreshPeriod_nsPerRow = 64 * 1000000 / 8192; /* 64ms/8192 */
  sdramconfig.refreshUrgThreshold    = sdramconfig.refreshPeriod_nsPerRow;
  sdramconfig.refreshBurstLen        = 1;
  return SEMC_ConfigureSDRAM(SEMC, kSEMC_SDRAM_CS0, &sdramconfig, clockFrq);
}

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
#include "fsl_cache.h"
#endif

#define ARM_DWT_CYCCNT          (*(volatile uint32_t *)0xE0001004) // Cycle count register

// micros for SDK using GPT   add gpt component to project
#include "fsl_gpt.h"

#define GPTx GPT1

void micros_init() {
  // run GPT at 1mhz
  gpt_config_t gptConfig;

  CLOCK_SetMux(kCLOCK_PerclkMux, 0);
  CLOCK_SetDiv(kCLOCK_PerclkDiv, 0);

  GPT_GetDefaultConfig(&gptConfig);

  /* Initialize GPT module */
  GPT_Init(GPTx, &gptConfig);

  /* Divide GPT clock source frequency by 150 inside GPT module */
  GPT_SetClockDivider(GPTx, CLOCK_GetFreq(kCLOCK_IpgClk) / 1000000);
  GPT_StartTimer(GPTx);
}

uint32_t micros() {
  return GPT_GetCurrentTimerCount(GPTx);
}

void delay(uint32_t ms) {
  uint32_t t = micros();
  while (micros() < t + 1000 * ms);
}

// #define Serial.
/*!
   @brief Main function
*/
int main(void)
{
  /* Hardware initialize. */
  BOARD_ConfigMPU();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_BootClockRUN();

  CLOCK_InitSysPfd(kCLOCK_Pfd2, 29);
  /* Set semc clock to 163.86 MHz */
  CLOCK_SetMux(kCLOCK_SemcMux, 1);
  CLOCK_SetDiv(kCLOCK_SemcDiv, 1);
  USER_LED_INIT(1U);
  BOARD_InitDebugConsole();
  MSDK_EnableCpuCycleCounter();

  /* Update the core clock */
  SystemCoreClockUpdate();
  micros_init();

  PRINTF("\n SEMC SDRAM Example Start! 12/24 \n");
  PRINTF(__FILE__);
  PRINTF("\n float? %f\n", 0.01234f);
  PRINTF("\t HELLO WORLD!\n");
  if (BOARD_InitSEMC() != kStatus_Success)
  {
    PRINTF("\r\n SEMC SDRAM Init Failed\n");
  }
  else
  {
    PRINTF("\r\n SEMC SDRAM Init Succeeded!\n");
    USER_LED_TOGGLE();
  }
  uint32_t aTime;
  /* 32Bit data read and write. */
  //aTime = MSDK_GetCpuCycleCount();
  aTime = micros();
  SEMC_SDRAMReadWrite32BitAll();
  aTime = micros() - aTime;
  PRINTF("\t ALL Write-Test took %d micros\n", aTime);
  USER_LED_TOGGLE();

  /* 32Bit data read and write. */
  aTime = micros();
  SEMC_SDRAMReadWrite32Bit();
  aTime = micros() - aTime;
  PRINTF("\t Write-Test took %d micros\n\n", aTime);
  USER_LED_TOGGLE();

  /* 16Bit data read and write. */
  aTime = micros();
  SEMC_SDRAMReadWrite16Bit();
  aTime = micros() - aTime;
  PRINTF("\t Write-Test took %d micros\n\n", aTime);
  USER_LED_TOGGLE();

  /* 8Bit data read and write. */
  aTime = micros();
  SEMC_SDRAMReadWrite8Bit();
  aTime = micros() - aTime;
  PRINTF("\t Write-Test took %d micros\n\n", aTime);

  main2();

  PRINTF(" SEMC SDRAM Example End.\n");
  //    GPIO_PinWrite(EXAMPLE_LED_GPIO, EXAMPLE_LED_GPIO_PIN, 1U);
  while (1)
  {
    // SysTick_DelayTicks(1000U);
    delay(1000U);
    USER_LED_TOGGLE();
  }
}

void SEMC_SDRAMReadWrite32BitAll(void) {
  uint32_t index;
  uint32_t datalen = 32 * 1024 * 1024 / sizeof(uint32_t);
  uint32_t *sdram = (uint32_t*) EXAMPLE_SEMC_START_ADDRESS; /* SDRAM start address. */
  bool result = true;

  PRINTF("\n SEMC SDRAM Memory 32 bit Write Start, Start Address 0x%x, Data Length %d !\n",
         sdram, datalen * sizeof(uint32_t));
  /* Prepare data and write to SDRAM. */
  uint32_t aTime;
  /* 32Bit data read and write. */
  aTime = micros();
  for (index = 0; index < datalen; index++) {
    sdram[index] = index;
  }
  aTime = micros() - aTime;
  PRINTF("\t ALL Write took %d micros\n", aTime);
  PRINTF("SEMC SDRAM Read 32 bit Data Start, Start Address 0x%x, Data Length %d !\n",
         sdram, datalen * sizeof(uint32_t));

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
  x // no cache here
  DCACHE_InvalidateByRange(EXAMPLE_SEMC_START_ADDRESS, 4U * SEMC_EXAMPLE_DATALEN);
#endif

  aTime = micros();
  /* Read data from the SDRAM. */
  for (index = 0; index < datalen; index++) {
    if (sdram[index] != index) {
      result = false;
      break;
    }
  }
  aTime = micros() - aTime;
  PRINTF("\t ALL Read&Test took %d micros\n", aTime);

  if (!result) {
    PRINTF("ALL SEMC SDRAM 32 bit Data Write and Read Compare Failed!\n\n");
  } else {
    PRINTF("ALL SEMC SDRAM 32 bit Data Write and Read Compare Succeed!\n");
  }
}

void SEMC_SDRAMReadWrite32Bit(void)
{
  uint32_t index;
  uint32_t datalen = SEMC_EXAMPLE_DATALEN;
  uint32_t *sdram  = (uint32_t *)EXAMPLE_SEMC_START_ADDRESS; /* SDRAM start address. */
  bool result      = true;

  PRINTF("\n SEMC SDRAM Memory 32 bit Write Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);
  /* Prepare data and write to SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_writeBuffer[index] = index;
    sdram[index]             = sdram_writeBuffer[index];
  }

  PRINTF("\n SEMC SDRAM Read 32 bit Data Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
  DCACHE_InvalidateByRange(EXAMPLE_SEMC_START_ADDRESS, 4U * SEMC_EXAMPLE_DATALEN);
#endif

  /* Read data from the SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_readBuffer[index] = sdram[index];
  }

  PRINTF("\n SEMC SDRAM 32 bit Data Write and Read Compare Start!\n");
  /* Compare the two buffers. */
  while (datalen--)
  {
    if (sdram_writeBuffer[datalen] != sdram_readBuffer[datalen])
    {
      result = false;
      break;
    }
  }

  if (!result)
  {
    PRINTF("\n SEMC SDRAM 32 bit Data Write and Read Compare Failed!\n");
  }
  else
  {
    PRINTF("\n SEMC SDRAM 32 bit Data Write and Read Compare Succeed!\n");
  }
}

static void SEMC_SDRAMReadWrite16Bit(void)
{
  uint32_t index;
  uint32_t datalen = SEMC_EXAMPLE_DATALEN;
  uint16_t *sdram  = (uint16_t *)EXAMPLE_SEMC_START_ADDRESS; /* SDRAM start address. */
  bool result      = true;

  PRINTF("\n SEMC SDRAM Memory 16 bit Write Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);

  memset(sdram_writeBuffer, 0, sizeof(sdram_writeBuffer));
  memset(sdram_readBuffer, 0, sizeof(sdram_readBuffer));

  /* Prepare data and write to SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_writeBuffer[index] = index % 0xFFFF;
    sdram[index]             = sdram_writeBuffer[index];
  }

  PRINTF("\n SEMC SDRAM Read 16 bit Data Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
  DCACHE_InvalidateByRange(EXAMPLE_SEMC_START_ADDRESS, 4U * SEMC_EXAMPLE_DATALEN);
#endif

  /* Read data from the SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_readBuffer[index] = sdram[index];
  }

  PRINTF("\n SEMC SDRAM 16 bit Data Write and Read Compare Start!\n");
  /* Compare the two buffers. */
  while (datalen--)
  {
    if (sdram_writeBuffer[datalen] != sdram_readBuffer[datalen])
    {
      result = false;
      break;
    }
  }

  if (!result)
  {
    PRINTF("\n SEMC SDRAM 16 bit Data Write and Read Compare Failed!\n");
  }
  else
  {
    PRINTF("\n SEMC SDRAM 16 bit Data Write and Read Compare Succeed!\n");
  }
}

static void SEMC_SDRAMReadWrite8Bit(void)
{
  uint32_t index;
  uint32_t datalen = SEMC_EXAMPLE_DATALEN;
  uint8_t *sdram   = (uint8_t *)EXAMPLE_SEMC_START_ADDRESS; /* SDRAM start address. */
  bool result      = true;

  PRINTF("\n SEMC SDRAM Memory 8 bit Write Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);

  memset(sdram_writeBuffer, 0, sizeof(sdram_writeBuffer));
  memset(sdram_readBuffer, 0, sizeof(sdram_readBuffer));

  /* Prepare data and write to SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_writeBuffer[index] = index % 0x100;
    sdram[index]             = sdram_writeBuffer[index];
  }

  PRINTF("\n SEMC SDRAM Read 8 bit Data Start, Start Address 0x%x, Data Length %d !\n", sdram, datalen);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
  DCACHE_InvalidateByRange(EXAMPLE_SEMC_START_ADDRESS, 4U * SEMC_EXAMPLE_DATALEN);
#endif

  /* Read data from the SDRAM. */
  for (index = 0; index < datalen; index++)
  {
    sdram_readBuffer[index] = sdram[index];
  }

  PRINTF("\n SEMC SDRAM 8 bit Data Write and Read Compare Start!\n");
  /* Compare the two buffers. */
  while (datalen--)
  {
    if (sdram_writeBuffer[datalen] != sdram_readBuffer[datalen])
    {
      result = false;
      break;
    }
  }

  if (!result)
  {
    PRINTF("\n SEMC SDRAM 8 bit Data Write and Read Compare Failed!\n");
  }
  else
  {
    PRINTF("\n SEMC SDRAM 8 bit Data Write and Read Compare Succeed!\n");
  }
}
#endif

// https://github.com/PaulStoffregen/teensy41_psram_memtest/blob/master/teensy41_psram_memtest.ino

#ifdef TeensySketch
extern "C" uint8_t external_psram_size;
#define printf Serial.printf
#else
#define printf PRINTF
uint8_t external_psram_size = 16;
#endif

bool check_fixed_pattern(uint32_t pattern);
bool check_lfsr_pattern(uint32_t seed);

uint32_t testMsec[100][4]; // Write us, Read Test us, Val Seed/Pattern, Type {1=Pat, 2=Seed, 3=Fail)
#define vRd 0
#define vWr 1
#define vPt 2
#define vTp 3
int tt = 0;

bool memory_ok = false;
uint32_t *memory_begin, *memory_end;

bool check_fixed_pattern(uint32_t pattern);
bool check_lfsr_pattern(uint32_t seed);
void SDRAMsetup();
void setup()
{
#ifdef TeensySketch
  while (!Serial) ; // wait
  pinMode(13, OUTPUT);
#endif
  uint8_t size = external_psram_size;

#ifdef TEENSY_SDRAM
  SDRAMsetup();
  size = 16;
  memory_begin = (uint32_t *)(0x80000000);
  memory_end = (uint32_t *)(0x80000000 + size * 1048576);
#else
#ifdef TeensySketch
  const float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
  const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / (float)(((CCM_CBCMR >> 29) & 7) + 1);
  printf(" CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
  memory_begin = (uint32_t *)(0x70000000);
  memory_end = (uint32_t *)(0x70000000 + size * 1048576);
#else
  memory_begin = (uint32_t *)(0x80000000);
  memory_end = (uint32_t *)(0x80000000 + size * 1048576);
#endif
#endif
  printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  printf("EXTMEM Memory Test, %d Mbyte\n", size);
  printf("EXTMEM Memory begin, %08X \n", memory_begin);
  printf("EXTMEM Memory end, %08X \n", memory_end);

  if (size == 0) return;
  uint32_t msec = 0;


  check_fixed_pattern(0x5A698421);
  check_lfsr_pattern(2976674124ul);
  check_lfsr_pattern(1438200953ul);
  check_lfsr_pattern(3413783263ul);
  check_lfsr_pattern(1900517911ul);
  check_lfsr_pattern(1227909400ul);
  check_lfsr_pattern(276562754ul);
  check_lfsr_pattern(146878114ul);
  check_lfsr_pattern(615545407ul);
  check_lfsr_pattern(110497896ul);
  check_lfsr_pattern(74539250ul);
  check_lfsr_pattern(4197336575ul);
  check_lfsr_pattern(2280382233ul);
  check_lfsr_pattern(542894183ul);
  check_lfsr_pattern(3978544245ul);
  check_lfsr_pattern(2315909796ul);
  check_lfsr_pattern(3736286001ul);
  check_lfsr_pattern(2876690683ul);
  check_lfsr_pattern(215559886ul);
  check_lfsr_pattern(539179291ul);
  check_lfsr_pattern(537678650ul);
  check_lfsr_pattern(4001405270ul);
  check_lfsr_pattern(2169216599ul);
  check_lfsr_pattern(4036891097ul);
  check_lfsr_pattern(1535452389ul);
  check_lfsr_pattern(2959727213ul);
  check_lfsr_pattern(4219363395ul);
  check_lfsr_pattern(1036929753ul);
  check_lfsr_pattern(2125248865ul);
  check_lfsr_pattern(3177905864ul);
  check_lfsr_pattern(2399307098ul);
  check_lfsr_pattern(3847634607ul);
  check_lfsr_pattern(27467969ul);
  check_lfsr_pattern(520563506ul);
  check_lfsr_pattern(381313790ul);
  check_lfsr_pattern(4174769276ul);
  check_lfsr_pattern(3932189449ul);
  check_lfsr_pattern(4079717394ul);
  check_lfsr_pattern(868357076ul);
  check_lfsr_pattern(2474062993ul);
  check_lfsr_pattern(1502682190ul);
  check_lfsr_pattern(2471230478ul);
  check_lfsr_pattern(85016565ul);
  check_lfsr_pattern(1427530695ul);
  check_lfsr_pattern(1100533073ul);
  check_fixed_pattern(0x55555555);
  check_fixed_pattern(0x33333333);
  check_fixed_pattern(0x0F0F0F0F);
  check_fixed_pattern(0x00FF00FF);
  check_fixed_pattern(0x0000FFFF);
  check_fixed_pattern(0xAAAAAAAA);
  check_fixed_pattern(0xCCCCCCCC);
  check_fixed_pattern(0xF0F0F0F0);
  check_fixed_pattern(0xFF00FF00);
  check_fixed_pattern(0xFFFF0000);
  check_fixed_pattern(0xFFFFFFFF);
  check_fixed_pattern(0x00000000);
  msec = 0;
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 1 == testMsec[ii][vTp] ) {
      printf("test Pattern %08X Write %u us Read/Test %u us \n", testMsec[ii][vPt], testMsec[ii][vRd], testMsec[ii][vWr] );
      msec += testMsec[ii][vRd] + testMsec[ii][vWr];
    }
  }
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 2 == testMsec[ii][vTp] ) {
      printf("test RndSeed %u Write %u us Read/Test %u us \n", testMsec[ii][vPt], testMsec[ii][vRd], testMsec[ii][vWr] );
      msec += testMsec[ii][vRd] + testMsec[ii][vWr];
    }
  }
  memory_ok = true;
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 3 == testMsec[ii][vTp] ) {
      printf("test FAIL : Seed %08X == Pattern %u Write %u us Read/Test %u us \n", testMsec[ii][vRd], testMsec[ii][vWr] );
      memory_ok = false;
    }
  }
  printf(" test ran for %.2f seconds\n", (float)msec / 1000000.0f);
  if (memory_ok)
    printf("All memory tests passed :-)\n");
}

bool fail_message(volatile uint32_t *location, uint32_t actual, uint32_t expected)
{
  printf(" Error at %08X, read %08X but expected %08X\n",
         (uint32_t)location, actual, expected);
  return false;
}

// fill the entire RAM with a fixed pattern, then check it
bool check_fixed_pattern(uint32_t pattern)
{
  volatile uint32_t *p;

  testMsec[tt][vTp] = 3;
  testMsec[tt][vPt] = pattern;
  printf("testing with fixed pattern %08X\n", pattern);
  testMsec[tt][vRd] = micros();
  for (p = memory_begin; p < memory_end; p++) {
    *p = pattern;
  }
  testMsec[tt][vRd] = micros() - testMsec[tt][vRd];
#ifdef TeensySketch
  arm_dcache_flush_delete((void *)memory_begin,
                          (uint32_t)memory_end - (uint32_t)memory_begin);
#endif
  testMsec[tt][vWr] = micros();
  for (p = memory_begin; p < memory_end; p++) {
    uint32_t actual = *p;
    if (actual != pattern) {
        tt++;
        return fail_message(p, actual, pattern);
    }
  }
  testMsec[tt][vWr] = micros() - testMsec[tt][vWr];
  testMsec[tt][vTp] = 1;
  tt++;
  return true;
}

// fill the entire RAM with a pseudo-random sequence, then check it
bool check_lfsr_pattern(uint32_t seed)
{
  volatile uint32_t *p;
  uint32_t reg;

  testMsec[tt][vTp] = 3;
  testMsec[tt][vPt] = seed;
  printf("testing with pseudo-random sequence, seed=%u\n", seed);
  reg = seed;
  testMsec[tt][vRd] = micros();
  for (p = memory_begin; p < memory_end; p++) {
    *p = reg;
    for (int i = 0; i < 3; i++) {
      if (reg & 1) {
        reg >>= 1;
        reg ^= 0x7A5BC2E3;
      } else {
        reg >>= 1;
      }
    }
  }
  testMsec[tt][vRd] = micros() - testMsec[tt][vRd];
#ifdef TeensySketch
  arm_dcache_flush_delete((void *)memory_begin,
                          (uint32_t)memory_end - (uint32_t)memory_begin);
#endif
  reg = seed;
  testMsec[tt][vWr] = micros();
  for (p = memory_begin; p < memory_end; p++) {
    uint32_t actual = *p;
    if (actual != reg) {
        tt++;
        return fail_message(p, actual, reg);
    }
    //printf(" reg=%08X\n", reg);
    for (int i = 0; i < 3; i++) {
      if (reg & 1) {
        reg >>= 1;
        reg ^= 0x7A5BC2E3;
      } else {
        reg >>= 1;
      }
    }
  }
  testMsec[tt][vWr] = micros() - testMsec[tt][vWr];
  testMsec[tt][vTp] = 2;
  tt++;
  return true;
}

#ifdef TeensySketch
void loop()
{
  digitalWrite(13, HIGH);
  delay(100);
  if (!memory_ok) digitalWrite(13, LOW); // rapid blink if any test fails
  delay(100);
}
#else
void main2()
{
  setup();
}

#endif

#ifdef TeensySketch
// post #57 CODE

#define SDRAM_Read        0x08          /*  SDRAM memory read. */
#define SDRAM_Write       0x09         /*  SDRAM memory write. */
#define SDRAM_Modeset     0x0a       /*  SDRAM MODE SET. */
#define SDRAM_Active      0x0b        /*  SDRAM active. */
#define SDRAM_AutoRefresh 0x0c   /*  SDRAM auto-refresh. */
#define SDRAM_SelfRefresh 0x0d   /*  SDRAM self-refresh. */
#define SDRAM_Precharge   0x0e     /*  SDRAM precharge. */
#define SDRAM_Prechargeall 0x0f  /*  SDRAM precharge all. */
#define SEMC_IPCMD_KEY(n)  (((uint32_t)(((uint32_t)(n)) << 16)) & 0xFFFF0000U)

unsigned int ns_to_clocks(float ns, float freq)
{
  float clocks = ceilf(ns * 1.0e-9f * freq);
  if (clocks < 1.0f) return 1;
  return (unsigned int)clocks;
}
bool SendIPCommand(uint32_t address, uint16_t command, uint32_t write, uint32_t *read)
{
  uint32_t cmdMode;
  bool readCmd = 0;
  bool writeCmd = 0;
  /* Clear status bit */
  SEMC_INTR |= 0x1U;
  /* Set address. */
  SEMC_IPCR0 = address;
  /* Check command mode. */
  cmdMode = command & 0xFU;
  readCmd = (cmdMode == SDRAM_Read);
  writeCmd = (cmdMode == SDRAM_Write) || (cmdMode == SDRAM_Modeset);
  if (writeCmd)
  {
    /* Set data. */
    SEMC_IPTXDAT = write;
  }
  /* Set command code. */
  SEMC_IPCMD = command | SEMC_IPCMD_KEY(0xA55A);
  /* Wait for command done. */
  bool result = IPCommandComplete();
  if (result != true)
  {
    return result;
  }
  if (readCmd)
  {
    /* Get the read data */
    *read = SEMC_IPRXDAT;
  }
  return true;
}
bool IPCommandComplete()
{
  /* Poll status bit till command is done*/
  while (!(SEMC_INTR & 0x1))
  {
  };
  /* Clear status bit */
  SEMC_INTR |= 0x1;
  /* Check error status */
  if (SEMC_INTR & 0x2)
  {
    SEMC_INTR |= 0x2;
    return false;
  }
  return true;
}
bool sdram_init()
{
  // use PLL3 PFD1 664.62 divided by 4 or 5, for 166 or 133 MHz
  const unsigned int clockdiv = 5;
  CCM_CBCDR = (CCM_CBCDR & ~(CCM_CBCDR_SEMC_PODF(7))) |
              CCM_CBCDR_SEMC_CLK_SEL | CCM_CBCDR_SEMC_ALT_CLK_SEL |
              CCM_CBCDR_SEMC_PODF(clockdiv - 1);
  delayMicroseconds(1);
  const float freq = 664.62e6 / (float)clockdiv;
  CCM_CCGR3 |= CCM_CCGR3_SEMC(CCM_CCGR_ON);
  // software reset
  SEMC_BR0 = 0;
  SEMC_BR1 = 0;
  SEMC_BR2 = 0;
  SEMC_BR3 = 0;
  SEMC_BR4 = 0;
  SEMC_BR5 = 0;
  SEMC_BR6 = 0;
  SEMC_BR7 = 0;
  SEMC_BR8 = 0;
  SEMC_MCR = SEMC_MCR_SWRST;
  elapsedMicros timeout = 0;
  while (SEMC_MCR & SEMC_MCR_SWRST) {
    if (timeout > 1500) return false;
  }
  configure_sdram_pins();
  // TODO: configure pad registers for 200 MHz, fast drive, hyst?
  // TODO: IOMUXC_SEMC_I_IPP_IND_DQS4_SELECT_INPUT not needed?
  // turn on SEMC hardware, same settings as NXP's SDK
  SEMC_MCR |= SEMC_MCR_MDIS | SEMC_MCR_CTO(0xFF) | SEMC_MCR_BTO(0x1F);
  // TODO: reference manual page 1364 says "Recommend to set BMCR0 with 0x0 for
  // applications that require restrict sequence of transactions", same on BMCR1
  SEMC_BMCR0 = SEMC_BMCR0_WQOS(5) | SEMC_BMCR0_WAGE(8) |
               SEMC_BMCR0_WSH(0x40) | SEMC_BMCR0_WRWS(0x10);
  SEMC_BMCR1 = SEMC_BMCR1_WQOS(5) | SEMC_BMCR1_WAGE(8) |
               SEMC_BMCR1_WPH(0x60) | SEMC_BMCR1_WRWS(0x24) | SEMC_BMCR1_WBR(0x40);
  SEMC_MCR &= ~SEMC_MCR_MDIS;
  // configure SDRAM parameters
  SEMC_BR0 = 0x80000000 | SEMC_BR_MS(13 /*13 = 32 Mbyte*/) | SEMC_BR_VLD;
  SEMC_SDRAMCR0 = SEMC_SDRAMCR0_CL(3) |
                  SEMC_SDRAMCR0_COL(3) |  // 3 = 9 bit column
                  SEMC_SDRAMCR0_BL(3) |   // 3 = 8 word burst length
                  SEMC_SDRAMCR0_PS;       // use 16 bit data
  SEMC_SDRAMCR1 =
    SEMC_SDRAMCR1_ACT2PRE(ns_to_clocks(42, freq)) | // tRAS: ACTIVE to PRECHARGE
    SEMC_SDRAMCR1_CKEOFF(ns_to_clocks(42, freq)) |  // self refresh
    SEMC_SDRAMCR1_WRC(ns_to_clocks(12, freq)) |     // tWR: WRITE recovery
    SEMC_SDRAMCR1_RFRC(ns_to_clocks(67, freq)) |    // tRFC or tXSR: REFRESH recovery
    SEMC_SDRAMCR1_ACT2RW(ns_to_clocks(18, freq)) |  // tRCD: ACTIVE to READ/WRITE
    SEMC_SDRAMCR1_PRE2ACT(ns_to_clocks(18, freq));  // tRP: PRECHARGE to ACTIVE/REFRESH
  SEMC_SDRAMCR2 = 0; // TODO... page 1425
  //#define SEMC_SDRAMCR2_ITO(n)            ((uint32_t)(n & 0xFF)<<24)
  //#define SEMC_SDRAMCR2_ACT2ACT(n)        ((uint32_t)(n & 0xFF)<<16)
  //#define SEMC_SDRAMCR2_REF2REF(n)        ((uint32_t)(n & 0xFF)<<8)
  //#define SEMC_SDRAMCR2_SRRC(n)           ((uint32_t)(n & 0xFF)<<0)
  SEMC_SDRAMCR3 = 0; // TODO...page 1426
  //#define SEMC_SDRAMCR3_UT(n)             ((uint32_t)(n & 0xFF)<<24)
  //#define SEMC_SDRAMCR3_RT(n)             ((uint32_t)(n & 0xFF)<<16)
  //#define SEMC_SDRAMCR3_PRESCALE(n)       ((uint32_t)(n & 0xFF)<<8)
  //#define SEMC_SDRAMCR3_REBL(n)           ((uint32_t)(n & 0x07)<<1)
  //#define SEMC_SDRAMCR3_REN               ((uint32_t)(1<<0))
  SEMC_SDRAMCR2 =
    SEMC_SDRAMCR2_SRRC((ns_to_clocks(67, freq) - 1)) |
    SEMC_SDRAMCR2_REF2REF(ns_to_clocks(60, freq)) |           /* No Minus one to keep with RM */
    SEMC_SDRAMCR2_ACT2ACT(ns_to_clocks(60, freq)) | /* No Minus one to keep with RM */
    SEMC_SDRAMCR2_ITO(0);
  //sdk has: idle = config->tIdleTimeout_Ns / config->tPrescalePeriod_Ns
  //RM states
  /*   SEMC closes all opened pages if the SDRAM idle time lasts more than idle timeout period. SDRAM is
    considered idle when there is no AXI Bus transfer and no SDRAM command pending.
    00000000b - IDLE timeout period is 256*Prescale period.
    00000001-11111111b - IDLE timeout period is ITO*Prescale period.
  */
  uint32_t prescaleperiod = 160 * (1000000000 / freq);
  uint16_t prescale = prescaleperiod / 16 / (1000000000 / freq);
  if (prescale > 256)
  {
    Serial.println("Invalid Timer Setting");
    while (1) {}
  }
  uint16_t refresh = (64 * 1000000 / 8192) / prescaleperiod;
  uint16_t urgentRef = refresh;
  //uint16_t idle = 0 / prescaleperiod;

  SEMC_SDRAMCR3 = SEMC_SDRAMCR3_REBL((1 - 1)) |
                  /* N * 16 * 1s / clkSrc_Hz = config->tPrescalePeriod_Ns */
                  SEMC_SDRAMCR3_PRESCALE(prescale) | SEMC_SDRAMCR3_RT(refresh) | SEMC_SDRAMCR3_UT(urgentRef);
  SEMC_IPCR1 = 2; // IP commadns, data is 16 bits wide
  SEMC_IPCR2 = 0;
  // TODO: send IP commands to initialize SDRAM chip
  //  precharge all
  //  auto refresh (NXP SDK sends this twice, why?)
  //  mode set
  bool result_cmd = SendIPCommand(0x80000000, SDRAM_Prechargeall, 0, NULL);
  if (result_cmd != true)
  {
    return result_cmd;
  }
  result_cmd = SendIPCommand(0x80000000, SDRAM_AutoRefresh, 0, NULL);
  if (result_cmd != true)
  {
    return result_cmd;
  }
  result_cmd = SendIPCommand(0x80000000, SDRAM_AutoRefresh, 0, NULL);
  if (result_cmd != true)
  {
    return result_cmd;
  }
  /* Mode setting value. */
  uint16_t mode = (uint16_t)3 | (uint16_t)(3 << 4);
  result_cmd = SendIPCommand(0x80000000, SDRAM_Modeset, mode, NULL);
  if (result_cmd != true)
  {
    return result_cmd;
  }
  // enable refresh
  SEMC_SDRAMCR3 |= SEMC_SDRAMCR3_REN;
  // TODO: configure MPU to allow read/write, disallow exec, use cache
  if (result_cmd == false) return false;
  return true; // hopefully SDRAM now working at 80000000 to 81FFFFFF
}

void SDRAMsetup() {
  Serial.begin(9600);
  Serial.println("\nSDRAM Init Experiment");
  if (sdram_init()) {
    Serial.println(":-)");
  } else {
    Serial.println("nope :(");
  }
}
void configure_sdram_pins()
{
  // initialize pins
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_00 = 0x0110F9;   //SEMC_D0
  /* Slew Rate Field: Fast Slew Rate
    Drive Strength Field: R0/7
    Speed Field: max(200MHz)
    Open Drain Enable Field: Open Drain Disabled
    Pull / Keep Enable Field: Pull/Keeper Enabled
    Pull / Keep Select Field: Keeper
    Pull Up / Down Config. Field: 100K Ohm Pull Down
    Hyst. Enable Field: Hysteresis Enabled */
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_01 = 0x0110F9;   //SEMC_D0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_02 = 0x0110F9;   //SEMC_D1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_03 = 0x0110F9;   //SEMC_D2
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_04 = 0x0110F9;   //SEMC_D3
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_05 = 0x0110F9;   //SEMC_D4
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_06 = 0x0110F9;   //SEMC_D5
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_07 = 0x0110F9;   //SEMC_D6
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_08 = 0x0110F9;   //SEMC_DMO
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_08 = 0x0110F9;   //SEMC_A0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_10 = 0x0110F9;   //SEMC_A1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_11 = 0x0110F9;   //SEMC_A2
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_12 = 0x0110F9;   //SEMC_A3
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_13 = 0x0110F9;   //SEMC_A4
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_14 = 0x0110F9;   //SEMC_A5
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_15 = 0x0110F9;   //SEMC_A6
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_16 = 0x0110F9;   //SEMC_A7
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_17 = 0x0110F9;   //SEMC_A8

  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_18 = 0x0110F9;   //SEMC_A9
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_19 = 0x0110F9;   //SEMC_A11
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_20 = 0x0110F9;   //SEMC_A12
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_21 = 0x0110F9;   //SEMC_BA0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_22 = 0x0110F9;   //SEMC_BA1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_23 = 0x0110F9;   //SEMC_A10
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_24 = 0x0110F9;   //SEMC_CAS
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_25 = 0x0110F9;   //SEMC_RAS
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_26 = 0x0110F9;   //SEMC_CLK
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_27 = 0x0110F9;   //SEMC_CKE
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_28 = 0x0110F9;   //SEMC_WE
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_29 = 0x0110F9;   //SEMC_CS0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_30 = 0x0110F9;   //SEMC_D8
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_31 = 0x0110F9;   //SEMC_D9
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_32 = 0x0110F9;   //SEMC_D10
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_33 = 0x0110F9;   //SEMC_D11
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_34 = 0x0110F9;   //SEMC_D12

  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_35 = 0x0110F9;   //SEMC_D13
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_36 = 0x0110F9;   //SEMC_D14
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_37 = 0x0110F9;   //SEMC_D15
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_38 = 0x0110F9;   //SEMC_DM1
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_39 = 0x0110F9;   //SEMC_DQS
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_40 = 0x0110F9;   //SEMC_MD0
  IOMUXC_SW_PAD_CTL_PAD_GPIO_EMC_41 = 0x0110F9;   //SEMC_MD1
  /* Default to Pauls version
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17 = 0x00;

    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34 = 0x00;

    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_40 = 0x00;
    IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_41 = 0x00;
  */
  // configure pins
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_00 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_01 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_02 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_03 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_04 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_05 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_07 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_09 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_10 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_11 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_12 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_13 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_14 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_15 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_16 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_17 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_18 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_19 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_20 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_21 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_22 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_23 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_24 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_25 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_26 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_27 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_28 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_29 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_30 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_31 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_32 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_33 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_34 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_35 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_36 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_37 = 0x10;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_38 = 0;
  IOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_39 = 0x10;
}

#endif
