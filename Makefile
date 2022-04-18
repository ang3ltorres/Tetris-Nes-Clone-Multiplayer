BIN = main.exe

CPP = g++
CPPFLAGS = -std=c++11 -Wall -pedantic -O3 -s -Iinclude -include raylib.h -include new
LFLAGS = -Llib -lraylib

SRCDIR = src
OBJDIR = obj
BINDIR = bin

all: $(BINDIR)/$(BIN)
	$(BINDIR)/$(BIN)

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

-include $(OBJECTS:.o=.d)

$(BINDIR)/$(BIN): $(OBJECTS)
	$(CPP) $(OBJECTS) -o $@ $(LFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@IF NOT EXIST "$(OBJDIR)" MKDIR "$(OBJDIR)"
	$(CPP) $< -c -o $@ $(CPPFLAGS) -MMD -MF $(@:.o=.d)

clean:
	rm -f -r $(OBJECTS) $(OBJECTS:.o=.d) $(BINDIR)/$(BIN)
