CC=gcc

execifup:
	$(CC) -Wall -static -o execifup execifup.c

dynamic:
	$(CC) -Wall -o execifup execifup.c

test: execifup cleantest
	LOG_DEBUG=1 ./execifup 60 "true" 2>&1 | grep DEBUG: || (echo "test fail: debug logging is not working" ; exit 1)
	./execifup 99999 "false" "true" || (echo "test fail: should have returned 0" ; exit 1)
	(./execifup 99999 "false" "false" && (echo "test fail: should have returned 1" ; exit 1) ; exit 0)
	./execifup 0 "true" || (echo "test fail: should have returned 0" ; exit 1)
	(./execifup 0 "false" && (echo "test fail: should have returned 1" ; exit 1) ; exit 0)
	SHELL=/bin/bash ./execifup 0 'echo $$BASH_VERSION' | grep '[0-9]' || (echo "test fail: custom SHELL is not working"; exit 1)
	touch /tmp/shutdown_in_progress && ./execifup 0 "false" || (echo "test fail: shutdown check is not working"; exit 1)
	touch /tmp/shutdown_in_progress && IGNORE_SHUTDOWN_FILE=1 ./execifup 0 "true" || (echo "test fail: ignore shutdown is not working"; exit 1)
	@echo "** all tests pass **"

.PHONY: clean

clean:
	rm -f execifup core

cleantest:
	rm -f /tmp/shutdown_in_progress

