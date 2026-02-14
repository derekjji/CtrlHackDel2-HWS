# Makefile for Healthcare Data Backend
# Requires: libcurl, OpenSSL

CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lcurl -lcrypto

# Detect OS
ifeq ($(OS),Windows_NT)
    RM = del /Q
    EXE = .exe
else
    RM = rm -f
    EXE =
endif

# Targets
all: healthcare_backend do_backend encrypt_util

healthcare_backend: healthcare_backend.c s3_auth.c s3_auth.h
	$(CC) $(CFLAGS) healthcare_backend.c s3_auth.c -o healthcare_backend$(EXE) $(LIBS)

do_backend: digitalocean_backend.c s3_auth.c s3_auth.h
	$(CC) $(CFLAGS) digitalocean_backend.c s3_auth.c -o do_backend$(EXE) $(LIBS)

encrypt_util: encrypt_util.c
	$(CC) $(CFLAGS) encrypt_util.c -o encrypt_util$(EXE) -lcrypto

clean:
	$(RM) healthcare_backend$(EXE) do_backend$(EXE) encrypt_util$(EXE)
	$(RM) *.o

test_encrypt:
	./encrypt_util$(EXE)

.PHONY: all clean test_encrypt
