#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/leds.h"
#include <stdio.h>

#define MAX_NEIGHBORS 5
#define NEIGHBOR_TIMEOUT (60) // seconds

struct neighbor {
  linkaddr_t addr;
  uint16_t rx_count;
  uint16_t tx_count;
  int16_t last_rssi;
  float prr;
  uint32_t last_seen;
};

static struct neighbor neighbors[MAX_NEIGHBORS];
static int num_neighbors = 0;

PROCESS(broadcast_process, "Broadcast with neighbor stats + RSSI sort");
AUTOSTART_PROCESSES(&broadcast_process);

/* Get current time in seconds */
static uint32_t get_time() {
  return clock_seconds();
}

/* Find neighbor index */
static int find_neighbor_index(const linkaddr_t *addr) {
  int i;
  for(i = 0; i < num_neighbors; i++) {
    if(linkaddr_cmp(&neighbors[i].addr, addr)) {
      return i;
    }
  }
  return -1;
}

/* Remove expired neighbors */
static void remove_stale_neighbors() {
  uint32_t now = get_time();
  int i = 0;
  while(i < num_neighbors) {
    if(now - neighbors[i].last_seen > NEIGHBOR_TIMEOUT) {
      printf("Removing neighbor %d.%d due to timeout\n",
             neighbors[i].addr.u8[0], neighbors[i].addr.u8[1]);

      int j;
      for(j = i; j < num_neighbors - 1; j++) {
        neighbors[j] = neighbors[j + 1];
      }
      num_neighbors--;
    } else {
      i++;
    }
  }
}

/* Update neighbor info */
static void update_neighbor(const linkaddr_t *from, int16_t rssi) {
  int index = find_neighbor_index(from);

  if(index != -1) {
    neighbors[index].rx_count++;
    neighbors[index].last_rssi = rssi;
    neighbors[index].last_seen = get_time();
    if(neighbors[index].tx_count > 0) {
      neighbors[index].prr = (float)neighbors[index].rx_count / neighbors[index].tx_count;
    }
  } else if(num_neighbors < MAX_NEIGHBORS) {
    linkaddr_copy(&neighbors[num_neighbors].addr, from);
    neighbors[num_neighbors].rx_count = 1;
    neighbors[num_neighbors].tx_count = 1; // assume TX=1 if unknown
    neighbors[num_neighbors].last_rssi = rssi;
    neighbors[num_neighbors].prr = 1.0;
    neighbors[num_neighbors].last_seen = get_time();
    num_neighbors++;
  } else {
    printf("Neighbor table full. Ignoring %d.%d\n", from->u8[0], from->u8[1]);
  }
}

/* Sort by RSSI descending */
static void sort_neighbors_by_rssi() {
  int i, j;
  for(i = 0; i < num_neighbors - 1; i++) {
    for(j = i + 1; j < num_neighbors; j++) {
      if(neighbors[j].last_rssi > neighbors[i].last_rssi) {
        struct neighbor temp = neighbors[i];
        neighbors[i] = neighbors[j];
        neighbors[j] = temp;
      }
    }
  }
}

/* Print neighbor table */
static void print_neighbors() {
  int i;
  sort_neighbors_by_rssi(); // Sort before print

  printf("=== Neighbor Table (sorted by RSSI) ===\n");
  for(i = 0; i < num_neighbors; i++) {
    printf("Node %d.%d - RX: %u, TX: %u, RSSI: %d dBm, PRR: %.2f\n",
           neighbors[i].addr.u8[0], neighbors[i].addr.u8[1],
           neighbors[i].rx_count, neighbors[i].tx_count,
           neighbors[i].last_rssi, (double)neighbors[i].prr);
  }
  printf("=======================================\n");
}

/* Broadcast receive callback */
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
  int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  printf("Received packet from %d.%d (RSSI: %d dBm)\n",
         from->u8[0], from->u8[1], rssi);

  update_neighbor(from, rssi);
  print_neighbors();
}

/* Broadcast callbacks */
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

    remove_stale_neighbors();

    int i;
    for(i = 0; i < num_neighbors; i++) {
      neighbors[i].tx_count++;
      neighbors[i].prr = (float)neighbors[i].rx_count / neighbors[i].tx_count;
    }

    packetbuf_copyfrom("Hello", 6);
    broadcast_send(&broadcast);
    printf("Broadcast message sent\n");
  }

  PROCESS_END();
}
