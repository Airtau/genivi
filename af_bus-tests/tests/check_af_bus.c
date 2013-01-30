#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/connector.h>
#include <linux/sockios.h>


#include "../src/afbus.h"

#define MAX_CLIENT 10

static int seq = 1;

static void netlink_send(int nlsock, struct cn_msg *msg)
{
  struct nlmsghdr *nlh;
  unsigned int size;
  int ret;
  char buf[4096];
  struct cn_msg *m;

  size = NLMSG_SPACE(sizeof(struct cn_msg) + msg->len);

  nlh = (struct nlmsghdr *)buf;
  nlh->nlmsg_seq = seq++;
  nlh->nlmsg_pid = getpid();
  nlh->nlmsg_type = NLMSG_DONE;
  nlh->nlmsg_len = NLMSG_LENGTH(size - sizeof(*nlh));
  nlh->nlmsg_flags = 0;

  m = NLMSG_DATA(nlh);
  memcpy(m, msg, sizeof(*m) + msg->len);

  ret = send(nlsock, nlh, size, 0);
  fail_unless (ret > 0, "Netlink send failed");
}

static void add_match_rule(struct sockaddr_bus *address,
                           const char *rule,
                           int cmd,
                           int expect_success)
{
  static int nlsock = 0;

  int ret;
  char buf[sizeof(struct cn_msg) + sizeof(struct nfdbus_nl_cfg_req) + 1024];

  struct cn_msg *data;
  struct nlmsghdr *reply;
  struct nfdbus_nl_cfg_req *req;
  struct nfdbus_nl_cfg_reply *req_reply;

  if (nlsock == 0)
    {
      struct sockaddr_nl l_local;

      nlsock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
      fail_unless (nlsock > 0, "Netlink socket failed");

      l_local.nl_family = AF_NETLINK;
      l_local.nl_groups = 0;
      l_local.nl_pid = 0;
      ret = bind(nlsock, (struct sockaddr *)&l_local, sizeof(struct sockaddr_nl));
      fail_unless (ret == 0, "Netlink bind failed");
    }

  memset(buf, 0, sizeof(buf));

  data = (struct cn_msg *)buf;

  data->id.idx = CN_IDX_NFDBUS;
  data->id.val = CN_VAL_NFDBUS;
  data->seq = seq++;
  data->ack = 0;
  data->len = sizeof(struct nfdbus_nl_cfg_req) + strlen(rule) + 1;
  req = (struct nfdbus_nl_cfg_req *) data->data;

  req->cmd = cmd;
  req->len = strlen(rule) + 1;
  req->addr = *address;
  strcpy((char *)req->data, rule);

  netlink_send(nlsock, data);

  memset(buf, 0, sizeof(buf));
  ret = recv(nlsock, buf, sizeof(buf), 0);
  fail_unless (ret > 0, "Netlink recv failed");

  reply = (struct nlmsghdr *)buf;
  if (expect_success)
    fail_unless (reply->nlmsg_type == NLMSG_DONE, "Netlink reply failed");
  else
    fail_unless (reply->nlmsg_type == NLMSG_ERROR, "Netlink reply failed");
}

