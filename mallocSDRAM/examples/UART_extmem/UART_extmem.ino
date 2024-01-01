#include "SDRAM_t4.h"
SDRAM_t4 sdram;

#define BUFFSIZE 2048
#define XFERSIZE 2064
#define XFEREACH 2001

#if 0 // DTCM buffers
// 2 Here TWO L/s=532 B/s=1106028 @XFEREACH 1000
// 2 Here TWO L/s=270 B/s=1101060 @XFEREACH 2000
char buf1t[BUFFSIZE];
char buf1r[BUFFSIZE];
char buf2t[BUFFSIZE];
char buf2r[BUFFSIZE];
char xfer[XFERSIZE + 10];
char xferCMP[XFERSIZE + 10];
char xbuff1[XFERSIZE];
char xbuff2[XFERSIZE];
#else // direct address
// Opps @221 MHz : 1 Here ONE 58368 xb B[2032 2050] xb ERR[198 360]
// 2 Here TWO L/s=269 B/s=1096982
char *buf1t = (char *)(0x80004000); // [BUFFSIZE];
char *buf1r = (char *)(0x80008000); // [BUFFSIZE];
char *buf2t = (char *)(0x8000C000); // [BUFFSIZE];
char *buf2r = (char *)(0x80010000); // [BUFFSIZE];
char *xfer = (char *)(0x80020000); // [XFERSIZE + 10];
char *xferCMP = (char *)(0x80024000); // [XFERSIZE + 10];
char *xbuff1 = (char *)(0x80028000); // [XFERSIZE];
char *xbuff2 = (char *)(0x8002c000); // [XFERSIZE];
#endif

void setup() {
  while (!Serial) ; // wait
  pinMode(LED_BUILTIN, OUTPUT);
  // if ( CrashReport ) Serial.print( CrashReport );
  if (sdram.init()) {
    Serial.print( "\n\tSUCCESS sdram.init()\n");
  }
  Serial.printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  Serial1.begin(6000000);
  Serial2.begin(6000000);
  int ii;
  for ( ii = 0; ii < XFERSIZE; ii++ ) xfer[ii] = (ii % 91) + 33;
  for ( ; ii < XFERSIZE; ii++ ) xfer[ii] = (ii % 91) + 32;
  for ( ii = 90; ii < XFERSIZE; ii += 91 ) xfer[ii] = '\n';
  for ( ii = 0; ii < XFERSIZE; ii++) xferCMP[ii] = xfer[ii];
  xfer[XFEREACH] = 0;
  Serial1.addMemoryForWrite(buf1t, BUFFSIZE);
  Serial1.addMemoryForRead(buf1r, BUFFSIZE);
  Serial2.addMemoryForWrite(buf2t, BUFFSIZE);
  Serial2.addMemoryForRead(buf2r, BUFFSIZE);
}

uint32_t cntLp = 0;
uint32_t cntLpL = 0;
uint32_t cntBy = 0;
uint32_t cntByL = 0;
elapsedMillis aTime;
void loop() {
  int ii;
  static int xb1 = 0, xb2 = 0;
  static int xb1ERR = 0, xb2ERR = 0;
  for ( ii = 0; xbuff1[ii] != '!' && ii < xb1; ii++ );
  if ( 0 != memcmp( &xbuff1[ii], xferCMP, xb1 - ii) ) {
    Serial.print( "\txbuff 1 no cmp!\n");
    xb1ERR++;
  }
  for ( ii = 0; xbuff2[ii] != '!' && ii < xb2; ii++ );
  if ( 0 != memcmp( &xbuff2[ii], xferCMP, xb2 - ii) ) {
    Serial.print( "\txbuff 2 no cmp!\n");
    xb2ERR++;
  }
  xb1 = 0;
  xb2 = 0;
  cntLp++;
  if ( aTime >= 1000 ) {
    cntLpL = cntLp;
    cntLp = 0;
    cntByL = cntBy;
    cntBy = 0;
    aTime -= 1000;
  }
  int ab1 = 0, ab2 = 0;
  while ( Serial1.available() || Serial2.available()) {
    ab1 = Serial1.available();
    ab2 = Serial2.available();
    if ( ab1 != 0 ) xb1 += Serial1.readBytes( &xbuff1[xb1], ab1 );
    if ( ab2 != 0 ) xb2 += Serial2.readBytes( &xbuff2[xb2], ab2 );
    delayMicroseconds(15);
  }
  xbuff1[xb1] = 0;
  xbuff2[xb2] = 0;
  cntBy += xb1 + xb2;
  Serial.printf( "\n%s", xbuff1 );
  Serial.printf( "\n%s", xbuff2 );
  Serial.println();
  digitalToggleFast( LED_BUILTIN );
  Serial1.printf( "\n1 Here ONE %u\txb B[%u %u] xb ERR[%u %u]\n", millis(), xb1, xb2, xb1ERR, xb2ERR );
  Serial2.printf( "\n2 Here TWO L/s=%u B/s=%u\n", cntLpL, cntByL );
  Serial1.write( xfer, XFEREACH );
  Serial2.write( xfer, XFEREACH );
}
