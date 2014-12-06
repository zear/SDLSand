CXX		:= /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
STRIP		:= /opt/gcw0-toolchain/usr/bin/mipsel-linux-strip
SYSROOT		:= $(shell $(CXX) --print-sysroot)
CXXFLAGS	:= $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
LDFLAGS		:= $(shell $(SYSROOT)/usr/bin/sdl-config --libs) -lm
RELEASEDIR	:= release
TARGET		:= sdlsand.elf
OBJS		:= main.o CmdLine.o

ifdef DEBUG
	CXXFLAGS	+= -ggdb
else
	CXXFLAGS	+= -O3
endif

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
ifndef DEBUG
	$(STRIP) $(TARGET)
endif

opk: $(TARGET)
	mkdir -p		$(RELEASEDIR)
	cp $(TARGET)		$(RELEASEDIR)
	cp default.gcw0.desktop	$(RELEASEDIR)
	cp sdlsand.png		$(RELEASEDIR)
	cp readme.txt		$(RELEASEDIR)
	mksquashfs		$(RELEASEDIR) sdlsand.opk -all-root -noappend -no-exports -no-xattrs

clean:
	rm -Rf $(TARGET) $(OBJS) $(RELEASEDIR)


