#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 8080

// Fonction sécurisée pour construire le JSON
static enum MHD_Result send_stats(void *cls, struct MHD_Connection *connection, const char *url,
                                  const char *method, const char *version, const char *upload_data,
                                  size_t *upload_data_size, void **con_cls) {
   
    int nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    // On alloue dynamiquement 512 Ko pour être totalement serein
    size_t capacity = 512 * 1024;
    char *json = malloc(capacity);
    if (!json) return MHD_NO;

    snprintf(json, capacity, "{\"cores\": [");

    for (int i = 0; i < nr_cpus; i++) {
        char *core_segment = malloc(64 * 1024); // 64 Ko par cœur
        if (!core_segment) { free(json); return MHD_NO; }

        sprintf(core_segment, "{\"id\": %d, \"procs\": \"", i);

        DIR *dir = opendir("/proc");
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir))) {
                if (isdigit(entry->d_name[0])) {
                    pid_t pid = atoi(entry->d_name);
                    cpu_set_t mask;
                    CPU_ZERO(&mask);
                    if (sched_getaffinity(pid, sizeof(cpu_set_t), &mask) == 0 && CPU_ISSET(i, &mask)) {
                        char path[64], comm[64];
                        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
                        FILE *f = fopen(path, "r");
                        if (f) {
                            if (fgets(comm, sizeof(comm), f)) {
                                comm[strcspn(comm, "\n")] = 0;
                                // On évite le débordement du segment
                                if (strlen(core_segment) + strlen(comm) + 5 < 60000) {
                                    strcat(core_segment, comm);
                                    strcat(core_segment, ", ");
                                }
                            }
                            fclose(f);
                        }
                    }
                }
            }
            closedir(dir);
        }
       
        // Nettoyage de la virgule finale
        size_t c_len = strlen(core_segment);
        if (c_len > 2 && core_segment[c_len-2] == ',') core_segment[c_len-2] = '\0';
       
        strcat(core_segment, "\"}");
        if (i < nr_cpus - 1) strcat(core_segment, ",");
       
        // Ajout au JSON principal
        strcat(json, core_segment);
        free(core_segment);
    }
    strcat(json, "]}");

    struct MHD_Response *res = MHD_create_response_from_buffer(strlen(json), (void*)json, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(res, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(res, "Content-Type", "application/json");
    return MHD_queue_response(connection, 200, res);
}

int main() {
    struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL, &send_stats, NULL, MHD_OPTION_END);
    if (!d) return 1;
    printf("AGAMA SHIELD ENGINE : http://localhost:8080\n");
    printf("Scan sécurisé en cours... Appuyez sur Entrée pour quitter.\n");
    getchar();
    MHD_stop_daemon(d);
    return 0;
}
