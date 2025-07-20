/**
 * \file
 *         Assignment 3 Counting the neighbor nodes 
 * \author
 *         Khanh Hoang - MSSV : 2113392
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

#define MAX_NEIGHBORS 10

struct neighbor {
  linkaddr_t addr;
  uint16_t rx_count;
};

static struct neighbor neighbors[MAX_NEIGHBORS];
static int num_neighbors = 0;

PROCESS(broadcast_process, "Broadcast process with neighbor counting");
AUTOSTART_PROCESSES(&broadcast_process);

static int find_neighbor_index(const linkaddr_t *addr) {
  int i;
  for(i = 0; i < num_neighbors; i++) {
    if(linkaddr_cmp(&neighbors[i].addr, addr)) {
      return i;
    }
  }
  return -1;
}

static void update_neighbor(const linkaddr_t *from) {
  int index = find_neighbor_index(from);

  if(index != -1) {
    neighbors[index].rx_count++;
  } else if(num_neighbors < MAX_NEIGHBORS) {
    linkaddr_copy(&neighbors[num_neighbors].addr, from);
    neighbors[num_neighbors].rx_count = 1;
    num_neighbors++;
  } else {
    printf("Neighbor table full. Ignoring %d.%d\n", from->u8[0], from->u8[1]);
  }
}

static void print_neighbors() {
  printf("=== Neighbor Table ===\n");
  int i;
  for(i = 0; i < num_neighbors; i++) {
    printf("Neighbor %d.%d - RX: %u packets\n",
           neighbors[i].addr.u8[0], neighbors[i].addr.u8[1],
           neighbors[i].rx_count);
  }
  printf("=======================\n");
}

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
  printf("Broadcast received from %d.%d\n", from->u8[0], from->u8[1]);
  update_neighbor(from);
  print_neighbors();
}

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

PROCESS_THREAD(broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {
    etimer_set(&et, CLOCK_SECOND * 2 + random_rand() % (CLOCK_SECOND * 2));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    printf("Broadcast message sent\n");
  }

  PROCESS_END();
}
