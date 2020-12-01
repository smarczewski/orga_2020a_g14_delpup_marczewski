build:
	cargo build --release

test: build
	../test_recode.sh ./build/release/recode57

clean:
	cargo clean

.PHONY: build clean test
