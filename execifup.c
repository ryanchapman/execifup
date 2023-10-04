#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/sysinfo.h>

#define LOG_DEBUG(_fmt, ...)            \
({                                      \
  if (getenv("LOG_DEBUG") != NULL) {    \
    fprintf(stderr, "DEBUG: ");         \
    fprintf(stderr, _fmt, __VA_ARGS__); \
  }                                     \
})

void usage(char *argv0)
{
  fprintf(stderr, "usage: %s <n_secs> \"<cmd_if_up_for_n_secs>\" \"<cmd_if_not_up_for_n_secs>\"\n", argv0);
  fprintf(stderr, "\n");
  fprintf(stderr, " Run commands based on machine uptime.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, " Useful for things like healthchecks where you don\'t want to log error messages when the\n");
  fprintf(stderr, " machine or container is starting up.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, " n_secs                    number of seconds machine must be up for\n");
  fprintf(stderr, " cmd_if_up_for_n_secs      command to run if the machine has been up n seconds\n"); 
  fprintf(stderr, " cmd_if_not_up_for_n_secs  command to run if the machine has not yet been up n seconds\n"); 
  fprintf(stderr, "\n");
  fprintf(stderr, "examples:\n");
  fprintf(stderr, " %s 60 \"(curl localhost/healthz || echo 'ERROR: healthcheck failed')\" &>/proc/1/fd/1\n", argv0);
  fprintf(stderr, " If this machine/container has been up for at least a minute, hit http://localhost/healthz and if that\n");
  fprintf(stderr, " does not return 200, echo an error message.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, " %s 60 \"(curl localhost/healthz || echo 'ERROR: healthcheck failed')\" \"(curl localhost/healthz || echo 'INFO: healthcheck failed')\" &>/proc/1/fd/1\n", argv0);
  fprintf(stderr, " If this machine/container has been up for at least a minute, hit http://localhost/healthz and if that\n");
  fprintf(stderr, " does not return 200, echo an error message.  If the machine has not been up for at least 60 seconds and the\n");
  fprintf(stderr, " healthcheck is failing, log an INFO message.  This allows you to set up logging rules that ignore INFO healthcheck\n");
  fprintf(stderr, " failures.\n");
  fprintf(stderr, "\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 3) {
    usage(argv[0]);
  }

  char *ssecs = argv[1];
  long secs = strtol(ssecs, NULL, 10);
  if (secs <= LONG_MIN || secs >= LONG_MAX) {
    perror("strtol(secs)");
    exit(1);
  }

  char *cmd_if_up = argv[2];
  char *cmd_if_not_up = NULL;

  if (argc >= 4) {
    cmd_if_not_up = argv[3];
  }

  struct sysinfo si;
  if (sysinfo(&si) != 0) {
    perror("sysinfo()");
    exit(1); 
  }

  LOG_DEBUG("this machine has been up for %ld seconds\n", si.uptime);
  LOG_DEBUG("secs=%s, strtol(secs)=%ld\n", ssecs, secs);
  LOG_DEBUG("cmd_if_up=%s\n", cmd_if_up);
  LOG_DEBUG("cmd_if_not_up=%s\n", cmd_if_not_up);

  if (si.uptime > secs) {
    LOG_DEBUG("machine has been up at least %ld seconds (actually up %ld secs)\n", secs, si.uptime);
    LOG_DEBUG("executing \"%s\"\n", cmd_if_up);
    system(cmd_if_up);
  } else {
    LOG_DEBUG("machine has NOT been up at least %ld seconds (actually up %ld secs)\n", secs, si.uptime);
    if (cmd_if_not_up != NULL) {
      LOG_DEBUG("executing \"%s\"\n", cmd_if_not_up);
      system(cmd_if_not_up);
    }
  }
}
