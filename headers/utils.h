#ifdef UTILS_HEADER
#define UTILS_HEADER

#include <netinet/tcp.h> // TCP struct
#include <netinet/ip6.h> // IPV6 struct

uint16_t checksum(uint16_t* addr, int len);
uint16_t tcp6_checksum(struct ip6_hdr, struct tcphdr);
char *allocate_strmem(int);
uint8_t *allocate_ustrmem(int);
uint16_t *allocate_intmem(int);


#endif
