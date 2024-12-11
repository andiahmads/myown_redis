#include <arpa/inet.h>
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

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static void do_something(int connfd) {
  char rbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  printf("client says %s\n", rbuf);

  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket");
  }

  int val = 1;
  // digunakan untuk mengatur opsi pada socket, dalam hal ini opsi SO_REUSEADDR.
  // Fungsi ini memungkinkan pengaturan tertentu pada socket yang dibuat dengan
  // socket().
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET; // alamat untuk socket AF_INET protokol IPv4
                             // (Internet Protocol versi 4).
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0

  // digunakan dalam pemrograman jaringan untuk menghubungkan sebuah socket
  //  (yang direpresentasikan oleh file descriptor fd) ke alamat tertentu
  //  (addr), yang biasanya mengacu pada server atau peer dalam komunikasi
  //  jaringan.
  int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  rv = listen(fd, SOMAXCONN); // * Maximum queue length specifiable by listen.
  if (rv) {
    die("listen()");
  }

  while (true) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
      continue; // error
    }

    do_something(connfd);
    close(connfd);
  }
  return 0;
}
