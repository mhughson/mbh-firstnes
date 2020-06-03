@echo off

set name="main"

set path=%path%;..\bin\

set CC65_HOME=..\

MUSIC\text2data.exe MUSIC\songs.txt -ca65
MUSIC\nsf2data.exe MUSIC\sounds.nsf -ca65

cc65 -Oirs %name%.c --add-source
ca65 crt0.s
ca65 %name%.s -g

ld65 -C nrom_32k_vert.cfg -o %name%.nes crt0.o %name%.o nes.lib -Ln labels.txt

del *.o

move /Y labels.txt BUILD\ 
move /Y %name%.s BUILD\ 
move /Y %name%.nes BUILD\ 

BUILD\%name%.nes
