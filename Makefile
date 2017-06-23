SRCDIR=src
BINDIR=bin
DEPDIR=.depend
HOOKSDIR=hooks
BININCDIR=$(BINDIR)/include

all: nuclear-compositor

### CC Vars

CCSRCDIR=$(SRCDIR)
CCBINDIR=$(BINDIR)
CCDEPDIR=$(DEPDIR)

CC=gcc
CFLAGS=-g -Wall -fdiagnostics-color -I include/linmath.h
LDFLAGS=-DOSVR_RM_USE_OPENGLES20=1 -lwayland-server -lX11 -lXcursor -lX11-xcb -lGLESv2 -lEGL -lxkbcommon-x11 -lxkbcommon -lm -losvrClientKit -losvrRenderManager
CCBIN=nuclear-compositor

CCSRCS = $(shell find $(CCSRCDIR) -type f -name '*.c')
CCBINS = $(patsubst $(CCSRCDIR)/%.c, $(CCBINDIR)/%.o, $(CCSRCS))
CCDEPS = $(patsubst $(CCSRCDIR)/%.c, $(CCDEPDIR)/%.d, $(CCSRCS))

-include $(CCDEPS)

$(CCBIN): $(CCBINS)
	$(CC) $(LDFLAGS) -o $(CCBIN) $(CCBINS)

# C Source => Objects
$(CCBINDIR)/%.o: $(CCSRCDIR)/%.c
	@mkdir -p "$(@D)"
	@mkdir -p "$(patsubst $(CCBINDIR)%,$(CCDEPDIR)%, $(@D))"
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -DDEP -MM $< -MT $@ -MF $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@))
	@cp -f $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@)) $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@)).tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@)).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@))
	@rm -f $(patsubst $(CCBINDIR)/%.o,$(CCDEPDIR)/%.d, $(@)).tmp

### Main

clean:
	@rm -vrf $(BINDIR)
	@rm -vf $(CCBIN) $(RESBIN)
