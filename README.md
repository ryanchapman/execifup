# execifup - Execute commands based on uptime

This is useful for things like healthchecks, where you may only want to log at `ERROR` level if the machine
has been up for a few minutes (to allow for services to start).  You can still log at `INFO` level if the 
machine hasn't been up for long.

This only works on Linux currently.  It wouldn't be hard to add support for other flavors of UNIX and maybe even Windows.
Please open a PR if you add support for a different operating system.

## Usage

```
execifup <n_secs> "<cmd_if_up_for_n_secs>" "[cmd_if_not_up_for_n_secs]"

n_secs                    required   number of seconds machine must be up for
cmd_if_up_for_n_secs      required   command to run if the machine has been up n seconds
cmd_if_not_up_for_n_secs  optional   command to run if the machine has not yet been up n seconds
```

## Examples

This is an example where a docker or AWS ECS style healthcheck would log at `ERROR` level only after the 
container has been up for 60 seconds.  If the container has not been up for 60 seconds, it will still log
that the healthcheck failed, but at `INFO` level.  This lets you do things like build a log filter that will only
forward those `ERROR` healthcheck logs to Slack.  You may still want to know anytime a healthcheck fails, but only
keeping it in Cloudwatch for later review if needed.


```
execifup 60 "(curl localhost/healthz || echo 'ERROR: healthcheck failed')" "(curl localhost/healthz || echo 'INFO: healthcheck failed')" &>/proc/1/fd/1
```

Same example, but don't do anything if the machine hasn't been up for 60 seconds yet
```
execifup 60 "(curl localhost/healthz || echo 'ERROR: healthcheck failed')" &>/proc/1/fd/1
```

## Building

```
make
```

### Cleaning

```
make clean
```

### Test

To run all tests:
```
make test
```

## Debugging

You can set the environment variable `LOG_DEBUG=1` if you'd like the program to print debug logs.

```
root@c815bed12f7c:/src# LOG_DEBUG=1 ./execifup 5555555555 "(false || echo ERROR)" "(false || echo INFO)"
DEBUG: this machine has been up for 12008 seconds
DEBUG: secs=5555555555, strtol(secs)=5555555555
DEBUG: cmd_if_up=(false || echo ERROR)
DEBUG: cmd_if_not_up=(false || echo INFO)
DEBUG: machine has NOT been up at least 5555555555 seconds (actually up 12008 secs)
DEBUG: executing "(false || echo INFO)"
INFO

root@c815bed12f7c:/src# LOG_DEBUG=1 ./execifup 60 "(false || echo ERROR)" "(false || echo INFO)"
DEBUG: this machine has been up for 12015 seconds
DEBUG: secs=60, strtol(secs)=60
DEBUG: cmd_if_up=(false || echo ERROR)
DEBUG: cmd_if_not_up=(false || echo INFO)
DEBUG: machine has been up at least 60 seconds (actually up 12015 secs)
DEBUG: executing "(false || echo ERROR)"
ERROR
```

## Custom shell

By default, `execifup` will use `/bin/sh` as the shell which your commands are passed to.  If you would like to
use a different shell, set the environment variable `SHELL`.

For example, to use bash, run something like
```
SHELL=/bin/bash execifup 60 "(curl localhost/healthz || echo 'ERROR: healthcheck failed')" &>/proc/1/fd/1
```

## Detecting if a shutdown is in progress

If a shutdown is in progress, you probably don't want to run healthchecks, which could fail and log false alarms.

Have your entrypoint write the file `/tmp/shutdown_in_progress` when it gets `SIGTERM`, and `execifup` will
no longer log false alarms.

(I have a custom `dumb-init` that I may open source which has support for this.  You can also add it yourself fairly easily.)

If `execifup` sees that the file `/tmp/shutdown_in_progress` exists, it justs exits with return code `0`.

To turn off this behavior and have `execifup` ignore the presence of `/tmp/shutdown_in_progress`, set the environment
variable `IGNORE_SHUTDOWN_FILE=1`.  For example,

```
root@c815bed12f7c:/src# touch /tmp/shutdown_in_progress
root@c815bed12f7c:/src# IGNORE_SHUTDOWN_FILE=1 ./execifup 60 "false" "false"
```
would return 1

