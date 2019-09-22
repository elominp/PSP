CC			= gcc
CXX			= g++
UPX			= upx
OBJCOPY		= objcopy
TAR			= tar
ECHO		= echo
STRIP		= strip
RM			= rm -rf

CFLAGS		= -Os -flto -std=gnu99 -Wall -Wextra -Werror -nostdlib -fno-builtin -m32
LDFLAGS		= -Os -flto -nostdlib -fno-builtin -m32
ARCH		= i386
MULTI_ARCH	= i386
FORMAT		= elf32
TAR_ARCHIVE = archive
TAR_OPTS	= -cvzf

SRCS		= bootstrap.c
OBJS		= $(SRCS:.c=.o)
PACKAGE		= hello_world
HEADER		= bootstrap.h

BIN			= foo

all: package $(OBJS)
	$(CC) $(OBJS) $(TAR_ARCHIVE).o -o $(BIN) $(LDFLAGS)
	$(STRIP) $(BIN)

clean:
	$(RM) $(OBJS) $(TAR_ARCHIVE) $(TAR_ARCHIVE).o .$(PACKAGE)

fclean:	clean
	$(RM) $(BIN)

re: fclean all

package:
	$(TAR) $(TAR_OPTS) $(TAR_ARCHIVE) $(PACKAGE)
	$(OBJCOPY) --input binary --output $(FORMAT)-$(ARCH) \
		--binary-architecture $(MULTI_ARCH) \
		--rename-section .data=.rodata,CONTENTS,ALLOC,LOAD,READONLY,DATA \
		$(TAR_ARCHIVE) $(TAR_ARCHIVE).o
	$(ECHO) "#ifndef BINARY_ARCHIVE_H" > $(HEADER)
	$(ECHO) "#define BINARY_ARCHIVE_H" >> $(HEADER)
	$(ECHO) "#define BINARY_ARCHIVE_NAME \"$(PACKAGE)\"" >> $(HEADER)
	$(ECHO) "#define BINARY_ARCHIVE_TEMP \".$(PACKAGE)\"" >> $(HEADER)
	$(ECHO) "extern char _binary_$(TAR_ARCHIVE)_start[];" >> $(HEADER)
	$(ECHO) "extern char _binary_$(TAR_ARCHIVE)_end[];" >> $(HEADER)
	$(ECHO) "extern char _binary_$(TAR_ARCHIVE)_size[];" >> $(HEADER)
	$(ECHO) "#endif" >> $(HEADER)

.PHONY: clean fclean all re package