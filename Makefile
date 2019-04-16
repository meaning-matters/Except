SOURCES		= Except.c Lifo.c Assert.c Alloc.c Hash.c List.c
OBJECTS		= $(SOURCES:.c=.o)
PROGRAM		= t

CPPFLAGS		= -DEXCEPT_MT_SHARED -DEXCEPT_THREAD_POSIX -DDEBUG #-DEXCEPT_DEBUG
WARNINGS		= -Wno-incompatible-pointer-types -Wno-unused-value -Wno-return-type -Wno-unused-value -Wno-null-dereference
CFLAGS		= -g -lpthread $(WARNINGS) #-fvolatile

EX		= except

.KEEP_STATE:

CC	= cc

%.o: %.c
	$(COMPILE.c) -o $@ $<

default: $(PROGRAM) th

$(OBJECTS) Test.o: Makefile $(SOURCES:.c=.h)

$(PROGRAM): $(OBJECTS) Test.o
	$(LINK.c) -o $(PROGRAM) $(OBJECTS) Test.o

th: $(OBJECTS) thread.c
	$(CC) thread.c -o th $(CPPFLAGS) $(CFLAGS) $(OBJECTS)

clean:
	$(RM) $(OBJECTS) *.o *% core *.class $(PROGRAM) th *~ *.uu *.jar *.tar article/*%

release: clean
	cd ..; jar cvf $(EX).jar $(SOURCES:%.c=$(EX)/%.c) $(SOURCES:%.c=$(EX)/%.h) $(EX)/Test.c $(EX)/README $(EX)/thread.c $(EX)/Makefile
	mv ../$(EX).jar .
	uuencode < $(EX).jar $(EX).jar > $(EX).jar.uu
	rm $(EX).jar
	cd ..; tar cvf $(EX).tar $(SOURCES:%.c=$(EX)/%.c) $(SOURCES:%.c=$(EX)/%.h) $(EX)/Test.c $(EX)/README $(EX)/thread.c $(EX)/Makefile
	mv ../$(EX).tar .
	gzip $(EX).tar
	uuencode < $(EX).tar.gz $(EX).tgz > $(EX).tgz.uu
	rm $(EX).tar.gz

