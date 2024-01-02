
#define BUFFSIZE 20480 // #1: 2048  #2: 20480
#define XFERSIZE 20640 // #1: 2064  #2: 20640
#define XFEREACH 20001 // #1: 2001  #2: 20001

// EasyTransfer ???
#define SPD 6000000 // FASTER 20Mbaud? : https://forum.pjrc.com/index.php?threads/teensy4-1-max-baud-rate.67150/
#define DB_SERIAL_CNT 5
#define USED_UARTS 2
HardwareSerialIMXRT *psAll[DB_SERIAL_CNT] = { &Serial1, &Serial2, &Serial4, &Serial5, &Serial6 };

char *SerBArr[DB_SERIAL_CNT][3];
enum idxBA { iTX = 0, iRX, iXF }; // index Buffer Array Tx, Rx, XFer
#define iTX 0
#define iRX 1
#define iXF 2

// #define USERAM_DTCM 1
#if USERAM_DTCM // DTCM or DMAMEM buffers
char Where[] = "malloc DMAMEM";
// 2 Here TWO L/s=532 B/s=1106028 @XFEREACH 1000
// 2 Here TWO L/s=270 B/s=1101060 @XFEREACH 2000
#define sdram_malloc malloc
#else // sdram_malloc address SDRAM
// Opps @221 MHz : 1 Here ONE 58368 xb B[2032 2050] xb ERR[198 360]
// 2 Here TWO L/s=269 B/s=1096982
char Where[] = "sdram_malloc";
#endif
char *xfer; // = (char *)(0x80000000); // [XFERSIZE + 10];
char *xferCMP; //  = (char *)(0x80010000); // [XFERSIZE + 10];

void setup() {
  while (!Serial) ; // wait
  pinMode(LED_BUILTIN, OUTPUT);
  // if ( CrashReport ) Serial.print( CrashReport );
  for ( uint32_t ii = 0; ii < USED_UARTS; ii++ ) {
    SerBArr[ii][iTX] = (char *)sdram_malloc(24*1024);
    SerBArr[ii][iRX] = (char *)sdram_malloc(24*1024);
    SerBArr[ii][iXF] = (char *)sdram_malloc(24*1024);
  }
  xfer = (char *)sdram_malloc(24*1024);
  xferCMP = (char *)sdram_malloc(24*1024);
  Serial.printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  for ( int ii = 0; ii < USED_UARTS; ii++ ) {
    psAll[ii]->begin(SPD);
  }
  for ( int ii = 0; ii < USED_UARTS; ii++ ) {
    psAll[ii]->addMemoryForWrite(SerBArr[ii][iTX], BUFFSIZE);
    psAll[ii]->addMemoryForRead(SerBArr[ii][iRX], BUFFSIZE);
  }
  int ii; // Prep Xfer buffer and copy to Compare buffer  
  for ( ii = 0; ii < XFERSIZE; ii++ ) xfer[ii] = (ii % 91) + 33;
  for ( ; ii < XFERSIZE; ii++ ) xfer[ii] = (ii % 91) + 32;
  for ( ii = 90; ii < XFERSIZE; ii += 91 ) xfer[ii] = '\n';
  for ( ii = 0; ii < XFERSIZE; ii++) xferCMP[ii] = xfer[ii];
  xfer[XFEREACH] = 0;
}

uint32_t cntLp = 0;
uint32_t cntLpL = 0;
uint32_t cntBy = 0;
uint32_t cntByL = 0;
elapsedMillis aTime;
static int xb[USED_UARTS], xb1 = 0, xb2 = 0;
static int xbERR[USED_UARTS], xb1ERR = 0, xb2ERR = 0;

void loop() {
  int ii;
  if ( 0 == xb1 ) {
    aTime = 0;
  }
  else {
    for ( ii = 0; SerBArr[0][iXF][ii] != '!' && ii < xb1; ii++ );
    if ( 0 != memcmp( &SerBArr[0][iXF][ii], xferCMP, xb1 - ii) ) {
      Serial.print( "\txbuff 1 no cmp!\n");
      xb1ERR++;
    }
    for ( ii = 0; SerBArr[1][iXF][ii] != '!' && ii < xb2; ii++ );
    if ( 0 != memcmp( &SerBArr[1][iXF][ii], xferCMP, xb2 - ii) ) {
      Serial.print( "\txbuff 2 no cmp!\n");
      xb2ERR++;
    }
  }
  xb1 = 0;
  xb2 = 0;
  for ( ii=0; ii<USED_UARTS; ii++ ) xb[ii]=0;
  cntLp++;
  if ( aTime >= 1000 ) {
    cntLpL = cntLp;
    cntLp = 0;
    cntByL = cntBy;
    cntBy = 0;
    aTime -= 1000;
  }
  int ab1 = 0, ab2 = 0;
  int aib[USED_UARTS];
  for ( ii=0; ii<USED_UARTS; ii++ ) aib[ii]=0;
  while ( psAll[0]->available() || psAll[1]->available()) {
    ab1 = psAll[0]->available();
    ab2 = psAll[1]->available();
    if ( ab1 != 0 ) xb1 += psAll[0]->readBytes( &SerBArr[0][iXF][xb1], ab1 );
    if ( ab2 != 0 ) xb2 += psAll[1]->readBytes( &SerBArr[1][iXF][xb2], ab2 );
    delayMicroseconds(15);
  }
  SerBArr[0][iXF][xb1] = 0;
  SerBArr[1][iXF][xb2] = 0;
  char kk1,kk2;
  kk1 = SerBArr[0][iXF][xb1/20];
  kk2 = SerBArr[1][iXF][xb2/20];
  SerBArr[0][iXF][xb1/20] = 0;
  SerBArr[1][iXF][xb2/20] = 0;
  cntBy += xb1 + xb2;
  Serial.printf( "\n%.200s", SerBArr[0][iXF] );
  Serial.printf( "\n%.200s", SerBArr[1][iXF] );
  Serial.println();
  digitalToggleFast( LED_BUILTIN );
  Serial1.printf( "\n1 Here ONE %u\txb B[%u %u] xb ERR[%u %u]\n", millis(), xb1, xb2, xb1ERR, xb2ERR );
  Serial2.printf( "\n2 Here TWO L/s=%u B/s=%u MEM=%s\n", cntLpL, cntByL, Where );
  SerBArr[0][iXF][xb1/20] = kk1;
  SerBArr[1][iXF][xb2/20] = kk2;
  Serial1.write( xfer, XFEREACH );
  Serial2.write( xfer, XFEREACH );
}