START_TEST (test_bus_create)
{
  int ret;
  int sockfd;

  sockfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (sockfd != -1, "Seqpacket socket creation failed");
 
  ret = close(sockfd);
  fail_unless (ret == 0, "Close failed");

  sockfd = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_DBUS);
  fail_unless (sockfd != -1, "Seqpacket socket creation failed");
 
  ret = close(sockfd);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_bind)
{
  int ret;
  int sockfd;
  struct sockaddr_bus address;

  sockfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (sockfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_bind");
  ret = bind(sockfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = close(sockfd);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_getsockname)
{
  int ret;
  int sockfd;
  struct sockaddr_bus address;
  int addr_len = sizeof(address);

  sockfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (sockfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_sockname");
  ret = bind(sockfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  memset(&address, 0, sizeof(address));
  ret = getsockname(sockfd, (struct sockaddr *) &address, &addr_len);
  fail_unless(ret == 0, "Get sock name failed %d", ret);

  ret = close(sockfd);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_connect)
{
  int ret;
  int i;
  int serverfd;
  int clientfd[MAX_CLIENT];
  struct sockaddr_bus address;
  int addr_len = sizeof(address);

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_connect");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	  clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
	  fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
	  ret = connect(clientfd[i], (struct sockaddr *) &address, sizeof(address));
	  fail_unless (ret == 0, "connect failed");
  }

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	       ret = close(clientfd[i]);
	       fail_unless (ret == 0, "Close failed");
  }
}
END_TEST

START_TEST (test_bus_connect_sub_protocol)
{
  int ret;
  int i;
  int serverfd_none;
  int serverfd_dbus;
  int clientfd_none;
  int clientfd_dbus;
  struct sockaddr_bus address_none;
  struct sockaddr_bus address_dbus;

  serverfd_none = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_NONE);
  fail_unless (serverfd_none != -1, "Seqpacket socket creation failed");

  serverfd_dbus = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_DBUS);
  fail_unless (serverfd_dbus != -1, "Seqpacket socket creation failed");

  memset(&address_none, 0, sizeof(address_none));
  address_none.sbus_family = AF_BUS;
  strcpy(address_none.sbus_path, "/tmp/bus_connect_none");
  ret = bind(serverfd_none, (struct sockaddr *) &address_none, sizeof(address_none));
  fail_unless(ret == 0, "Bind failed %d", ret);

  memset(&address_dbus, 0, sizeof(address_dbus));
  address_dbus.sbus_family = AF_BUS;
  strcpy(address_dbus.sbus_path, "/tmp/bus_connect_dbus");
  ret = bind(serverfd_dbus, (struct sockaddr *) &address_dbus, sizeof(address_dbus));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd_none, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  ret = listen(serverfd_dbus, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  clientfd_none = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_NONE);
  fail_unless (clientfd_none != -1, "Seqpacket socket creation failed");
  ret = connect(clientfd_none, (struct sockaddr *) &address_dbus, sizeof(address_dbus));
  fail_unless (ret == -1, "connect wrongly succeed");
  ret = connect(clientfd_none, (struct sockaddr *) &address_none, sizeof(address_none));
  fail_unless (ret == 0, "connect failed");

  clientfd_dbus = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_DBUS);
  fail_unless (clientfd_dbus != -1, "Seqpacket socket creation failed");
  ret = connect(clientfd_dbus, (struct sockaddr *) &address_none, sizeof(address_none));
  fail_unless (ret == -1, "connect wrongly succeed");
  ret = connect(clientfd_dbus, (struct sockaddr *) &address_dbus, sizeof(address_dbus));
  fail_unless (ret == 0, "connect failed");

  ret = close(serverfd_none);
  fail_unless (ret == 0, "Close failed");
  ret = close(serverfd_dbus);
  fail_unless (ret == 0, "Close failed");

  ret = close(clientfd_none);
  fail_unless (ret == 0, "Close failed");
  ret = close(clientfd_dbus);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_connect_getsockname)
{
  int ret;
  int i;
  int serverfd;
  int clientfd[MAX_CLIENT];
  struct sockaddr_bus address;
  struct sockaddr_bus raddress;
  int addr_len = sizeof(raddress);

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_connect_getsockname");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, 10);
  fail_unless(ret == 0, "Listen failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	  clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
	  fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
	  ret = connect(clientfd[i], (struct sockaddr *) &address,
			sizeof(address));
	  fail_unless (ret == 0, "connect failed");

	  memset(&raddress, 0, sizeof(raddress));
	  ret = getsockname(clientfd[i], (struct sockaddr *) &raddress, &addr_len);
	  fail_unless(ret == 0, "Get sock name failed %d", ret);
  }

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	       ret = close(clientfd[i]);
	       fail_unless(ret == 0, "Close failed");
  }
}
END_TEST

START_TEST (test_bus_send)
{
  int ret;
  int i;
  int serverfd;
  int clientfd[MAX_CLIENT];
  int afd[MAX_CLIENT];
  struct sockaddr_bus address;
  int addr_len = sizeof(address);
  char *msg = "hello world\n";
  char *msg2 = "good bye world\n";
  char buffer[1024];

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_send");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	  clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
	  fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
	  ret = connect(clientfd[i], (struct sockaddr *) &address, sizeof(address));
	  fail_unless (ret == 0, "connect failed");
	  
	  afd[i] = accept(serverfd, NULL, NULL);
	  fail_unless(afd[i] >= 0, "accept failed");

	  ret = send(clientfd[i], msg, strlen(msg) + 1, 0);
	  fail_unless (ret == strlen(msg) + 1, "send failed");

	  memset(buffer, 0, 1024);

	  ret = recv(afd[i], buffer, 1024, 0);
	  fail_unless(ret == strlen(msg) + 1, "recv failed");
	  fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	  /* now test the other direction */

	  ret = send(afd[i], msg2, strlen(msg2) + 1, 0);
	  fail_unless (ret == strlen(msg2) + 1, "send failed");

	  memset(buffer, 0, 1024);

	  ret = recv(clientfd[i], buffer, 1024, 0);
	  fail_unless(ret == strlen(msg2) + 1, "recv failed");
	  fail_unless(memcmp(msg2, buffer, ret) == 0, "bad buffer");
  }

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	       ret = close(clientfd[i]);
	       fail_unless (ret == 0, "Close failed");

	       ret = close(afd[i]);
	       fail_unless (ret == 0, "Close failed");
  }
}
END_TEST

START_TEST (test_bus_abstract_send)
{
  int ret;
  int i;
  int serverfd;
  int clientfd[MAX_CLIENT];
  int afd[MAX_CLIENT];
  struct sockaddr_bus address;
  int addr_len = sizeof(address);
  char *msg = "hello world\n";
  char *msg2 = "good bye world\n";
  char buffer[1024];

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  address.sbus_path[0] = '\0';
  strcpy(address.sbus_path + 1, "/bus_abstract_send");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	  clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
	  fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
	  ret = connect(clientfd[i], (struct sockaddr *) &address, sizeof(address));
	  fail_unless (ret == 0, "connect failed");

	  afd[i] = accept(serverfd, NULL, NULL);
	  fail_unless(afd[i] >= 0, "accept failed");

	  ret = send(clientfd[i], msg, strlen(msg) + 1, 0);
	  fail_unless (ret == strlen(msg) + 1, "send failed");

	  memset(buffer, 0, 1024);

	  ret = recv(afd[i], buffer, 1024, 0);
	  fail_unless(ret == strlen(msg) + 1, "recv failed");
	  fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	  /* now test the other direction */

	  ret = send(afd[i], msg2, strlen(msg2) + 1, 0);
	  fail_unless (ret == strlen(msg2) + 1, "send failed");

	  memset(buffer, 0, 1024);

	  ret = recv(clientfd[i], buffer, 1024, 0);
	  fail_unless(ret == strlen(msg2) + 1, "recv failed");
	  fail_unless(memcmp(msg2, buffer, ret) == 0, "bad buffer");
  }

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");

  for (i = 0; i < MAX_CLIENT; i++) {
	       ret = close(clientfd[i]);
	       fail_unless (ret == 0, "Close failed");

	       ret = close(afd[i]);
	       fail_unless (ret == 0, "Close failed");
  }
}
END_TEST

