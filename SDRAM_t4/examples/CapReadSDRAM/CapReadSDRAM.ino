#include "SDRAM_t4.h"

SDRAM_t4 sdram;

uint32_t readRepeat = 5000; // Writes once to Test memory, will repeat reads and test compare 'readRepeat' times
uint32_t cntLoopTests=0;
// https://github.com/PaulStoffregen/teensy41_psram_memtest/blob/master/teensy41_psram_memtest.ino

uint32_t check_lfsr_pattern_halves(uint32_t seed);
void doTest( uint do_one );

static uint32_t lfsrPatt[ 44 ] = { 2976674124ul, 1438200953ul, 3413783263ul, 1900517911ul, 1227909400ul, 276562754ul, 146878114ul, 615545407ul, 110497896ul, 74539250ul, 4197336575ul, 2280382233ul, 542894183ul, 3978544245ul, 2315909796ul, 3736286001ul, 2876690683ul, 215559886ul, 539179291ul, 537678650ul, 4001405270ul, 2169216599ul, 4036891097ul, 1535452389ul, 2959727213ul, 4219363395ul, 1036929753ul, 2125248865ul, 3177905864ul, 2399307098ul, 3847634607ul, 27467969ul, 520563506ul, 381313790ul, 4174769276ul, 3932189449ul, 4079717394ul, 868357076ul, 2474062993ul, 1502682190ul, 2471230478ul, 85016565ul, 1427530695ul, 1100533073ul };
const uint32_t lfsrCnt = sizeof(lfsrPatt) / sizeof(lfsrPatt[0]);

bool memory_ok = true;
uint32_t *memory_begin, *memory_mid, *memory_end, memoryHalf;

// void SDRAMsetup();

void loop()
{
  digitalToggle(13);
//  cntLoopTests+=1;

#if 0
    bool inputSer=false;
    while ( Serial.available() ) {
      Serial.read();
      inputSer=true;
    }
    if ( inputSer ) {
      if (bTimer) {
        tTimer.end();
        Serial.printf("\n\tintervalTimer OFF\n\n");
        bTimer = false;
      }
      else {
        Serial.printf("\n\tintervalTimer ON\n\n");
        tTimer.begin(tmr_callback, 10);  // us
        bTimer = true;
      }
    }
#endif

}

uint size = 32;
void setup()
{
  while (!Serial) ; // wait
  pinMode(13, OUTPUT);
  if ( CrashReport ) Serial.print( CrashReport );
  if (sdram.init()) {
    Serial.print( "\tSUCCESS sdram.init()\n");
  }

#if 0 // todo get the right stuff
  const float clocks[4] = {396.0f, 720.0f, 664.62f, 528.0f};
  const float frequency = clocks[(CCM_CBCMR >> 8) & 3] / (float)(((CCM_CBCMR >> 29) & 7) + 1);
  Serial.printf(" CCM_CBCMR=%08X (%.1f MHz)\n", CCM_CBCMR, frequency);
#endif

  memory_begin = (uint32_t *)(0x80000000);
  memoryHalf = 0x80000000 + size * 524288;
  memory_mid = (uint32_t *)(memoryHalf);
  memory_end = (uint32_t *)(0x80000000 + size * 1048576);

  Serial.printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  Serial.printf("EXTMEM Memory Test, %d Mbyte\t", size);
  Serial.printf("begin, %08X \t", memory_begin);
  Serial.printf("middle, %08X \t", memory_mid);
  Serial.printf("end, %08X \n", memory_end);
  doTest( 1 );
  doTest( 22 );
  doTest( 42 );
}

void doTest( uint do_one ) {
  if (size == 0) return;
  if ( do_one >= lfsrCnt )  return;
  Serial.printf("\n\tPseudoRand(%u) Seed %u with readRepeat %u  ...\n", do_one, lfsrPatt[do_one], readRepeat );
  check_lfsr_pattern_halves( lfsrPatt[do_one] );
  if (memory_ok)
    Serial.printf("All memory tests passed :-) \n");
}

bool fail_message(volatile uint32_t *location, uint32_t actual, uint32_t expected)
{
  Serial.printf(" Error at %08X, read %08X but expected %08X\n",
                (uint32_t)location, actual, expected);
  return false;
}

// fill the Low half of RAM with a pseudo-random sequence, then check it against copy made to Upper half
// Test: Write same Psuedo Random values to Lower and Upper (16MB of 32MB SDRAM)
// 
uint32_t check_lfsr_pattern_halves(uint32_t seed)
{
  volatile uint32_t *p;
  volatile uint32_t *p2;
  uint32_t testMsec;
  uint32_t reg;
  uint32_t MemResSum=0, MemRes;

  reg = seed;
  testMsec = micros();
  for (p = memory_begin, p2=memory_mid; p < memory_mid; p++, p2++) {
    *p = reg;
    *p2 = reg;
    for (int i = 0; i < 3; i++) {
      if (reg & 1) {
        reg >>= 1;
        reg ^= 0x7A5BC2E3;
      } else {
        reg >>= 1;
      }
    }
  }
  arm_dcache_flush_delete((void *)memory_begin, (uint32_t)memory_end - (uint32_t)memory_begin);
  reg = seed;
  MemRes=0;
  for (p = memory_begin, p2=memory_mid; p < memory_mid; p++, p2++) {
    if ( *p != *p2 ) MemRes++;
  }

  if ( 0 != MemRes ) {
    Serial.printf("\nINSTANT FAIL pseudo-random sequence written does not match as written in both Halves: Errors Cnt %u\n", MemRes );
    memory_ok = false;
    return MemRes;
  }
  MemResSum = 0;

  for ( uint32_t ii=0; ii < readRepeat; ii++ ) {
    arm_dcache_flush_delete((void *)memory_begin, (uint32_t)memory_end - (uint32_t)memory_begin);

    for (p = memory_begin, p2=memory_mid; p < memory_mid; p++, p2++) {
      if ( *p != *p2 ) MemRes++;
    }

    MemResSum += MemRes;
  }

// validate the expected value are still read as expected
  arm_dcache_flush_delete((void *)memory_begin, (uint32_t)memory_end - (uint32_t)memory_begin);
  reg = seed;
  for (p = memory_begin, p2=memory_mid; p < memory_mid; p++, p2++) {
    if ( *p != reg ) MemRes++;
    if ( *p2 != reg ) MemRes++;
    for (int i = 0; i < 3; i++) {
      if (reg & 1) {
        reg >>= 1;
        reg ^= 0x7A5BC2E3;
      } else {
        reg >>= 1;
      }
    }
  }

  testMsec = micros() - testMsec;
  if ( 0 != MemResSum ) {
    Serial.printf("Fail pseudo-random sequence not same in both HalfCmp with seed=%u\n", seed );
    memory_ok = false;
  }
  Serial.printf("Same pseudo-random sequence in both HalfCmp Compare time secs %.2f with Error Cnt %u\n", testMsec/1000000.0, MemResSum );
  return MemResSum;
}
