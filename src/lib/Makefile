default: all

# disable builtin rules
.SUFFIXES:

.PRECIOUS: %.o %.d

CXXFLAGS ?= -O2 -Wall
CPPFLAGS ?= -O2
LDFLAGS ?= -O2

CXXFLAGS += -std=gnu++0x

CPPFLAGS += -Iinclude

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td

OBJS := \
	bytestream.o \
	console.o \
	embed.o \
	file.o \
	ini.o \
	path.o \
	stdioconsole.o \
	tokenize.o \
	utf.o \

DEPS=$(OBJS:.o=.d)

all: libnarf.a

%.o: %.cpp %.d
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS)
	mv -f $*.Td $*.d

%.d: ;

libnarf.a: $(OBJS)
	$(AR) rsc $@ $(OBJS)

clean:
	rm -f libnarf.a $(OBJS) $(DEPS)
