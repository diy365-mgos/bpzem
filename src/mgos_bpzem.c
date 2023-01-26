#include "mgos.h"
#include "mgos_uart.h"
#include "mgos_bpzem.h"

struct mg_bpzem {
  int uart_no;
}

mgos_bpzem_t mgos_bpzem_create(enum mgos_bpzem_type type) {
  // check if the type is correct
  switch (type) {
    case MGOS_BPZEM_014:
    case MGOS_BPZEM_016:
      break;
    default:
      return NULL; // wrong type
  }

  int uart_no = 0;
  struct mgos_uart_config ucfg;
  mgos_uart_config_set_defaults(uart_no, &ucfg);

  ucfg.baud_rate = 9600;
  ucfg.num_data_bits = 8;
  ucfg.parity = MGOS_UART_PARITY_NONE;
  ucfg.stop_bits = MGOS_UART_STOP_BITS_1;

  if (!mgos_uart_configure(uart_no, &ucfg)) {
    LOG(LL_ERROR, ("Failed to configure UART%d", uart_no));
  }

  struct mg_bpzem *instance = calloc(1, sizeof(struct mg_bpzem));
  if (instance)
  {
    instance->uart_no = uart_no;
  }
  return (mgos_bpzem_t)instance;
}

bool mgos_bpzem_init(void) {
  return true;
}