START_TEST (test_bus_unicast)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char *msg2 = "good bye world\n";
	char buffer[1024];

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_unicast");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	/* First try to send without joining the bus */
	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[1], sizeof(struct sockaddr_bus));
	fail_unless (ret < 0, "send didn't failed %d", ret);

	/* clientfd[0] joins the bus */
	ret = setsockopt(afd[0], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
	fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

	/* trying to join an already joined peer should fail */
	ret = setsockopt(afd[0], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
	fail_unless (ret != 0, "BUS_JOIN_BUS setsockopt() did not failed %d", ret);

	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[1], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

	/*
	 * clientfd[1] hasn't joined the bus yet so the packet
	 * was default routed to the bus master
	 */
	ret = recv(afd[0], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	/* Now clientfd[1] joins the bus */
	ret = setsockopt(afd[1], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
	fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

	/* send to clientfd[1] again, this time it has to receive the packet */
	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[1], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

	ret = recv(clientfd[1], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_add_addr)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char *msg2 = "good bye world\n";
	char buffer[1024];
	struct bus_addr sbus_addr;

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_add_addr");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	sbus_addr.s_addr = 0x0001000000000001;

	ret = setsockopt(afd[0], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_ADD_ADDR setsockopt() failed %d", ret);

	/* two different peers cannot have the same address */
	ret = setsockopt(afd[1], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret != 0, "BUS_ADD_ADDR setsockopt() not failed %d", ret);

	raddress[0].sbus_addr.s_addr = sbus_addr.s_addr;

	ret = sendto(clientfd[1], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

	ret = recv(clientfd[0], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	/* remove the address */
	ret = setsockopt(afd[0], SOL_BUS, BUS_DEL_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_DEL_ADDR setsockopt() failed %d", ret);

	ret = sendto(clientfd[1], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	/* should receive the bus master since  client doesn't own the address */
	ret = recv(afd[1], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_send_default)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char *msg2 = "good bye world\n";
	char buffer[1024];

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_send_default");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	/* non used address, AF_BUS should route to the socket peer (afd[0]) */
	raddress[0].sbus_addr.s_addr = 0x0001000000000001;

	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

	ret = recv(afd[0], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	/* test the other way */

	ret = sendto(afd[0], msg2, strlen(msg2) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg2) + 1, "send other way failed %d", ret);

	memset(buffer, 0, 1024);

	/* the bus master has to receive the data itself */

	ret = recv(afd[0], buffer, 1024, 0);
	fail_unless(ret == strlen(msg2) + 1, "recv failed");
	fail_unless(memcmp(msg2, buffer, ret) == 0, "bad buffer");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_multicast)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char buffer[1024];
	struct bus_addr sbus_addr;

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_multicast");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	sbus_addr.s_addr = 0x0001000000000001;

	ret = setsockopt(afd[0], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_ADD_ADDR setsockopt() failed %d", ret);

	sbus_addr.s_addr = 0x0001000000000002;

	ret = setsockopt(afd[1], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_ADD_ADDR setsockopt() failed %d", ret);

	raddress[0].sbus_addr.s_addr = 0x0001ffffffffffff;

	ret = sendto(clientfd[2], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

        ret = recv(clientfd[0], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	memset(buffer, 0, 1024);

        ret = recv(clientfd[1], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	/* Now test to send a multicast message to all the peers */
	raddress[0].sbus_addr.s_addr = 0x0000ffffffffffff;

	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	for (i = 0; i < MAX_CLIENT; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == strlen(msg) + 1, "recv failed");
		fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");
	}

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_poll)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char buffer[1024];
	struct bus_addr sbus_addr;
	struct pollfd pfd[MAX_CLIENT];

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_poll");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);

		pfd[i].fd = clientfd[i];
		pfd[i].events = POLLIN;
		pfd[i].revents = 0;
	}

	raddress[0].sbus_addr.s_addr = 0x0000ffffffffffff;

	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	ret = poll(pfd, MAX_CLIENT, 10000);
	fail_unless (ret > 0, "poll() failed %d", ret);

	for(i = 0; i < MAX_CLIENT; i++)
		fail_unless (pfd[i].revents & POLLIN, "POLLIN failed for client %d revents %d", i, pfd[i].revents);

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_add_match_rule)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress_accept[MAX_CLIENT];
	struct sockaddr_bus raddress_getpeername[MAX_CLIENT];
	struct sockaddr_bus raddress_getsockname[MAX_CLIENT];
	char *msg;
	char *bad_msg;
	size_t msg_len;
	char buffer[1024];
	struct bus_addr sbus_addr;
	struct pollfd pfd[MAX_CLIENT];

	msg = "l\4\1\1'\0\0\0\253\1\0\0}\0\0\0\1\1o\0\31\0\0\0/org/gnome/SessionManager\0\0\0\0\0\0\0\2\1s\0\30\0\0\0org.gnome.SessionManager\0\0\0\0\0\0\0\0\3\1s\0\v\0\0\0ClientAdded\0\0\0\0\0\10\1g\0\1o\0\0\7\1s\0\4\0\0\0:1.0\0\0\0\0\"\0\0\0/org/gnome/SessionManager/Client16\0";
	bad_msg = "l\4\1\1'\0\0\0\253\1\0\0}\0\0\0\1\1o\0\31\0\0\0/org/gnome/SessionManager\0\0\0\0\0\0\0\2\1s\0\30\0\0\0org.gnome.SessionDestroy\0\0\0\0\0\0\0\0\3\1s\0\v\0\0\0ClientAdded\0\0\0\0\0\10\1g\0\1o\0\0\7\1s\0\4\0\0\0:1.0\0\0\0\0\"\0\0\0/org/gnome/SessionManager/Client16\0";
	msg_len = 183;

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_DBUS);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_add_match_rule");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		int addr_accept_len = sizeof(address);
		int addr_getsockname_len = sizeof(address);
		int addr_getpeername_len = sizeof(address);

		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, BUS_PROTO_DBUS);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd,
			(struct sockaddr *) &raddress_accept[i],
			&addr_accept_len);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress_getsockname[i], 0, sizeof(raddress_getsockname[i]));
		ret = getsockname(clientfd[i],
			(struct sockaddr *) &raddress_getsockname[i],
			&addr_getsockname_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);

		memset(&raddress_getpeername[i], 0, sizeof(raddress_getpeername[i]));
		ret = getpeername(afd[i],
			(struct sockaddr *) &raddress_getpeername[i],
			&addr_getpeername_len);
		fail_unless(ret == 0, "Get peer name failed %d", ret);

		fail_unless(addr_accept_len == sizeof(address));
		fail_unless(addr_getsockname_len == sizeof(address));
		fail_unless(addr_getpeername_len == sizeof(address));

		fail_unless(raddress_accept[i].sbus_family == AF_BUS);
		fail_unless(raddress_getsockname[i].sbus_family == AF_BUS);
		fail_unless(raddress_getpeername[i].sbus_family == AF_BUS);

		fail_unless(memcmp(&raddress_accept[i], &raddress_getsockname[i], sizeof(address)) == 0);
		fail_unless(memcmp(&raddress_accept[i], &raddress_getpeername[i], sizeof(address)) == 0);

	        add_match_rule(&raddress_accept[i], "type='method_call'",
                        NFDBUS_CMD_ADDMATCH, 1);
	        add_match_rule(&raddress_accept[i], "type='signal'",
                        NFDBUS_CMD_ADDMATCH, 1);
	        add_match_rule(&raddress_accept[i], "",
                        NFDBUS_CMD_REMOVEALLMATCH, 1);

		if (i % 2 == 0) { /* catch good messages */
	                add_match_rule(&raddress_accept[i],
				"interface='org.gnome.SessionManager'",
                                 NFDBUS_CMD_ADDMATCH, 1);
		} else { /* catch bad uppercase messages */
	                add_match_rule(&raddress_accept[i],
				"interface='org.gnome.SessionDestroy'",
                                NFDBUS_CMD_ADDMATCH, 1);
		}
	}

	fail_unless (1, "mark");
	address.sbus_addr.s_addr |= BUS_CLIENT_MASK;
	ret = sendto(clientfd[0], bad_msg, msg_len, 0,
		     (struct sockaddr *) &address, sizeof(struct sockaddr_bus));
	fail_unless (ret == msg_len, "send failed %d, %s", ret, strerror(errno));
	ret = sendto(clientfd[0], msg, msg_len, 0,
		     (struct sockaddr *) &address, sizeof(struct sockaddr_bus));
	fail_unless (ret == msg_len, "send failed %d, %s", ret, strerror(errno));

	for (i = 0; i < MAX_CLIENT; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == msg_len, "recv failed");
		if (i % 2 == 0)
			fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer on client %d", i);
		else
			fail_unless(memcmp(bad_msg, buffer, ret) == 0, "bad buffer");
	}
	fail_unless (1, "Mark after receive");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = recv(clientfd[i], buffer, 1024, MSG_DONTWAIT);
		fail_unless(ret == -1 && errno == EAGAIN,
		            "still something in the receiving queue: %d %s",
		             ret, strerror(errno));
	}

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_fd_pass)
{
	int ret;
	int i;
	int serverfd;
	int clientfd;
	int newfd;
	int received_fd = -1;
	int afd;
	int fd;
	struct sockaddr_bus address;
	struct sockaddr_bus raddress;
	int addr_len = sizeof(address);
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	struct iovec io = {"X", 1};
	char buf[CMSG_SPACE (sizeof(afd))];
	char buffer[1024];
	char cbuffer[1024];
	int pipefd[2];
	char *msg = "Hello";

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_fd_pass1");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	clientfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (clientfd != -1, "Seqpacket socket creation failed");
	ret = connect(clientfd, (struct sockaddr *) &address,
		      sizeof(address));
	fail_unless (ret == 0, "connect failed %d", ret);

	afd = accept(serverfd, NULL, NULL);
	fail_unless(afd >= 0, "accept failed");

	ret = send(clientfd, "X", 1, MSG_DONTWAIT);
	fail_unless(ret == 1, "Send clientfd failed");

	newfd = dup(afd);
	ret = close(afd);
	fail_unless(ret == 0, "Close failed");
	ret = send(newfd, "X", 1, MSG_DONTWAIT);
	fail_unless(ret == 1, "Send newfd failed");

	memset(buffer, 0, 1024);

	ret = recv(newfd, buffer, 1024, 0);
	fail_unless(ret == 1, "recv failed");

	memset(buffer, 0, 1024);

	ret = recv(clientfd, buffer, 1024, 0);
	fail_unless(ret == 1, "recv failed");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	ret = close(clientfd);
	fail_unless (ret == 0, "Close failed");

	ret = close(newfd);
	fail_unless (ret == 0, "Close failed");

	/* now test with fd passing using a pipe  */
	ret = pipe(pipefd);
	fail_unless (ret == 0, "Pipe creation failed");

	ret = write(pipefd[1], msg, sizeof(msg));
	fail_unless (ret == sizeof(msg), "Pipe write failed %d", ret);

	memset(buffer, 0, 1024);

	ret = read(pipefd[0], &buffer, 1024);
	fail_unless (ret == sizeof(msg), "Pipe read failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_fd_pass2");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	clientfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (clientfd != -1, "Seqpacket socket creation failed");
	ret = connect(clientfd, (struct sockaddr *) &address,
		      sizeof(address));
	fail_unless (ret == 0, "connect failed %d", ret);

	afd = accept(serverfd, NULL, NULL);
	fail_unless(afd >= 0, "accept failed");

	memset (&msgh, 0, sizeof (msgh));
	memset (buf, 0, sizeof (buf));

	msgh.msg_name = &address;
	msgh.msg_namelen = addr_len;
	msgh.msg_control = buf;
	msgh.msg_controllen = sizeof (buf);
	msgh.msg_iov = &io;
	msgh.msg_iovlen = 1;

	cmsg = CMSG_FIRSTHDR (&msgh);
	cmsg->cmsg_len = CMSG_LEN (sizeof (pipefd[1]));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	msgh.msg_controllen = cmsg->cmsg_len;

	memcpy (CMSG_DATA(cmsg), &pipefd[1], sizeof (pipefd[1]));
	ret = sendmsg (clientfd, &msgh, 0);
	fail_unless (ret > 0, "Send fd failed: %s", strerror(errno));

	/* pipefd[1] in flight. Close it locally. */
	ret = close(pipefd[1]);
	fail_unless(ret == 0, "Close failed");

	memset(&msgh, 0, sizeof(msgh));
	io.iov_base   = buffer;
	io.iov_len    = sizeof(buffer)-1;
	msgh.msg_iov    = &io;
	msgh.msg_iovlen = 1;
	msgh.msg_control = cbuffer;
	msgh.msg_controllen = sizeof(cbuffer);
	ret = recvmsg(afd, &msgh, 0);
	fail_unless(ret > 0, "Recvmsg failed");

	/* Loop over all control messages */
	cmsg = CMSG_FIRSTHDR(&msgh);
	while (cmsg != NULL) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type  == SCM_RIGHTS)
			received_fd =  *(int *) CMSG_DATA(cmsg);
		cmsg = CMSG_NXTHDR(&msgh, cmsg);
	}

	fail_unless(received_fd > 0, "Fd not received");

	ret = write(received_fd, msg, sizeof(msg));
	fail_unless (ret == sizeof(msg), "Pipe write failed %d", ret);

	memset(buffer, 0, 1024);

	ret = read(pipefd[0], &buffer, 1024);
	fail_unless (ret == sizeof(msg), "Pipe read failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	ret = close(received_fd);
	fail_unless(ret == 0, "Close received fd failed");

	ret = close(pipefd[0]);
	fail_unless (ret == 0, "Close failed");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	ret = close(clientfd);
	fail_unless (ret == 0, "Close failed");

	ret = close(afd);
	fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_bpf)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char *msg2 = "good bye world\n";
	char buffer[1024];
	struct sock_filter ins[512];
	struct sock_fprog filter;

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_bpf");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	memset(ins, 0, sizeof(ins));
	ins[0].code = BPF_RET|BPF_K;
	ins[0].k = 3;
	filter.len = 1;
	filter.filter = ins;

	ret = setsockopt(clientfd[1], SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
	fail_unless (ret == 0, "Attach socket filter failed");

	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[1], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	memset(buffer, 0, 1024);

	ret = recv(clientfd[1], buffer, 1024, 0);
	fail_unless(ret == 3, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

START_TEST (test_bus_increase_buff)
{
#define WMEM_SIZE 4096 * 52 * 2 * 4 /* wmem_default = 212992 (4096 * 52) */
#define SEND_ITER 4
  int ret;
  int i;
  int serverfd;
  int clientfd;
  int afd;
  struct sockaddr_bus address;
  int addr_len = sizeof(address);
  char sbuff[WMEM_SIZE / SEND_ITER];
  int sndbuf = WMEM_SIZE;
  int curbuf = 0;

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_max_queue");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  clientfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (clientfd != -1, "Seqpacket socket creation failed");
  ret = connect(clientfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless (ret == 0, "connect failed");

  afd = accept(serverfd, NULL, NULL);
  fail_unless(afd >= 0, "accept failed");

  memset(sbuff, 1, WMEM_SIZE / SEND_ITER);

  for (i = 0; i < SEND_ITER; i++) {
	  ret = send(clientfd, sbuff, WMEM_SIZE / SEND_ITER, MSG_DONTWAIT);
	  fail_unless (ret == -1, "send did not failed %d", ret);
  }

  ret = close(clientfd);
  fail_unless (ret == 0, "Close failed");

  ret = close(afd);
  fail_unless (ret == 0, "Close failed");

  ret = setsockopt(serverfd, SOL_BUS, BUS_SET_SENDBUF, &sndbuf, sizeof(int));
  fail_unless (ret == 0, "BUS_SET_SENDBUF setsockopt() failed %d", ret);

  clientfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (clientfd != -1, "Seqpacket socket creation failed");
  ret = connect(clientfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless (ret == 0, "connect failed");

  afd = accept(serverfd, NULL, NULL);
  fail_unless(afd >= 0, "accept failed");

  memset(sbuff, 1, WMEM_SIZE / SEND_ITER);

  for (i = 0; i < SEND_ITER; i++) {
	  ret = send(clientfd, sbuff, WMEM_SIZE / SEND_ITER, MSG_DONTWAIT);
	  fail_unless (ret == WMEM_SIZE / SEND_ITER, "send failed %d", ret);
  }

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_fill_receive_queue)
{
  int ret;
  int i;
  int serverfd;
  int clientfd;
  int afd;
  struct sockaddr_bus address;
  int addr_len = sizeof(address);
  char *msg = "Hello world";
  char buffer[1024];
  int send_iter = 11;
  int status;

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd != -1, "Seqpacket socket creation failed");

  memset(&address, 0, sizeof(address));

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_fill_receive_queue");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  clientfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (clientfd != -1, "Seqpacket socket creation failed");
  ret = connect(clientfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless (ret == 0, "connect failed");

  afd = accept(serverfd, NULL, NULL);
  fail_unless(afd >= 0, "accept failed");

  for (i = 0; i < send_iter; i++) {
	  ret = send(afd, msg, strlen(msg), 0);
	  fail_unless (ret == strlen(msg), "send failed %d", ret);
  }

  /* the client has its receive queue full so he can't send messages */
  ret = send(clientfd, msg, strlen(msg), MSG_DONTWAIT);
  fail_unless (ret == -1, "send did not failed %d", ret);

  if (fork() == 0) {
	  ret = send(clientfd, msg, strlen(msg), 0);
	  fail_unless (ret == strlen(msg), "send did not failed %d", ret);

	  exit(0);
  }

  memset(buffer, 0, 1024);

  /* consume a packet */
  ret = recv(clientfd, buffer, 1024, 0);
  fail_unless (ret == strlen(msg), "recv failed");

  ret = send(clientfd, msg, strlen(msg), 0);
  fail_unless (ret == strlen(msg), "send failed %d", ret);

  ret = wait(&status);
  fail_unless (ret > 0, "Wait failed");
  fail_unless (WIFEXITED(status), "Child didn't exit correctly: %s",
	       strerror(errno));
  fail_unless (WEXITSTATUS(status) == 0, "Child exit code is %d",
	       WEXITSTATUS(status));

  ret = close(clientfd);
  fail_unless (ret == 0, "Close failed");

  ret = close(afd);
  fail_unless (ret == 0, "Close failed");

  ret = close(serverfd);
  fail_unless (ret == 0, "Close failed");
}
END_TEST

/* In a multicast group, 2 senders A and B send messages concurrently to
 * recipients C and D */
START_TEST (test_bus_multicast_order)
{
  int serverfd;
  int clientfd[4];
  int afd[4];
  struct sockaddr_bus address;
  int addr_len = sizeof(address);
  int ret;
  int i;
  int status;

  serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
  fail_unless (serverfd > 0, "Socket failed");

  address.sbus_family = AF_BUS;
  strcpy(address.sbus_path, "/tmp/bus_multicast_order");
  ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
  fail_unless(ret == 0, "Bind failed %d", ret);

  ret = listen(serverfd, MAX_CLIENT);
  fail_unless(ret == 0, "Listen failed");

  for (i = 0; i < 4; i++) {
	  clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
	  fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
	  ret = connect(clientfd[i], (struct sockaddr *) &address,
			sizeof(address));
	  fail_unless (ret == 0, "connect failed %d", ret);

	  afd[i] = accept(serverfd, NULL, NULL);
	  fail_unless(afd[i] >= 0, "accept failed");

	  ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
	  fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);
  }

  /* Install socket filter on senders, they don't receive anything */
  for (i = 0 ; i < 2 ; i++) {
    struct sock_filter ins[512];
    struct sock_fprog filter;

    memset(ins, 0, sizeof(ins));
    ins[0].code = BPF_RET|BPF_K;
    ins[0].k = 0;
    filter.len = 1;
    filter.filter = ins;

    /* Attach a socket filter to drop everything */
    ret = setsockopt(clientfd[i], SOL_SOCKET, SO_ATTACH_FILTER, &filter,
		     sizeof(filter));
    fail_unless (ret == 0, "Attach socket filter failed");
  }

#define NB_ITER 5

  /* Receiver C and D */
  if (fork() == 0) {
      /* Sleep to fill the buffers */
      struct timespec ts;
      ts.tv_sec = 0;
      if (_i == 0)
        ts.tv_nsec = 500000000;
      else
        ts.tv_nsec = 1;
      nanosleep(&ts, NULL);

      for (i = 0 ; i < NB_ITER * 2 ; i++) {
        int retC, retD;
        char bufferC[1024];
        char bufferD[1024];
        retC = recv(clientfd[2], bufferC, 1024, 0);
        if (retC <= 1) exit(1);
        retD = recv(clientfd[3], bufferD, 1024, 0);
        if (retD <= 1) exit(2);
        if (strcmp(bufferC, bufferD) != 0) {
          printf("Received: '%s' '%s' at #%d #%d\n",
                    bufferC, bufferD, _i + 1, i + 1);
          exit(3);
        }
      }
      exit(0);
  }

  address.sbus_addr.s_addr = 0x0000ffffffffffff;

  /* Sender A */
  if (fork() == 0) {
      char bufferA[1024];
      for (i = 0 ; i < NB_ITER ; i++) {
        sprintf(bufferA, "A%d", i + 1);
	ret = sendto(clientfd[0], bufferA, strlen(bufferA) + 1, 0,
		     (struct sockaddr *) &address,
		     sizeof(struct sockaddr_bus));
	if (ret < 1) exit(4);
      }
      exit(0);
  }

  /* Sender B */
  if (fork() == 0) {
	  char bufferB[1024];
	  for (i = 0 ; i < NB_ITER ; i++) {
	      sprintf(bufferB, "B%d", i + 1);
	ret = sendto(clientfd[1], bufferB, strlen(bufferB) + 1, 0,
		     (struct sockaddr *) &address,
		     sizeof(struct sockaddr_bus));
	if (ret < 1) exit(5);
      }
      exit(0);
  }

  for (i = 0 ; i < 3 ; i++) {
    ret = wait(&status);
    fail_unless (ret > 0, "Wait failed");
    fail_unless (WIFEXITED(status), "Child didn't exit correctly: %s", strerror(errno));
    fail_unless (WEXITSTATUS(status) == 0, "Child exit code is %d", WEXITSTATUS(status));
  }

  for (i = 0 ; i < 4 ; i++) {
    ret = close(clientfd[i]);
    fail_unless (ret == 0, "Close failed");

    ret = close(afd[i]);
    fail_unless (ret == 0, "Close failed");
  }

    ret = close(serverfd);
    fail_unless (ret == 0, "Close failed");
}
END_TEST

START_TEST (test_bus_eavesdrop)
{
	int ret;
	int i;
	int serverfd;
	int clientfd[MAX_CLIENT];
	int afd[MAX_CLIENT];
	struct sockaddr_bus address;
	struct sockaddr_bus raddress[MAX_CLIENT];
	int addr_len = sizeof(address);
	char *msg = "hello world\n";
	char buffer[1024];
	struct bus_addr sbus_addr;

	serverfd = socket(AF_BUS, SOCK_SEQPACKET, 0);
	fail_unless (serverfd != -1, "Seqpacket socket creation failed");

	memset(&address, 0, sizeof(address));

	address.sbus_family = AF_BUS;
	strcpy(address.sbus_path, "/tmp/bus_eavesdrop");
	ret = bind(serverfd, (struct sockaddr *) &address, sizeof(address));
	fail_unless(ret == 0, "Bind failed %d", ret);

	ret = listen(serverfd, MAX_CLIENT);
	fail_unless(ret == 0, "Listen failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		clientfd[i] = socket(AF_BUS, SOCK_SEQPACKET, 0);
		fail_unless (clientfd[i] != -1, "Seqpacket socket creation failed");
		ret = connect(clientfd[i], (struct sockaddr *) &address,
			      sizeof(address));
		fail_unless (ret == 0, "connect failed %d", ret);

		afd[i] = accept(serverfd, NULL, NULL);
		fail_unless(afd[i] >= 0, "accept failed");

		ret = setsockopt(afd[i], SOL_BUS, BUS_JOIN_BUS, NULL, 0);
		fail_unless (ret == 0, "BUS_JOIN_BUS setsockopt() failed %d", ret);

		memset(&raddress[i], 0, sizeof(raddress[i]));
		ret = getsockname(clientfd[i], (struct sockaddr *) &raddress[i], &addr_len);
		fail_unless(ret == 0, "Get sock name failed %d", ret);
	}

	/* add some peers to 0x0001 multicast prefix */

	sbus_addr.s_addr = 0x0001000000000001;

	ret = setsockopt(afd[0], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_ADD_ADDR setsockopt() failed %d", ret);

	sbus_addr.s_addr = 0x0001000000000002;

	ret = setsockopt(afd[1], SOL_BUS, BUS_ADD_ADDR, &sbus_addr,
			 sizeof(struct bus_addr));
	fail_unless (ret == 0, "BUS_ADD_ADDR setsockopt() failed %d", ret);

	/* set eavesdrop for one peer */

	ret = setsockopt(afd[2], SOL_BUS, BUS_SET_EAVESDROP, NULL, 0);
	fail_unless (ret == 0, "BUS_SET_EAVESDROP setsockopt() failed %d", ret);

	raddress[0].sbus_addr.s_addr = 0x0001ffffffffffff;

	ret = sendto(clientfd[4], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	/* the peers on the multicast group and the eavesdropper should receive */

	for (i = 0; i < 3; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == strlen(msg) + 1, "recv failed");
		fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");
	}

	/* the others peers shouldn't receive the message */
	for (i = 3; i < MAX_CLIENT; i++) {
		ret = recv(clientfd[i], buffer, 1024, MSG_DONTWAIT);
		fail_unless(ret == -1 && errno == EAGAIN,
		            "still something in the receiving queue: %d %s",
		             ret, strerror(errno));
	}

	/* set eavesdrop for another peer */

	ret = setsockopt(afd[3], SOL_BUS, BUS_SET_EAVESDROP, NULL, 0);
	fail_unless (ret == 0, "BUS_SET_EAVESDROP setsockopt() failed %d", ret);

	ret = sendto(clientfd[4], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[0], sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	/* both eavesdroppers should receive */

	for (i = 0; i < 4; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == strlen(msg) + 1, "recv failed");
		fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");
	}

	/* the others peers shouldn't receive the message */
	for (i = 4; i < MAX_CLIENT; i++) {
		ret = recv(clientfd[i], buffer, 1024, MSG_DONTWAIT);
		fail_unless(ret == -1 && errno == EAGAIN,
		            "still something in the receiving queue: %d %s",
		             ret, strerror(errno));
	}

	/* send an unicast message to the accepted socket */

	ret = send(clientfd[4], msg, strlen(msg) + 1, 0);
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	/* again both eavesdroppers should receive */

	memset(buffer, 0, 1024);

	ret = recv(afd[4], buffer, 1024, 0);
	fail_unless(ret == strlen(msg) + 1, "recv failed");
	fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");

	for (i = 2; i < 4; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == strlen(msg) + 1, "recv failed");
		fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");
	}

	/* the others peers shouldn't receive the message */
	for (i = 4; i < MAX_CLIENT; i++) {
		ret = recv(clientfd[i], buffer, 1024, MSG_DONTWAIT);
		fail_unless(ret == -1 && errno == EAGAIN,
		            "still something in the receiving queue: %d %s",
		             ret, strerror(errno));
	}

	/* send an unicast message to other peer */
	ret = sendto(clientfd[0], msg, strlen(msg) + 1, 0,
		     (struct sockaddr *) &raddress[4],
		     sizeof(struct sockaddr_bus));
	fail_unless (ret == strlen(msg) + 1, "send failed %d", ret);

	/* the other peer and the eavesdropper should receive */

	for (i = 2; i < 5; i++) {
		memset(buffer, 0, 1024);

		ret = recv(clientfd[i], buffer, 1024, 0);
		fail_unless(ret == strlen(msg) + 1, "recv failed");
		fail_unless(memcmp(msg, buffer, ret) == 0, "bad buffer");
	}

	/* the others peers shouldn't receive the message */
	for (i = 5; i < MAX_CLIENT; i++) {
		ret = recv(clientfd[i], buffer, 1024, MSG_DONTWAIT);
		fail_unless(ret == -1 && errno == EAGAIN,
		            "still something in the receiving queue: %d %s",
		             ret, strerror(errno));
	}

	ret = close(serverfd);
	fail_unless (ret == 0, "Close failed");

	for (i = 0; i < MAX_CLIENT; i++) {
		ret = close(clientfd[i]);
		fail_unless (ret == 0, "Close failed");

		ret = close(afd[i]);
		fail_unless (ret == 0, "Close failed");
	}
}
END_TEST

Suite *
afbus_suite (void)
{
  Suite *s = suite_create("AF_BUS");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_set_timeout(tc_core, 30);

  tcase_add_test (tc_core, test_bus_create);
  tcase_add_test (tc_core, test_bus_bind);
  tcase_add_test (tc_core, test_bus_getsockname);
  tcase_add_test (tc_core, test_bus_connect);
  tcase_add_test (tc_core, test_bus_connect_sub_protocol);
  tcase_add_test (tc_core, test_bus_connect_getsockname);
  tcase_add_test (tc_core, test_bus_send);
  tcase_add_test (tc_core, test_bus_unicast);
  tcase_add_test (tc_core, test_bus_add_addr);
  tcase_add_test (tc_core, test_bus_send_default);
  tcase_add_test (tc_core, test_bus_multicast);
  tcase_add_test (tc_core, test_bus_abstract_send);
  tcase_add_test (tc_core, test_bus_poll);
  tcase_add_test (tc_core, test_bus_add_match_rule);
  tcase_add_test (tc_core, test_bus_fd_pass);
  tcase_add_test (tc_core, test_bus_bpf);
  tcase_add_test (tc_core, test_bus_increase_buff);
  tcase_add_test (tc_core, test_bus_fill_receive_queue);
  tcase_add_test (tc_core, test_bus_multicast_order);
  tcase_add_test (tc_core, test_bus_eavesdrop);

  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
  int number_failed;

  system("rm /tmp/bus_*");

  Suite *s = afbus_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

