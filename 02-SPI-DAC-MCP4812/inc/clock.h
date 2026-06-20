 //******************
  //Clock Library
  //
  // CREATED: Sept/24/2024, by Carlos Estay
  //
  // FILE: clock.h
  //
  //
 #include "stm32g0b1xx.h"

  #ifndef CLOCK_H
  #define CLOCK_H

  #define RCC_CFGR_SW_PLL RCC_CFGR_SW_1 //PLL as system clock
  /*  
    PLL_CLK = PLL_IN x (N / M) / R 
    Minumum R = 2

    R and M factor must subtract 1 as 0 means setting 1
  */
typedef enum PllRangeTypedef__
{
    // 64 MHz (N=8,  M=1, R=2)
    PLL_64MHZ = 0x30000802,

    // 60 MHz (N=15, M=1, R=4)
    PLL_60MHZ = 0x70000F02,

    // 50 MHz (N=25, M=2, R=4)
    PLL_50MHZ = 0x70001912,

    // 48 MHz (N=12, M=1, R=4)
    PLL_48MHZ = 0x70000C02,

    // 40 MHz (N=10, M=1, R=4)
    PLL_40MHZ = 0x70000A02,

    // 32 MHz (N=8,  M=1, R=4)
    PLL_32MHZ = 0x70000802,

    // 20 MHz (N=10, M=1, R=8)
    PLL_20MHZ = 0xF0000A02,

    // 16 MHz (N=8,  M=1, R=8)
    PLL_16MHZ = 0xF0000802,

} PllRange;


  typedef enum MCO_DivTpedef__
  {
      MCO_Div1 = 0U << 28,
      MCO_Div2 = 1U << 28,
      MCO_Div4 = 2U << 28,
      MCO_Div8 = 3U << 28,
      MCO_Div16 = 4U << 28,
      MCO_Div32 = 5U << 28,
      MCO_Div64 = 6U << 28,  
      MCO_Div128= 7U << 28,
      MCO_Div256  = 8u << 28,   // G0B1 only
      MCO_Div512  = 9u << 28,   // G0B1 only
      MCO_Div1024 = 10u << 28   // G0B1 only
  }MCO_Div;

  typedef enum MCO_SelectTpedef__
  {
      MCO_Sel_None = 0U << 24,
      MCO_Sel_SYSCLK = 1U << 24,
      MCO_Sel_HSI16 = 2U << 24,
      MCO_Sel_HSI48 = 2u  << 24,   // G0B1 only
      MCO_Sel_MSI = 3U << 24,
      MCO_Sel_HSE = 4U << 24,
      MCO_Sel_PLL = 5U << 24,
      MCO_Sel_LSI = 6U << 24,
      MCO_Sel_LSE = 7U << 24,
      MCO_Sel_PLLPCLK  = 8u  << 24,   // G0B1 only
      MCO_Sel_PLLQCLK  = 9u  << 24,   // G0B1 only
      MCO_Sel_RTCCLK   = 10u << 24,   // G0B1 only
      MCO_Sel_RTCWU    = 11u << 24    // G0B1 only
  }MCO_Select;


  void Clock_InitPll(PllRange pllRange);
  void Clock_EnableOutput(MCO_Select mcoSel, MCO_Div mcoDiv);
  void Clock_LSE_Init();
  #endif /* CLOCK_H */