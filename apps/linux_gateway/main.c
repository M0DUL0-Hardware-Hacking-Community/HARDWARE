#include "power10/config/device_config.h"
#include "power10/firmware/device_app.h"

#include <stdint.h>
#include <stdio.h>

int main(void) {
  power10_device_state state = {0};
  for (uint32_t cycle = 0U; cycle < POWER10_MAXIMUM_HOST_CYCLES; ++cycle) {
    if (power10_run_device_cycle(&state) != POWER10_CYCLE_OK) {
      (void)fputs("device cycle failed\n", stderr);
      return 1;
    }
  }
  return 0;
}
