#include <arpa/inet.h>
#include <assert.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
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

static int32_t read_full(int fd, char *buf, ssize_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1; // EOF
    }
    assert((ssize_t)rv <= n);
    n -= (ssize_t)n;
    buf += rv;
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, ssize_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);
    if (rv <= 0) {
      return -1; // EOF
    }
    assert((ssize_t)rv <= n);
    n -= (ssize_t)n;
    buf += rv;
  }
  return 0;
}

const ssize_t k_max_msg = 4096;
static int32_t one_request(int connfd) {
  // 4 bytes header
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  int32_t err = read_full(connfd, rbuf, 4);
  if (err) {
    if (errno == 0) {
      msg("EOF");
    } else {
      msg("read() error");
    }
    return err;
  }

  uint32_t len = 0;
  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    msg("to long err");
    return -1;
  }

  // request body
  err = read_full(connfd, &rbuf[4], len);
  if (err) {
    msg("read() errxxx");
    return err;
  }

  // do something
  rbuf[4 + len] = '\0';
  printf("client says: %s\n", &rbuf[4]);

  // reply using the same protocol
  const char reply[] = "world";
  char wbuf[4 + sizeof(reply)];
  len = (uint32_t)strlen(reply);
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], reply, len);
  return write_all(connfd, wbuf, 4 + len);
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

    while (true) {
      int32_t err = one_request(connfd);
      if (err) {
        break;
      }
    }

    // do_something(connfd);
    close(connfd);
  }
  return 0;
}
