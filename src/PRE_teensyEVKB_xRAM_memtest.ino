#define TeensySketch 1
#ifndef TeensySketch
/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
 * Definitions
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
 * Prototypes
 ******************************************************************************/
extern status_t BOARD_InitSEMC(void);
static void SEMC_SDRAMReadWrite32BitAll(void);
static void SEMC_SDRAMReadWrite32Bit(void);
static void SEMC_SDRAMReadWrite16Bit(void);
static void SEMC_SDRAMReadWrite8Bit(void);

void main2();

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t sdram_writeBuffer[SEMC_EXAMPLE_DATALEN];
uint32_t sdram_readBuffer[SEMC_EXAMPLE_DATALEN];


/*******************************************************************************
 * Code
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
    while(micros() < t + 1000*ms);
}

// #define Serial.
/*!
 * @brief Main function
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
    aTime = micros()-aTime;
    PRINTF("\t ALL Write-Test took %d micros\n", aTime);
    USER_LED_TOGGLE();

    /* 32Bit data read and write. */
    aTime = micros();
    SEMC_SDRAMReadWrite32Bit();
    aTime = micros()-aTime;
    PRINTF("\t Write-Test took %d micros\n\n", aTime);
    USER_LED_TOGGLE();

    /* 16Bit data read and write. */
    aTime = micros();
    SEMC_SDRAMReadWrite16Bit();
    aTime = micros()-aTime;
    PRINTF("\t Write-Test took %d micros\n\n", aTime);
    USER_LED_TOGGLE();

    /* 8Bit data read and write. */
    aTime = micros();
    SEMC_SDRAMReadWrite8Bit();
    aTime = micros()-aTime;
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
    aTime = micros()-aTime;
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
uint8_t external_psram_size =16;
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

void setup()
{
#ifdef TeensySketch
  while (!Serial) ; // wait
  pinMode(13, OUTPUT);
#endif
  uint8_t size = external_psram_size;

  printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  printf("EXTMEM Memory Test, %d Mbyte\n", size);

  if (size == 0) return;
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
  uint32_t msec = 0;


  if (!check_fixed_pattern(0x5A698421)) return;
  if (!check_lfsr_pattern(2976674124ul)) return;
  if (!check_lfsr_pattern(1438200953ul)) return;
  if (!check_lfsr_pattern(3413783263ul)) return;
  if (!check_lfsr_pattern(1900517911ul)) return;
  if (!check_lfsr_pattern(1227909400ul)) return;
  if (!check_lfsr_pattern(276562754ul)) return;
  if (!check_lfsr_pattern(146878114ul)) return;
  if (!check_lfsr_pattern(615545407ul)) return;
  if (!check_lfsr_pattern(110497896ul)) return;
  if (!check_lfsr_pattern(74539250ul)) return;
  if (!check_lfsr_pattern(4197336575ul)) return;
  if (!check_lfsr_pattern(2280382233ul)) return;
  if (!check_lfsr_pattern(542894183ul)) return;
  if (!check_lfsr_pattern(3978544245ul)) return;
  if (!check_lfsr_pattern(2315909796ul)) return;
  if (!check_lfsr_pattern(3736286001ul)) return;
  if (!check_lfsr_pattern(2876690683ul)) return;
  if (!check_lfsr_pattern(215559886ul)) return;
  if (!check_lfsr_pattern(539179291ul)) return;
  if (!check_lfsr_pattern(537678650ul)) return;
  if (!check_lfsr_pattern(4001405270ul)) return;
  if (!check_lfsr_pattern(2169216599ul)) return;
  if (!check_lfsr_pattern(4036891097ul)) return;
  if (!check_lfsr_pattern(1535452389ul)) return;
  if (!check_lfsr_pattern(2959727213ul)) return;
  if (!check_lfsr_pattern(4219363395ul)) return;
  if (!check_lfsr_pattern(1036929753ul)) return;
  if (!check_lfsr_pattern(2125248865ul)) return;
  if (!check_lfsr_pattern(3177905864ul)) return;
  if (!check_lfsr_pattern(2399307098ul)) return;
  if (!check_lfsr_pattern(3847634607ul)) return;
  if (!check_lfsr_pattern(27467969ul)) return;
  if (!check_lfsr_pattern(520563506ul)) return;
  if (!check_lfsr_pattern(381313790ul)) return;
  if (!check_lfsr_pattern(4174769276ul)) return;
  if (!check_lfsr_pattern(3932189449ul)) return;
  if (!check_lfsr_pattern(4079717394ul)) return;
  if (!check_lfsr_pattern(868357076ul)) return;
  if (!check_lfsr_pattern(2474062993ul)) return;
  if (!check_lfsr_pattern(1502682190ul)) return;
  if (!check_lfsr_pattern(2471230478ul)) return;
  if (!check_lfsr_pattern(85016565ul)) return;
  if (!check_lfsr_pattern(1427530695ul)) return;
  if (!check_lfsr_pattern(1100533073ul)) return;
  if (!check_fixed_pattern(0x55555555)) return;
  if (!check_fixed_pattern(0x33333333)) return;
  if (!check_fixed_pattern(0x0F0F0F0F)) return;
  if (!check_fixed_pattern(0x00FF00FF)) return;
  if (!check_fixed_pattern(0x0000FFFF)) return;
  if (!check_fixed_pattern(0xAAAAAAAA)) return;
  if (!check_fixed_pattern(0xCCCCCCCC)) return;
  if (!check_fixed_pattern(0xF0F0F0F0)) return;
  if (!check_fixed_pattern(0xFF00FF00)) return;
  if (!check_fixed_pattern(0xFFFF0000)) return;
  if (!check_fixed_pattern(0xFFFFFFFF)) return;
  if (!check_fixed_pattern(0x00000000)) return;
  msec=0;
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
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 3 == testMsec[ii][vTp] )
      printf("test FAIL : Seed %08X == Pattern %u Write %u us Read/Test %u us \n", testMsec[ii][vRd], testMsec[ii][vWr] );
  }
  printf(" test ran for %.2f seconds\n", (float)msec / 1000000.0f);
  printf("All memory tests passed :-)\n");
  memory_ok = true;
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
    if (actual != pattern) return fail_message(p, actual, pattern);
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
    if (actual != reg) return fail_message(p, actual, reg);
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
