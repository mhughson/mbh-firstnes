GAME=from_below
SONGS=MUSIC/songs.s
SOUNDS=MUSIC/sounds.s
CC=cc65
CA=ca65
LD=ld65
text2data=text2data
nsf2data=nsf2data
CFLAGS=-Oirs
DBGFLAGS=-g --add-source
ADBGFLAGS=-g
VSON=-D VS_SYS_ENABLED=1 -D VS_SRAM_ENABLED=0
VSOFF=-D VS_SYS_ENABLED=0 -D VS_SRAM_ENABLED=0
LDFLAGS=-C nrom_32k_vert.cfg --dbgfile main.dbg -Ln labels.txt

GIT_COMMIT:=$(if $(shell git log -1 2>/dev/null),_$(shell git log -1 --date=format:%Y_%m_%d --pretty=format:"%ad" 2>/dev/null),)

all: audio nes vssys
default: nes vssys

nes = BUILD/$(GAME)$(GIT_COMMIT).nes
vssys = BUILD/$(GAME)_vs$(GIT_COMMIT).nes

nes: $(nes)
vssys: $(vssys)

$(SONGS): 
	$(text2data) MUSIC/songs.txt -ca65

$(SOUNDS):
	$(nsf2data) MUSIC/sounds.nsf -ca65

AUDIO = $(SONGS) $(SOUNDS)

audio: $(AUDIO)

$(GAME).s:
	$(CC) -o $@ $(DBGFLAGS) $(CFLAGS) $(VSOFF) main.c

$(GAME)_vs.s:
	$(CC) -o $@ $(DBGFLAGS) $(CFLAGS) $(VSON) main.c

crt0.o: crt0.s $(AUDIO)
	$(CA) -o $@ $(VSOFF) crt0.s

crt0_vs.o: crt0.s $(AUDIO)
	$(CA) -o $@ $(VSON) crt0.s

$(GAME).o: $(GAME).s
	$(CA) -o $@ $(GAME).s $(ADBGFLAGS)

$(GAME)_vs.o: $(GAME)_vs.s
	$(CA) -o $@ from_below_vs.s $(ADBGFLAGS)

nes_obj = crt0.o $(GAME).o

vs_obj = crt0_vs.o $(GAME)_vs.o

BUILD/$(GAME)$(GIT_COMMIT).nes: $(nes_obj)
	-mkdir -p BUILD
	$(LD) -o $@ $(LDFLAGS) $(nes_obj) nes.lib

BUILD/$(GAME)_vs$(GIT_COMMIT).nes: $(vs_obj)
	-mkdir -p BUILD
	$(LD) -o $@ $(LDFLAGS) $(vs_obj) nes.lib


clean-audio:
	-rm $(SONGS) $(SOUNDS)

clean-nes:
	rm -f crt0.o $(GAME).o $(GAME).s *.dbg $(nes) labels.txt
	-rmdir BUILD || true

clean-vs:
	rm -f crt0_vs.o $(GAME)_vs.o $(GAME)_vs.s *.dbg $(vssys) labels.txt
	-rmdir BUILD || true

clean-obj-nes:
	rm -f crt0.o $(GAME).o $(GAME).s *.dbg labels.txt

clean-obj-vs:
	rm -f crt0_vs.o $(GAME)_vs.o $(GAME)_vs.s *.dbg labels.txt

clean-obj: clean-obj-nes clean-obj-vs

clean: clean-nes clean-vs

clean-all: clean-audio clean
