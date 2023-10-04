CC=gcc

execifup:
	$(CC) -static -o execifup execifup.c

dynamic:
	$(CC) -o execifup execifup.c

test: execifup
	LOG_DEBUG=1 ./execifup 60 "true" 2>&1 | grep DEBUG: || (echo "test fail: debug logging is not working" ; exit 1)
	./execifup 99999 "false" "true" || (echo "test fail: should have returned 0" ; exit 1)
	(./execifup 99999 "false" "false" && (echo "test fail: should have returned 1" ; exit 1) ; exit 0)
	./execifup 0 "true" || (echo "test fail: should have returned 0" ; exit 1)
	(./execifup 0 "false" && (echo "test fail: should have returned 1" ; exit 1) ; exit 0)
	@echo "all tests pass"	

.PHONY: clean

clean:
	rm -f execifup core
