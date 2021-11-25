
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>


// RawSocket
#include <netinet/ip6.h> // IPV6 struct
#include <netinet/ip.h>
#include <netinet/tcp.h> // TCP struct
#include <net/if.h> // IFREQ strutct
#include <linux/if_packet.h> // sockaddr_ll struct
#include <netinet/ip.h>
#include <linux/if_ether.h> // ETH_P_ALL
#include <arpa/inet.h>


// Constantes
#define ETH_HDRLEN 14 // Header Ethernet
#define IP6_HDRLEN 40 // Header IPV6
#define TCP_HDRLEN 20 // Header TCP without data


uint16_t tcp6_checksum(struct ip6_hdr iphdr, struct tcphdr tcphdr);
uint16_t checksum(uint16_t* addr, int len);

int main (int argc, char **argv){

    int      i, status, frame_length, sd, bytes;
    uint8_t  src_mac[6], dst_mac[6], ether_frame[IP_MAXPACKET];
    int tcp_flags[8];
    uint8_t  iface[16], target[INET6_ADDRSTRLEN], src_ip[INET6_ADDRSTRLEN], dst_ip[INET6_ADDRSTRLEN];
 
    struct ip6_hdr iphdr;
    struct tcphdr tcphdr;
    struct sockaddr_ll device;
    struct ifreq ifr;


    if (argc == 0){
        return 0;
    }
	memset(src_mac, 0, 6 * sizeof(uint8_t));

	memset(dst_mac, 0, 6 * sizeof(uint8_t));

    memset(ether_frame, 0, IP_MAXPACKET * sizeof(uint8_t));

	memset(iface, 0, 16 * sizeof(uint8_t));

    memset(target, 0,  INET6_ADDRSTRLEN * sizeof(uint8_t));

	memset(src_ip, 0, INET6_ADDRSTRLEN * sizeof(uint8_t));

	memset(dst_ip, 0, INET6_ADDRSTRLEN * sizeof(uint8_t));

	memset(tcp_flags, 0, 8 * sizeof(int));

    strcpy((char *)iface, argv[1]);

    if((sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0){
        perror("falha ao executar socket(). Falha ao criar socket descriptor ioctl().");
        exit  (EXIT_FAILURE);

    }
    
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface);
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0){
        perror("falha ao executar ioctl(). Falha ao obter MAC address de origer");
        exit (EXIT_FAILURE);
    }
    memcpy(src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof(uint8_t));
    
    memset(&device, 0, sizeof(device));

    device.sll_ifindex = if_nametoindex((char *)iface);
    if(device.sll_ifindex ==0){
        perror("Falha ao executar if_nametoindex(); Falha ao obter index da interface");
        exit(-1);
    }    

    strcpy((char *)src_ip, (char *)argv[2]);
    strcpy((char *)dst_ip, (char *)argv[3]);
    dst_mac[0] = 0xac;
    dst_mac[1] = 0xd5;  
    dst_mac[2] = 0x64;  
    dst_mac[3] = 0xf2;  
    dst_mac[4] = 0x99;  
    dst_mac[5] = 0xa9;  

    device.sll_family = AF_PACKET;
    memcpy(device.sll_addr, src_mac, 6*sizeof(uint8_t));
    device.sll_halen = 6;
    iphdr.ip6_flow = htonl((6<<28) | (0 << 20) | 0);
    iphdr.ip6_plen = htons(TCP_HDRLEN);
    iphdr.ip6_nxt = IPPROTO_TCP;
    iphdr.ip6_hops = 255;

    if((status = inet_pton(AF_INET6, (char *)src_ip, &(iphdr.ip6_src))) !=1){
        
		fprintf(stderr, "Falha na função inet_pton(). Falha ao preencher IPv6 de origem.\nMensagem: %s", strerror(status));
		exit(EXIT_FAILURE);
    }
    // Preenche IPv6 de destino (128 bits)
	if ((status = inet_pton(AF_INET6, (char *)dst_ip, &(iphdr.ip6_dst))) != 1)
	{
		fprintf(stderr, "Falha na função inet_pton(). Falha ao preencher IPv6 de destino.\nMensagem: %s", strerror(status));
		exit(EXIT_FAILURE);
	}

   // tcphdr.th_sport = htons(18995);
   // tcphdr.th_dport = htons(12345);
    uint16_t sport=0;
    uint16_t dport=0;

    sscanf(argv[5], "%hu", &sport);
    sscanf(argv[6], "%hu",&dport);

    
    tcphdr.th_sport = htons(sport);
    tcphdr.th_dport = htons(dport);

    tcphdr.th_seq = htonl(0);
    tcphdr.th_x2 = 0;
    tcphdr.th_off = TCP_HDRLEN / 4;

    tcp_flags[0] = 0;
    tcp_flags[1] = 1;
    tcp_flags[2] = 0;
    tcp_flags[3] = 0;
    tcp_flags[4] = 0;
    tcp_flags[5] = 0;
    tcp_flags[6] = 0;
    tcp_flags[7] = 0;

    tcphdr.th_flags = 0;
    for(i = 0; i<8;i++){

        tcphdr.th_flags += (tcp_flags[i] << i);
    }

    // Tamanho da Janela
	tcphdr.th_win = htons(65535);

	// Ponteiro de Urgente
	tcphdr.th_urp = htons(0);

	// Calcula Checksum
	tcphdr.th_sum = tcp6_checksum(iphdr, tcphdr);

	// Montagem do frame ETHERNET

	// Define tamanho do Frame Ethernet
	frame_length = 6 + 6 + 2 + IP6_HDRLEN + TCP_HDRLEN;

	// Copia o MAC de destino para o frame
	memcpy(ether_frame, dst_mac, 6 * sizeof(uint8_t));

	// Copia o MAC de origem para o frame
	memcpy(ether_frame + 6, src_mac, 6 * sizeof(uint8_t));

	// Define o tipo de frame
	ether_frame[12] = ETH_P_IPV6 / 256;
	ether_frame[13] = ETH_P_IPV6 % 256;
    
	// Copia os dados do header IP para o frame
	memcpy(ether_frame + ETH_HDRLEN, &iphdr, IP6_HDRLEN * sizeof(uint8_t));

	// Copia os dados do header TCP para o frame
    
	memcpy(ether_frame + ETH_HDRLEN + IP6_HDRLEN, &tcphdr, TCP_HDRLEN * sizeof(uint8_t));


	i = 1;
	while (1){
        
	// Envia o pacote
	if ((bytes = sendto(sd, ether_frame, frame_length, 0, (struct sockaddr *)&device, sizeof(device))) <= 0)
	{
		perror("Falha ao executar função sendto(). Falha ao enviar pacote.");
		exit(EXIT_FAILURE);
	}
    
  	tcphdr.th_seq = htonl(i);
    tcphdr.th_sum = tcp6_checksum(iphdr, tcphdr);
	// Copia os dados do header TCP para o frame
	memcpy(&ether_frame[54],  &tcphdr, TCP_HDRLEN * sizeof(uint8_t));

    i++;
//    sleep(1);

	}

	// Fecha o Socket
	//pclose(sd);

	// Libera memória alocada
	free(src_mac);
	free(dst_mac);
	free(ether_frame);
	free(iface);
	free(src_ip);
	free(dst_ip);
	free(tcp_flags);

}


