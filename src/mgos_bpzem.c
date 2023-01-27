#include "mgos.h"
#include "mgos_uart.h"
#include "mgos_bpzem.h"

struct mg_bpzem {
  enum mgos_bpzem_type bpzem_type;
  uint8_t slave_id;
};

mgos_bpzem_t mgos_bpzem_create(uint8_t slave_id, enum mgos_bpzem_type bpzem_type) {
  // check if the type is correct
  switch (bpzem_type) {
    case MGOS_BPZEM_014:
    case MGOS_BPZEM_016:
      break;
    default:
      return NULL; // wrong type
  }

  struct mg_bpzem *instance = calloc(1, sizeof(struct mg_bpzem));
  if (instance) {
    instance->bpzem_type = bpzem_type;
    instance->slave_id = slave_id;
  }
  return (mgos_bpzem_t)instance;
}


bool mgos_bpzem_init(void) {
  if (mgos_sys_config_get_bpzem_init_modbus()) {
    if (!mgos_modbus_connect()) {
      return false;
    }
  }
  return true;
}
