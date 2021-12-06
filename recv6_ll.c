/*
 * Receive an IPv6 packet via raw socket at the link layer (ethernet frame).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_ICMPV6, INET6_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <netinet/ip6.h>      // struct ip6_hdr
#include <netinet/icmp6.h>    // struct icmp6_hdr and ICMP6_ECHO_REQUEST
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <sys/time.h>         // gettimeofday()
#include <netinet/tcp.h> // TCP struct header

#include <errno.h>            // errno, perror()

// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP6_HDRLEN 40  // IPv6 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data



int main (int argc, char **argv) {

	int i, status,  sendsd, recvsd ;
	uint8_t src_mac[6], dst_mac[6], recv_ether_frame[IP_MAXPACKET];
	char interface[16], target[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];
	struct ip6_hdr *recv_iphdr;
    struct ethhdr *eth;
	struct sockaddr_ll device;
	struct ifreq ifr;
	struct sockaddr from;
	socklen_t fromlen;

    struct tcphdr *tcphdr;

    memset((uint8_t *)src_mac, 0, 6 * sizeof(uint8_t));
	memset((uint8_t *)dst_mac, 0, 6 * sizeof(uint8_t));

    
	// Interface to send packet through.
	if (argc > 1)
		strcpy(interface, argv[1]);
	else
		strcpy(interface, "eth0");

	// Submit request for a socket descriptor to look up interface.
	// We'll use it to send packets as well, so we leave it open.
	if ((sendsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror ("socket() failed to get socket descriptor for using ioctl() ");
		exit (EXIT_FAILURE);
	}

	// Use ioctl() to look up interface name and get its MAC address.
	memset (&ifr, 0, sizeof (ifr));
	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
	if (ioctl (sendsd, SIOCGIFHWADDR, &ifr) < 0) {
		perror ("ioctl() failed to get source MAC address ");
		return (EXIT_FAILURE);
	}

	// Copy source MAC address.
	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
    
	// Report source MAC address to stdout.
	printf ("MAC address for interface %s is ", interface);
	for (i = 0; i < 5; i++) {
		printf ("%02x:", src_mac[i]);
	}
	printf ("%02x\n", src_mac[5]);

	// Find interface index from interface name and store index in
	// struct sockaddr_ll device, which will be used as an argument of sendto().
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
		perror ("if_nametoindex() failed to obtain interface index ");
      exit (EXIT_FAILURE);
	}
	printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);



	// Submit request for a raw socket descriptor to receive packets.
	if ((recvsd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror ("socket() failed ");
		exit (EXIT_FAILURE);
	}

    eth =(struct ethhdr *) (recv_ether_frame);
	// Cast recv_iphdr as pointer to IPv6 header within received ethernet frame.
	recv_iphdr = (struct ip6_hdr *) (recv_ether_frame + ETH_HDRLEN);

	// Cast recv_icmphdr as pointer to ICMP header within received ethernet frame.
 
  // Cast recv_tcphdr as pointer to TCP header within received ethernet frame
  tcphdr = (struct tcphdr *)(recv_ether_frame + ETH_HDRLEN + IP6_HDRLEN);
    char flags[2];
	for (;;) {

		memset (recv_ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
		memset (&from, 0, sizeof (from));
		fromlen = sizeof (from);
		recvfrom (recvsd, recv_ether_frame, IP_MAXPACKET, 0, (struct sockaddr *) &from, &fromlen);

		// Check for an IP ethernet frame, carrying ICMP echo reply. If not, ignore and keep listening.
		if (((recv_ether_frame[12] << 8) + recv_ether_frame[13]) == ETH_P_IPV6) {
			// Extract source IP address from received ethernet frame.
			if (inet_ntop (AF_INET6, &(recv_iphdr->ip6_src),target , INET6_ADDRSTRLEN) == NULL) {
				status = errno;
				fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
				exit (EXIT_FAILURE);
			}

    
      inet_ntop(AF_INET6, &(recv_iphdr->ip6_dst), dst, INET6_ADDRSTRLEN);
      // TODO pegar as portas e as flags tcp
      inet_ntop(AF_INET6, &(tcphdr->th_flags), (char *)flags, 2);
			// Report 
      printf ("\n");
      printf ("             MAC Origem                 MAC Destino                IPV6 Origem                IPV6 Destino               Flags do Cabecalho TCP[rst - syn - fin]\n");
      printf ("         %02x:%02x:%02x:%02x:%02x:%02x    ", eth->h_source[0], eth->h_source[1],eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
      printf ("         %02x:%02x:%02x:%02x:%02x:%02x    ", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
      printf (" %-*s  ",25, target);
          printf ("  %-*s ",25,dst);
      printf ("             [RST:%02x - SYN:%02x - FIN:%02x]",tcphdr->rst, tcphdr->syn, tcphdr->fin);

      printf ("\n");

		}
	}  // End of Receive loop.


	// Close socket descriptors.
	close (sendsd);
	close (recvsd);

	return (EXIT_SUCCESS);
}

