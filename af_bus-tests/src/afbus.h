#ifndef AF_BUS_H
#define AF_BUS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/filter.h>
#include <sys/un.h>
#include <errno.h>
#include <inttypes.h>

#define PF_BUS			40
#define AF_BUS			PF_BUS

#define SOL_BUS		        280

#define BUS_ADD_ADDR            1
#define BUS_JOIN_BUS            2
#define BUS_DEL_ADDR            3
#define BUS_SET_EAVESDROP       4
#define BUS_UNSET_EAVESDROP     5
#define BUS_SET_SENDBUF 	6
#define BUS_SET_MAXQLEN 	7

/* 'protocol' to use in socket(AF_BUS, SOCK_SEQPACKET, protocol) */
#define BUS_PROTO_NONE 0
#define BUS_PROTO_DBUS 1
#define BUS_PROTO_MAX  1

#define BUS_PATH_MAX	        108

/* Bus address */
struct bus_addr {
	uint64_t s_addr; /* 16-bit prefix + 48-bit client address */
};

/* Structure describing an AF_BUS socket address. */
struct sockaddr_bus {
	sa_family_t sbus_family; /* AF_BUS */
	struct bus_addr	     sbus_addr; /* bus address */
	char sbus_path[BUS_PATH_MAX];  /* pathname */
};

#define BUS_MASTER_ADDR 0x0
#define BUS_PREFIX_BITS 16
#define BUS_CLIENT_BITS 48
#define BUS_PREFIX_MASK 0xffff000000000000
#define BUS_CLIENT_MASK 0x0000ffffffffffff

#define CN_IDX_NFDBUS                  0xA     /* netfilter D-Bus */
#define CN_VAL_NFDBUS                  0x1

#define NFDBUS_CMD_ADDMATCH     0x01
#define NFDBUS_CMD_REMOVEMATCH  0x02
#define NFDBUS_CMD_REMOVEALLMATCH  0x03

struct nfdbus_nl_cfg_req {
        __u32 cmd;
        __u32 len;
        struct sockaddr_bus addr;
        __u64 pad;
        unsigned char data[0];
};

struct nfdbus_nl_cfg_reply {
	__u32 ret_code;
};


#endif /* AF_BUS_H */
