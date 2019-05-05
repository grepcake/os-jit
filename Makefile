.PHONY: clean run

jit: jit.c
	cc -o $@ $^

run: jit
	./jit $(STRING)

clean:
	$(RM) jit
