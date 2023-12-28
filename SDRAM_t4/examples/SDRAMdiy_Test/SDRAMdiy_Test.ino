#include "SDRAM_t4.h"

SDRAM_t4 sdram;

#define TEENSY_SDRAM 1 // '1' SDRAM ELSE '0' T_4.1 PSRAM TEST
int readRepeat = 1; // Writes once to Test memory, will repeat reads and test compare 'readRepeat' times
// https://github.com/PaulStoffregen/teensy41_psram_memtest/blob/master/teensy41_psram_memtest.ino

extern "C" uint8_t external_psram_size;

bool check_fixed_pattern(uint32_t pattern);
bool check_lfsr_pattern(uint32_t seed);

uint32_t testMsec[100][4]; // Write us, Read Test us, Val Seed/Pattern, Type {1=Pat, 2=Seed, 3=Fail)
#define vRd 0 // read time
#define vWr 1 // write time
#define vPt 2 // Pattern/Seed
#define vTp 3 // 0=unused, 1=success fixed, 2=succ PRandom, 3=fail fixed, 4=fail=PRandom, 5=succ PRand HalfCopy
int tt = 0;

bool memory_ok = false;
uint32_t *memory_begin, *memory_mid, *memory_end;

bool check_fixed_pattern(uint32_t pattern);
bool check_lfsr_pattern(uint32_t seed);
void SDRAMsetup();
uint8_t size = external_psram_size;

uint32_t cntLoopTests=0;
void loop()
{
  digitalWrite(13, HIGH);
  delay(100);
  if (!memory_ok)
    digitalWrite(13, LOW); // rapid blink if any test fails
  else {
    doTests();
    cntLoopTests+=1;
    Serial.printf("\t\tLoop doTest() count=%u\n\n", cntLoopTests);
  }
  delay(100);
}

