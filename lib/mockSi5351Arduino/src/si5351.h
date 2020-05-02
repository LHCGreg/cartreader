#ifndef cartreader_mock_si5351_h
#define cartreader_mock_si5351_h

#include <stdint.h>

#define SI5351_CRYSTAL_LOAD_8PF (2 << 6)
#define SI5351_PLL_FIXED 80000000000ULL

enum si5351_pll {SI5351_PLLA, SI5351_PLLB};
enum si5351_clock {SI5351_CLK0, SI5351_CLK1, SI5351_CLK2, SI5351_CLK3,
	SI5351_CLK4, SI5351_CLK5, SI5351_CLK6, SI5351_CLK7};

class Si5351 {
  public:

  bool init(uint8_t xtal_load_c, uint32_t xo_freq, int32_t corr);
  void set_pll(uint64_t pll_freq, si5351_pll target_pll);
  uint8_t set_freq(uint64_t freq, si5351_clock clk);
  void output_enable(si5351_clock clk, uint8_t enable);
  void update_status();
};

#endif
