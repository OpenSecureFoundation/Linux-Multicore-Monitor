#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 8080

static enum MHD_Result handle_affinity(struct MHD_Connection *conn) {
    const char *p = MHD_lookup_connection_value(conn, MHD_GET_ARGUMENT_KIND, "pid");
    const char *c = MHD_lookup_connection_value(conn, MHD_GET_ARGUMENT_KIND, "cpu");
    if (p && c) {
        cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(atoi(c), &mask);
        if (sched_setaffinity(atoi(p), sizeof(cpu_set_t), &mask) == 0) {
            struct MHD_Response *r = MHD_create_response_from_buffer(2, "OK", MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(r, "Access-Control-Allow-Origin", "*");
            return MHD_queue_response(conn, 200, r);
        }
    }
    return MHD_NO;
}

static enum MHD_Result send_stats(void *cls, struct MHD_Connection *conn, const char *url, const char *m, const char *v, const char *d, size_t *s, void **c_cls) {
    if (strcmp(url, "/set_affinity") == 0) return handle_affinity(conn);
    int fd = open("/dev/core_monitor", O_RDONLY);
    if (fd < 0) return MHD_NO;
    char *buf = malloc(131072); ssize_t n = read(fd, buf, 131072); close(fd);
    struct MHD_Response *res = MHD_create_response_from_buffer(n, buf, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(res, "Content-Type", "application/json");
    MHD_add_response_header(res, "Access-Control-Allow-Origin", "*");
    return MHD_queue_response(conn, 200, res);
}

int main() {
    struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &send_stats, NULL, MHD_OPTION_END);
    if (!d) return 1;
    printf("CORE MONITOR ENGINE : http://localhost:8080\n");
    while(1) pause();
    return 0;
}
