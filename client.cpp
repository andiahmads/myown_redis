#include <cerrno>
#include <cstdio>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET; // alamat untuk socket AF_INET protokol IPv4
                             // (Internet Protocol versi 4).

  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(
      INADDR_LOOPBACK); // menggunakan alamat loopback (biasanya 127.0.0.1)

  // digunakan dalam pemrograman jaringan untuk menghubungkan sebuah socket
  // (yang direpresentasikan oleh file descriptor fd) ke alamat tertentu (addr),
  // yang biasanya mengacu pada server atau peer dalam komunikasi jaringan.
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("connect()");
  }

  char msg[] = "hello from client";
  write(fd, msg, strlen(msg));

  char rbuf[64] = {};
  ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    die("read");
  }

  printf("server says: %s\n", rbuf);
  close(fd);
  return 0;
}
