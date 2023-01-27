#include "mgos.h"
#include "mgos_uart.h"
#include "mgos_bpzem.h"
#include "mgos_modbus.h"

struct mg_bpzem {
  enum mgos_bpzem_type bpzem_type;
  uint8_t slave_id;
};

void mg_bpzem_read_response_handler(uint8_t status, struct mb_request_info mb_ri, 
                                    struct mbuf response, void* param) {
  struct mg_bpzem *instance = (struct mg_bpzem *)param;
  
  char str[1024];
  int length = 0;
  for (int i = 0; i < buffer.len && i < sizeof(str) / 3; i++) {
    length += sprintf(str + length, "%.2x ", buffer.buf[i]);
  }
  if (status == RESP_SUCCESS) {
    LOG(LL_INFO, ("%f - VALID RESPONSE, Status: %d, Buffer: %.*s", mgos_uptime(), status, length, str));
  } else {
    LOG(LL_INFO, ("%f - Invalid response, Status: %d, Buffer: %.*s", mgos_uptime(), status, length, str));
  }
}

void mg_bpzem_timer_cb(void* param) {
  struct mg_bpzem *instance = (struct mg_bpzem *)param;
  mb_read_holding_registers(instance->slave_id, 0xF8, 8, mg_bpzem_read_response_handler, instance);
}

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
    mgos_set_timer(5000, MGOS_TIMER_REPEAT, mg_bpzem_timer_cb, instance);
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