void setup()
{
  while (!Serial) ; // wait
  pinMode(13, OUTPUT);
  if ( CrashReport ) Serial.print( CrashReport );

#if TEENSY_SDRAM
  if (sdram.init()) {
    Serial.print( "\tSUCCESS sdram.init()\n");
  }
  const float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
  const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / (float)(((CCM_CBCMR >> 29) & 7) + 1);
  Serial.printf(" CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);

  size = 32;
  memory_begin = (uint32_t *)(0x80000000);
  memory_mid = (uint32_t *)(0x80000000 + size * 524288);
  memory_end = (uint32_t *)(0x80000000 + size * 1048576);
#else // T_4.1 PSRAM
  const float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
  const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / (float)(((CCM_CBCMR >> 29) & 7) + 1);
  Serial.printf(" CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
  memory_begin = (uint32_t *)(0x70000000);
  memory_mid = (uint32_t *)(0x70000000 + size * 524288);
  memory_end = (uint32_t *)(0x70000000 + size * 1048576);
#endif
  Serial.printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  Serial.printf("EXTMEM Memory Test, %d Mbyte\t", size);
  Serial.printf("begin, %08X \t", memory_begin);
  Serial.printf("middle, %08X \t", memory_mid);
  Serial.printf("end, %08X \n", memory_end);
  doTests();
  readRepeat = 5;
}

void doTests() {
  if (size == 0) return;
  uint32_t msec = 0;
  tt=0;

  check_fixed_pattern(0x5A698421);
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
  msec = 0;
  int tstMBp=0;
  int rdMSp=0;
  int wrMSp=0;
  int tstMBr=0;
  int rdMSr=0;
  int wrMSr=0;
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 1 == testMsec[ii][vTp] ) {
      Serial.printf("test Pattern %08X Write %u us Read/Test %u us \n", testMsec[ii][vPt], testMsec[ii][vWr], testMsec[ii][vRd] );
      msec += testMsec[ii][vRd] + testMsec[ii][vWr];
      tstMBp +=2;
      rdMSp += testMsec[ii][vRd];
      wrMSp += testMsec[ii][vWr];
    }
  }
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 2 == testMsec[ii][vTp] ) {
      Serial.printf("test RndSeed %u Write %u us Read/Test %u us \n", testMsec[ii][vPt], testMsec[ii][vWr], testMsec[ii][vRd] );
      msec += testMsec[ii][vRd] + testMsec[ii][vWr];
      tstMBr +=2;
      rdMSr += testMsec[ii][vRd];
      wrMSr += testMsec[ii][vWr];
    }
  }
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 5 == testMsec[ii][vTp] ) {
      Serial.printf("test RndSeed HalfCopy %u Write %u us Read/Test %u us \n", testMsec[ii][vPt], testMsec[ii][vWr], testMsec[ii][vRd] );
      msec += testMsec[ii][vRd] + testMsec[ii][vWr];
      tstMBr +=2;
      rdMSr += testMsec[ii][vRd];
      wrMSr += testMsec[ii][vWr];
    }
  }
  memory_ok = true;
  for ( int ii = 0; ii < 100; ii++ ) {
    if ( 3 == testMsec[ii][vTp] ) {
      Serial.printf("\t\ttest FAIL : Fixed Pattern %u  \n", testMsec[ii][vPt] );
      memory_ok = false;
    }
    if ( 4 == testMsec[ii][vTp] ) {
      Serial.printf("\t\ttest FAIL : Peusdo Random Seed %08X  \n", testMsec[ii][vPt] );
      memory_ok = false;
    }
  }
  Serial.printf(" test ran for %.2f seconds\n", (float)msec / 1000000.0f);
  Serial.printf("Fixed Pattern Write ran for %.2f and Read/Test %.2f secs\n", (float)wrMSp / 1000000.0f , (float)rdMSp / 1000000.0f);
  Serial.printf("Fixed Pattern Test %.2f MB per sec\n", (float) (size*tstMBp) / ((float)(wrMSp + rdMSp) / 1000000.0f));
  Serial.printf("Fixed Pattern Test WRITES %.2f MB per sec\n", (float) (size*tstMBp/2) / ((float)(wrMSp) / 1000000.0f));
  Serial.printf("Fixed Pattern Test & READ %.2f MB per sec\n", (float) (size*tstMBp/2) / ((float)(rdMSp) / 1000000.0f));
  Serial.printf("PsuedoRnd Patt Write ran for %.2f and Read/Test %.2f secs\n", (float)wrMSr / 1000000.0f , (float)rdMSr / 1000000.0f);
  Serial.printf("PsuedoRnd Patt Test %.2f MB per sec\n", (float) (size*tstMBr) / ((float)(wrMSr + rdMSr) / 1000000.0f));
  if (memory_ok)
    Serial.printf("All memory tests passed :-)\n");
}

bool fail_message(volatile uint32_t *location, uint32_t actual, uint32_t expected)
{
  Serial.printf(" Error at %08X, read %08X but expected %08X\n",
                (uint32_t)location, actual, expected);
  return false;
}

// fill the entire RAM with a fixed pattern, then check it
bool check_fixed_pattern(uint32_t pattern)
{
  volatile uint32_t *p;

  testMsec[tt][vTp] = 3;
  testMsec[tt][vPt] = pattern;
  Serial.printf("testing with fixed pattern %08X\t", pattern);
  testMsec[tt][vWr] = micros();
  for (p = memory_begin; p < memory_end; p++) {
    *p = pattern;
  }
  testMsec[tt][vWr] = micros() - testMsec[tt][vWr];
  arm_dcache_flush_delete((void *)memory_begin,
                          (uint32_t)memory_end - (uint32_t)memory_begin);
  for ( int ii = 0; ii < readRepeat; ii++) {
    testMsec[tt][vRd] = micros();
    for (p = memory_begin; p < memory_end; p++) {
      uint32_t actual = *p;
      if (actual != pattern) {
        tt++;
        Serial.println();
        return fail_message(p, actual, pattern);
      }
    }
    testMsec[tt][vRd] = micros() - testMsec[tt][vRd];
    if ( readRepeat > 1 ) Serial.print('.');
  }
  testMsec[tt][vTp] = 1;
  tt++;
  Serial.println();
  return true;
}

