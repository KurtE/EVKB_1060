#define SHOWSSD 1
#if SHOWSSD
#include "SSD1306help.h"
#endif

#define DB_SERIAL_CNT 5
#define USED_UARTS 5 // Set to number UARTS to test from psAll[] below
// #define USERAM_DTCM 1
#define SPD 3000000 // FASTER 20Mbaud? : https://forum.pjrc.com/index.php?threads/teensy4-1-max-baud-rate.67150/

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
//    Serial1// 2 & 0 :: Serial2// 17 & 18 :: Serial6// p# 24 Tx & 25 Rx
//      Serial5// p# 20 Tx & 21 Rx :: Serial4// p# B1_00 Tx & B1_01 Rx
char *SerBArr[DB_SERIAL_CNT][3];
enum idxBA { iTX = 0, iRX, iXF }; // index Buffer Array Tx, Rx, XFer
#define iTX 0
#define iRX 1
#define iXF 2

#if USERAM_DTCM // DTCM or DMAMEM buffers
char Where[] = "malloc DMAMEM";
#else // sdram_malloc address SDRAM
char Where[] = "sdram_malloc";
#endif
char *xfer; // Outgoing Packet Buffer
char *xferCMP; // Duplicate data to compare incoming data

uint32_t cntLp = 0;
uint32_t cntLpL = 0;
uint32_t cntBy = 0;
uint32_t cntByL = 0;
static int xb[USED_UARTS];
static int xbERR[USED_UARTS];
elapsedMillis aTime;

void setup() {
  uint32_t flSDRAM = 0;
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(13, HIGH);
  while (!Serial) ; // wait
  if ( CrashReport ) Serial.print( CrashReport );
  for ( uint32_t ii = 0; ii < USED_UARTS; ii++ ) {
    for ( uint32_t jj = 0; jj < 3; jj++ ) {
      SerBArr[ii][jj] = (char *)sdram_malloc(24 * 1024);
      if ( 0 == SerBArr[ii][jj] ) flSDRAM++;
    }
  }
  xfer = (char *)sdram_malloc(24 * 1024);
  if ( 0 == xfer ) flSDRAM++;
  xferCMP = (char *)sdram_malloc(24 * 1024);
  if ( 0 == xferCMP ) flSDRAM++;
  Serial.printf("Compile Time:: " __FILE__ " " __DATE__ " " __TIME__ "\n");
  if ( 0 != flSDRAM ) {
    Serial.printf("\t%u sdram_malloc FAILS!!\n");
    while (1);
  }
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
  // FEED the UARTS for First loop()
  for ( ii = 0; ii < USED_UARTS; ii++ ) {
    psAll[ii]->printf( "\nSETUP %s Here %u\t #B:%u #ERR:%u /s=%u B/s=%u MEM=%s\n", SerNames[ii], millis(), xb[ii], xbERR[ii], cntLpL, cntByL, Where );
    psAll[ii]->write( xfer, XFEREACH );
  }
  delay(100); // Allow first UART transfers to proceed
#if SHOWSSD
  ssdSetup();
#endif
  aTime = 0;
}

void loop() {
  int ii, jj;
  for ( jj = 0; 0 != cntLp && jj < USED_UARTS; jj++ ) { // Skip first 0 != cntLp, needed var xb[] on ReadBytes follows
    for ( ii = 0; SerBArr[jj][iXF][ii] != '!' && ii < xb[jj]; ii++ ); // GET ii: Skip 'stats' before packet starting "!"
    if ( 0 == ii || 90 < ii || 0 != memcmp( &SerBArr[jj][iXF][ii], xferCMP, XFEREACH) ) {
      xbERR[jj]++;
      Serial.printf( "\txbuff idx=%u no cmp! : %s Err#:%u #Rx:%u #Av:%u #ii:\n", jj, SerNames[jj], xbERR[jj], xb[jj], psAll[jj]->available(), ii);
#ifdef USB_DUAL_SERIAL
      SerialUSB1.printf( "\txbuff idx=%u no cmp! : %s Err#:%u #Rx:%u #Av:%u #ii:%u\n", jj, SerNames[jj], xbERR[jj], xb[jj], psAll[jj]->available(), ii);
#endif
      delay(1000);
    }
  }
  cntLp++;
  if ( aTime >= 1000 ) {
    textdraw();
    cntLpL = cntLp;
    cntByL = cntBy;
    cntLp = cntBy = 0;
    aTime -= 1000;
    digitalToggleFast( LED_BUILTIN );
  }
  for ( ii = 0; ii < USED_UARTS; ii++ ) xb[ii] = 0;
  int allCnt;
  int ab[USED_UARTS];
  for ( ii = 0; ii < USED_UARTS; ii++ ) ab[ii] = 0;
  do {
    allCnt = 0;
    for ( ii = 0; ii < USED_UARTS; ii++ )
      allCnt += ab[ii] = psAll[ii]->available();
    for ( ii = 0; ii < USED_UARTS; ii++ )
      if ( ab[ii] != 0 ) xb[ii] += psAll[ii]->readBytes( &SerBArr[ii][iXF][xb[ii]], ab[ii] );
    delayMicroseconds(50);
  } while ( 0 != allCnt);
  for ( ii = 0; ii < USED_UARTS; ii++ ) {
    SerBArr[ii][iXF][xb[ii]] = 0;
#ifdef USB_DUAL_SERIAL
    if ( xb[ii] < (XFEREACH) || xb[ii] > (XFEREACH + 90 ) ) {
      SerialUSB1.printf( "\n%.25s\n", SerBArr[ii][iXF] );
      if (xb[ii] > 50)
        SerialUSB1.printf( "...\n%s\n\tms=%u xb[ii]=%u\n", &SerBArr[ii][iXF][xb[ii] - 20], millis(), xb[ii] );
      else
        SerialUSB1.printf( "...\n%s\n\tms=%u xb[ii]=%u\n", &SerBArr[ii][iXF][xb[ii]], millis(), xb[ii] );
    }
#endif
    cntBy += xb[ii];
    Serial.printf( "\n%.150s", SerBArr[ii][iXF] );
  }
  for ( ii = 0; ii < USED_UARTS; ii++ ) {
    psAll[ii]->printf( "\n%s Here %u\t #B:%u #ERR:%u Lp/s=%u B/s=%u MEM=%s\n", SerNames[ii], millis(), xb[ii], xbERR[ii], cntLpL, cntByL, Where );
    psAll[ii]->write( xfer, XFEREACH );
  }
  Serial.println("\n\t --------------------\n");
}
void textdraw(void) {
#if SHOWSSD
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  for ( int ii = 0; ii < USED_UARTS; ii++ ) {
    display.printf( "%u #B%u #ERR:%u\n", ii, xb[ii], xbERR[ii] );
  }
  display.printf( "    %s\nms=%u Lp/s:%u\nB/s:%u", Where, millis(), cntLpL, cntByL );
  display.display();
#endif
}
