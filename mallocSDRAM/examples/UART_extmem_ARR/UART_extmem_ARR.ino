
#define DB_SERIAL_CNT 5
#define USED_UARTS 5
// #define USERAM_DTCM 1
#define SPD 5000000 // FASTER 20Mbaud? : https://forum.pjrc.com/index.php?threads/teensy4-1-max-baud-rate.67150/

#define BUFFSIZE 20480 // #1: 2048  #2: 20480
#define XFERSIZE 20640 // #1: 2064  #2: 20640
#define XFEREACH 20001 // #1: 2001  #2: 20001

#ifdef ARDUINO_TEENSY40
HardwareSerialIMXRT *psAll[DB_SERIAL_CNT] = { &Serial1, &Serial2, &Serial3, &Serial4, &Serial5 };
char SerNames[DB_SERIAL_CNT][16] = { "Serial1", "Serial2", "Serial3", "Serial4", "Serial5" };
#else
HardwareSerialIMXRT *psAll[DB_SERIAL_CNT] = { &Serial1, &Serial2, &Serial4, &Serial6, &Serial5 };
char SerNames[DB_SERIAL_CNT][16] = { "Serial1", "Serial2", "Serial4", "Serial6", "Serial5" };
#endif
/*    Serial1.begin(6000000); // 2 & 0
      Serial2.begin(6000000); // 17 & 18
      Serial6.begin(6000000); // p# 24 Tx & 25 Rx
      Serial5.begin(6000000); // p# 20 Tx & 21 Rx
      Serial4.begin(6000000); // p# B1_00 Tx & B1_01 Rx*/
char *SerBArr[DB_SERIAL_CNT][3];
enum idxBA { iTX = 0, iRX, iXF }; // index Buffer Array Tx, Rx, XFer
#define iTX 0
#define iRX 1
#define iXF 2

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
  if ( CrashReport ) Serial.print( CrashReport );
  for ( uint32_t ii = 0; ii < USED_UARTS; ii++ ) {
    SerBArr[ii][iTX] = (char *)sdram_malloc(24 * 1024);
    SerBArr[ii][iRX] = (char *)sdram_malloc(24 * 1024);
    SerBArr[ii][iXF] = (char *)sdram_malloc(24 * 1024);
  }
  xfer = (char *)sdram_malloc(24 * 1024);
  xferCMP = (char *)sdram_malloc(24 * 1024);
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
static int xb[USED_UARTS];
static int xbERR[USED_UARTS];

void loop() {
  int ii, jj;
  if ( 0 == xb[0] ) {
    aTime = 0;
  }
  else {
    for ( jj = 0; jj < USED_UARTS; jj++ ) {
      for ( ii = 0; SerBArr[jj][iXF][ii] != '!' && ii < xb[jj]; ii++ );
      if ( 0 != memcmp( &SerBArr[jj][iXF][ii], xferCMP, xb[jj] - ii) ) {
        xbERR[jj]++;
        Serial.printf( "\txbuff idx=%u no cmp! : %s Err#:%u #Rx:%u #Av:%u\n", jj, SerNames[jj], xbERR[jj], xb[jj], psAll[jj]->available());
      }
    }
  }
  for ( ii = 0; ii < USED_UARTS; ii++ ) xb[ii] = 0;
  cntLp++;
  if ( aTime >= 1000 ) {
    cntLpL = cntLp;
    cntLp = 0;
    cntByL = cntBy;
    cntBy = 0;
    aTime -= 1000;
  }
  int ab[USED_UARTS];
  for ( ii = 0; ii < USED_UARTS; ii++ ) ab[ii] = 0;
  int allCnt;
  do {
    allCnt = 0;
    for ( ii = 0; ii < USED_UARTS; ii++ )
      allCnt += ab[ii] = psAll[ii]->available();
    for ( ii = 0; ii < USED_UARTS; ii++ )
      if ( ab[ii] != 0 ) xb[ii] += psAll[ii]->readBytes( &SerBArr[ii][iXF][xb[ii]], ab[ii] );
    delayMicroseconds(15);
  } while ( 0 != allCnt);
  for ( ii = 0; ii < USED_UARTS; ii++ ) {
    SerBArr[ii][iXF][xb[ii]] = 0;
    cntBy += xb[ii];
    Serial.printf( "\n%.200s", SerBArr[ii][iXF] );
  }
  Serial.println();
  digitalToggleFast( LED_BUILTIN );
  for ( ii = 0; ii < USED_UARTS; ii++ ) {
    psAll[ii]->printf( "\n%s Here %u\t #B:%u #ERR:%u /s=%u B/s=%u MEM=%s\n", SerNames[ii], millis(), xb[ii], xbERR[ii], cntLpL, cntByL, Where );
    psAll[ii]->write( xfer, XFEREACH );
  }
}
