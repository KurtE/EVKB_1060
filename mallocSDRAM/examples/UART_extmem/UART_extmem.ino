#include "SDRAM_t4.h"
SDRAM_t4 sdram;

#define BUFFSIZE 2048
#define XFERSIZE 2064
#define XFEREACH 2001

// EasyTransfer ???
#define SPD 2000000
HardwareSerial *psAll[8] = { &Serial1, &Serial2, &Serial3, &Serial4, &Serial5, &Serial6, &Serial7, &Serial7 };
void XXXsetup() {
 pinMode(13, OUTPUT);
  for ( int ii = 0; ii < 7; ii++ ) {
    psAll[ii]->begin(SPD);
  }
}
char buffer[80];

void serialXfer(HardwareSerial * psIn, HardwareSerial * psOut, bool bWrite, uint32_t ii)
{
int whoami = 2;
char chWho = 'z';
  
  uint32_t cb = psIn->available();
  buffer[0] = 0; // NULL buffer when nothing read !!!
  if (cb) {
    if ( 0 != whoami ) Serial.print( cb ); // bugbug debug
    if (cb > sizeof(buffer)) cb = sizeof(buffer);
    if (cb >  (uint32_t)psOut->availableForWrite()) cb = (uint32_t)psOut->availableForWrite();
    if ( 0 == psIn->readBytes(buffer, cb) ) {
      Serial.printf( "Read Error on %u - expected %u Bytes" , ii, cb );
      buffer[0] = 0; // NULL buffer when nothing read !!!
    }
    else if ( bWrite ) {
      psOut->print((char)( chWho + ii));
      psOut->write(buffer, cb);
    }
  }
  if ( 0 == whoami ) // bugbug debug // FORCE OUTPUT
    psOut->print((char)( chWho + ii));  // bugbug debug
}

#if 0 // DTCM buffers
char Where[]="not SDRAM";
// 2 Here TWO L/s=532 B/s=1106028 @XFEREACH 1000
// 2 Here TWO L/s=270 B/s=1101060 @XFEREACH 2000
#define DMA_RAM DMAMEM // comment for DTCM
DMA_RAM char buf1t[BUFFSIZE];
DMA_RAM char buf1r[BUFFSIZE];
DMA_RAM char buf2t[BUFFSIZE];
DMA_RAM char buf2r[BUFFSIZE];
DMA_RAM char xfer[XFERSIZE + 10];
DMA_RAM char xferCMP[XFERSIZE + 10];
DMA_RAM char xbuff1[XFERSIZE];
DMA_RAM char xbuff2[XFERSIZE];
#else // direct address
// Opps @221 MHz : 1 Here ONE 58368 xb B[2032 2050] xb ERR[198 360]
// 2 Here TWO L/s=269 B/s=1096982
char Where[]="IN SDRAM";
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
  Serial2.printf( "\n2 Here TWO L/s=%u B/s=%u MEM=%s\n", cntLpL, cntByL, Where );
  Serial1.write( xfer, XFEREACH );
  Serial2.write( xfer, XFEREACH );
}