// fill the entire RAM with a pseudo-random sequence, then check it
bool check_lfsr_patternXX(uint32_t seed)
{
  volatile uint32_t *p;
  uint32_t reg;

  testMsec[tt][vTp] = 4;
  testMsec[tt][vPt] = seed;
  Serial.printf("testing with pseudo-random sequence, seed=%u\t", seed);
  reg = seed;
  testMsec[tt][vWr] = micros();
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
  testMsec[tt][vWr] = micros() - testMsec[tt][vWr];
  for ( int ii = 0; ii < readRepeat; ii++) {
    arm_dcache_flush_delete((void *)memory_begin,
                            (uint32_t)memory_end - (uint32_t)memory_begin);
    reg = seed;

    testMsec[tt][vRd] = micros();
    for (p = memory_begin; p < memory_end; p++) {
      uint32_t actual = *p;
      if (actual != reg) {
        tt++;
        Serial.println();
        return fail_message(p, actual, reg);
      }
      //Serial.printf(" reg=%08X\n", reg);
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
    if ( readRepeat > 1 ) Serial.print('.');
  }
  testMsec[tt][vTp] = 2;
  tt++;
  Serial.println();
  return true;
}

void memory_copy32(uint32_t *dest, const uint32_t *src, uint32_t *dest_end)
{
#if 0
  if (dest == src) return;
  do {
    *dest++ = *src++;
  } while (dest < dest_end);
#else
  asm volatile(
  " cmp %[src], %[dest]   \n"
  " beq.n 2f      \n"
  "1: ldr.w r3, [%[src]], #4  \n"
  " str.w r3, [%[dest]], #4 \n"
  " cmp %[end], %[dest]   \n"
  " bhi.n 1b      \n"
  "2:         \n"
  : [dest] "+r" (dest), [src] "+r" (src) : [end] "r" (dest_end) : "r3", "memory");
#endif
}

void memory_copy16(uint32_t *d32, const uint32_t *s32, uint32_t *de32)
{
  uint16_t *dest = (uint16_t *)d32;
  uint16_t *src = (uint16_t *)s32;
  uint16_t *dest_end = (uint16_t *)de32;
  if (dest == src) return;
  do {
    *dest++ = *src++;
  } while (dest < dest_end);
}

void memory_copy8(uint32_t *d32, const uint32_t *s32, uint32_t *de32)
{
  uint8_t *dest = (uint8_t *)d32;
  uint8_t *src = (uint8_t *)s32;
  uint8_t *dest_end = (uint8_t *)de32;
  if (dest == src) return;
  do {
    *dest++ = *src++;
  } while (dest < dest_end);
}

// fill the entire RAM with a pseudo-random sequence, then check it
bool check_lfsr_pattern(uint32_t seed)
{
  volatile uint32_t *p;
  uint32_t reg;
  static uint16_t idxHalfCopy=32;

  testMsec[tt][vTp] = 4;
  testMsec[tt][vPt] = seed;
  Serial.printf("testing with pseudo-random sequence HalfCopy %d, seed=%u\t", idxHalfCopy, seed );
  reg = seed;
  testMsec[tt][vWr] = micros();
  for (p = memory_begin; p < memory_mid; p++) {
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
  testMsec[tt][vWr] = micros() - testMsec[tt][vWr];

  switch( idxHalfCopy ) {
    case 32:
    default:
      memory_copy32( memory_mid, memory_begin, memory_end );
      idxHalfCopy = 16;
      break;
    case 16:
      memory_copy16( memory_mid, memory_begin, memory_end );
      idxHalfCopy = 8;
      break;
    case 8:
      memory_copy8( memory_mid, memory_begin, memory_end );
      idxHalfCopy = 32;
      break;
  }
  for ( int ii = 0; ii < readRepeat; ii++) {
    arm_dcache_flush_delete((void *)memory_begin,
                            (uint32_t)memory_end - (uint32_t)memory_begin);
    reg = seed;

    testMsec[tt][vRd] = micros();
    for (p = memory_mid; p < memory_end; p++) {
      uint32_t actual = *p;
      if (actual != reg) {
        tt++;
        Serial.println();
        return fail_message(p, actual, reg);
      }
      //Serial.printf(" reg=%08X\n", reg);
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
    if ( readRepeat > 1 ) Serial.print('.');
  }
  testMsec[tt][vTp] = 5;
  tt++;
  Serial.println();
  return true;
}