// Calcula o Checksum (RFC 1071)
uint16_t checksum(uint16_t *addr, int len){
	int count = len;
	register uint32_t sum = 0;
	uint16_t answer = 0;

	// Sum up 2-byte values until none or only one byte left.
	while (count > 1)
	{
		sum += *(addr++);
		count -= 2;
	}

	// Add left-over byte, if any.
	if (count > 0)
	{
		sum += *(uint8_t *)addr;
	}

	// Fold 32-bit sum into 16 bits; we lose information by doing this,
	// increasing the chances of a collision.
	// sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	// Checksum is one's compliment of sum.
	answer = ~sum;

	return (answer);
}

// Constrói o IPv6 TCP pseudo-header e chama o checksum (Section 8.1 of RFC 2460).
//
uint16_t tcp6_checksum(struct ip6_hdr iphdr, struct tcphdr tcphdr){
	uint32_t lvalue;
	char buf[IP_MAXPACKET], cvalue;
	char *ptr;
	int chksumlen = 0;

	ptr = &buf[0]; // ptr points to beginning of buffer buf

	// Copy source IP address into buf (128 bits)
	memcpy(ptr, &iphdr.ip6_src, sizeof(iphdr.ip6_src));
	ptr += sizeof(iphdr.ip6_src);
	chksumlen += sizeof(iphdr.ip6_src);

	// Copy destination IP address into buf (128 bits)
	memcpy(ptr, &iphdr.ip6_dst, sizeof(iphdr.ip6_dst));
	ptr += sizeof(iphdr.ip6_dst);
	chksumlen += sizeof(iphdr.ip6_dst);

	// Copy TCP length to buf (32 bits)
	lvalue = htonl(sizeof(tcphdr));
	memcpy(ptr, &lvalue, sizeof(lvalue));
	ptr += sizeof(lvalue);
	chksumlen += sizeof(lvalue);

	// Copy zero field to buf (24 bits)
	*ptr = 0;
	ptr++;
	*ptr = 0;
	ptr++;
	*ptr = 0;
	ptr++;
	chksumlen += 3;

	// Copy next header field to buf (8 bits)
	memcpy(ptr, &iphdr.ip6_nxt, sizeof(iphdr.ip6_nxt));
	ptr += sizeof(iphdr.ip6_nxt);
	chksumlen += sizeof(iphdr.ip6_nxt);

	// Copy TCP source port to buf (16 bits)
	memcpy(ptr, &tcphdr.th_sport, sizeof(tcphdr.th_sport));
	ptr += sizeof(tcphdr.th_sport);
	chksumlen += sizeof(tcphdr.th_sport);

	// Copy TCP destination port to buf (16 bits)
	memcpy(ptr, &tcphdr.th_dport, sizeof(tcphdr.th_dport));
	ptr += sizeof(tcphdr.th_dport);
	chksumlen += sizeof(tcphdr.th_dport);

	// Copy sequence number to buf (32 bits)
	memcpy(ptr, &tcphdr.th_seq, sizeof(tcphdr.th_seq));
	ptr += sizeof(tcphdr.th_seq);
	chksumlen += sizeof(tcphdr.th_seq);

	// Copy acknowledgement number to buf (32 bits)
	memcpy(ptr, &tcphdr.th_ack, sizeof(tcphdr.th_ack));
	ptr += sizeof(tcphdr.th_ack);
	chksumlen += sizeof(tcphdr.th_ack);

	// Copy data offset to buf (4 bits) and
	// copy reserved bits to buf (4 bits)
	cvalue = (tcphdr.th_off << 4) + tcphdr.th_x2;
	memcpy(ptr, &cvalue, sizeof(cvalue));
	ptr += sizeof(cvalue);
	chksumlen += sizeof(cvalue);

	// Copy TCP flags to buf (8 bits)
	memcpy(ptr, &tcphdr.th_flags, sizeof(tcphdr.th_flags));
	ptr += sizeof(tcphdr.th_flags);
	chksumlen += sizeof(tcphdr.th_flags);

	// Copy TCP window size to buf (16 bits)
	memcpy(ptr, &tcphdr.th_win, sizeof(tcphdr.th_win));
	ptr += sizeof(tcphdr.th_win);
	chksumlen += sizeof(tcphdr.th_win);

	// Copy TCP checksum to buf (16 bits)
	// Zero, since we don't know it yet
	*ptr = 0;
	ptr++;
	*ptr = 0;
	ptr++;
	chksumlen += 2;

	// Copy urgent pointer to buf (16 bits)
	memcpy(ptr, &tcphdr.th_urp, sizeof(tcphdr.th_urp));
	ptr += sizeof(tcphdr.th_urp);
	chksumlen += sizeof(tcphdr.th_urp);

	return checksum((uint16_t *)buf, chksumlen);
}

