#include "si5351.h"

bool Si5351::init(uint8_t xtal_load_c, uint32_t xo_freq, int32_t corr) {
  return true;
}

void Si5351::set_pll(uint64_t pll_freq, si5351_pll target_pll) {
  ;
}

uint8_t Si5351::set_freq(uint64_t freq, si5351_clock clk) {
  return 0;
}

void Si5351::output_enable(si5351_clock clk, uint8_t enable) {
  ;
}

void Si5351::update_status() {
  ;
}
