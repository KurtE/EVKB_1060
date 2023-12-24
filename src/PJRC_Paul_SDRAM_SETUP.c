unsigned int ns_to_clocks(float ns, float freq)
{
    float clocks = ceilf(ns * 1.0e-9f * freq);
    if (clocks < 1.0f) return 1;
    return (unsigned int)clocks;
}


bool sdram_init()
{
    // use PLL3 PFD1 664.62 divided by 4 or 5, for 166 or 133 MHz
    const unsigned int clockdiv = 5;
    CCM_CBCDR = (CCM_CBCDR & ~(CCM_CBCDR_SEMC_PODF(7))) |
        CCM_CBCDR_SEMC_CLK_SEL | CCM_CBCDR_SEMC_ALT_CLK_SEL |
        CCM_CBCDR_SEMC_PODF(clockdiv-1);
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
    SEMC_IPCR1 = 2; // IP commadns, data is 16 bits wide
    SEMC_IPCR2 = 0;

    // TODO: send IP commands to initialize SDRAM chip
    //  precharge all
    //  auto refresh (NXP SDK sends this twice, why?)
    //  mode set

    // enable refresh
    SEMC_SDRAMCR3 |= SEMC_SDRAMCR3_REN;

    // TODO: configure MPU to allow read/write, disallow exec, use cache

    return true; // hopefully SDRAM now working at 80000000 to 81FFFFFF
}


void setup() {
    Serial.begin(9600);
    Serial.println("SDRAM Init Experiment");
    if (sdram_init()) {
        Serial.println(":-)");
    } else {
        Serial.println("nope :(");
    }
}

void loop() {

}
