#include "mgos.h"
#include "mgos_uart.h"
#include "mgos_modbus.h"
#include "mgos_timers.h"
#include "mgos_bpzem.h"

struct mg_bpzem {
  enum mgos_bpzem_type bpzem_type;
  uint8_t slave_id;
  mgos_bpzem_read_data_handler_t read_data;
  void* read_data_param;
};

void mg_bpzem_read_response_handler(uint8_t status, struct mb_request_info mb_ri, 
                                    struct mbuf response, void* param) {
  
  struct mg_bpzem* instance = (struct mg_bpzem*)param;
  if (instance && instance->read_data) {
    struct mgos_bpzem_data_response resp;
    resp.status = status;
    resp.success = (status == RESP_SUCCESS);

    uint8_t slave_id = (response.len ? response.buf[0] : 0);
    if (resp.success && (instance->slave_id != slave_id)) {
      LOG(LL_ERROR, ("Invalid response. A response for slave %.2x has been sent to %.2x.",
        slave_id, instance->slave_id));
      return;
    }

    // RESPONSE FORMAT
    // |ID|04|  |V    |A          |P(W)       |E(Wh)      |F(Hz)|PF   |Alarm|
    //" f8 04 10 08 c3 00 00 00 00 00 00 00 00 00 01 00 00 01 f4 40 e4 00 00 "
    resp.data.voltage = (response.len >= 5 ? (parse_value_int((uint8_t*)response.buf+3) * 0.1) : NAN);
    resp.data.current = (response.len >= 9 ? (parse_value_long_32((uint8_t*)response.buf+5) * 0.001) : NAN);
    resp.data.power = (response.len >= 13 ? (parse_value_long_32((uint8_t*)response.buf+9) * 0.1) : NAN);
    resp.data.energy = (response.len >= 17 ? parse_value_long_32((uint8_t*)response.buf+13) : NAN);
    resp.data.frequency = (response.len >= 19 ? (parse_value_int((uint8_t*)response.buf+17) * 0.1) : NAN);
    resp.data.power_factor = (response.len >= 21 ? (parse_value_int((uint8_t*)response.buf+19) * 0.01) : NAN);
    resp.data.alarm = (response.len >= 23 ? parse_value_int((uint8_t*)response.buf+21) : 0);

    // Invoke handler
    instance->read_data(instance, &resp, instance->read_data_param);

    if (resp.success) {
      LOG(LL_DEBUG, ("PZEM (%.2x), status=%.2x, voltage=%f, current=%.f, power=%f, energy=%f, frequency=%f, power factor=%f, alarm=%.2x",
        instance->slave_id, resp.status,
        resp.data.voltage, resp.data.current,
        resp.data.power, resp.data.energy,
        resp.data.frequency, resp.data.power_factor,
        resp.data.alarm)); 
    } else {
      LOG(LL_ERROR, ("Error reading data from PZEM (%.2x), status=%.2x",
        instance->slave_id, resp.status)); 
    }
  }
}

void mg_bpzem_timer_cb(void* param) {
  mgos_bpzem_read_data((mgos_bpzem_t)param);
}

bool mgos_bpzem_on_read_data(mgos_bpzem_t pzem, mgos_bpzem_read_data_handler_t read_data, void* param) {
  if (!pzem || !read_data) return false;
  struct mg_bpzem* instance = (struct mg_bpzem*)pzem;
  instance->read_data = read_data;
  instance->read_data_param = param;
  return true;
}

bool mgos_bpzem_read_data_on_poll(mgos_bpzem_t pzem, int poll_ticks) {
  if (pzem) {
    if (mgos_set_timer(poll_ticks, MGOS_TIMER_REPEAT, mg_bpzem_timer_cb, pzem) != MGOS_INVALID_TIMER_ID) {
      return true;
    }
  }
  return false;
}

bool mgos_bpzem_read_data(mgos_bpzem_t pzem) {
  struct mg_bpzem* instance = (struct mg_bpzem*)pzem;
  if (!instance || !instance->read_data) return false;
  return mb_read_input_registers(instance->slave_id, 0x0000, 8, mg_bpzem_read_response_handler, instance);
}

mgos_bpzem_t mgos_bpzem_create(uint8_t slave_id, enum mgos_bpzem_type pzem_type) {
  // check if the type is correct
  switch (pzem_type) {
    case MGOS_BPZEM_014:
    case MGOS_BPZEM_016:
      break;
    default:
      return NULL; // wrong type
  }

  struct mg_bpzem *instance = calloc(1, sizeof(struct mg_bpzem));
  if (instance) {
    instance->bpzem_type = pzem_type;
    instance->slave_id = slave_id;
  }
  return (mgos_bpzem_t)instance;
}

uint8_t mgos_bpzem_get_id(mgos_bpzem_t pzem) {
  return (pzem ? ((struct mg_bpzem *)pzem)->slave_id : 0);
}

bool mgos_bpzem_init(void) {
  return true;
}
