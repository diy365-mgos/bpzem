/*
 * Copyright (c) 2023 DIY356
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_BPZEM_H_
#define MGOS_BPZEM_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MGOS_BPZEM_DEFAULT_ADDR 0xF8

enum mgos_bpzem_type {
  MGOS_BPZEM_014 = 14,
  MGOS_BPZEM_016 = 16,
};

struct mg_bpzem;

/* Generic and opaque bPZEM instance */
typedef struct mg_bpzem *mgos_bpzem_t;

mgos_bpzem_t mgos_bpzem_create(uint8_t slave_id, enum mgos_bpzem_type bpzem_type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_BPZEM_H_ */