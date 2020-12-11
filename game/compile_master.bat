@echo off

:: Options
:: audio
:: code
:: run

::echo %audio%
::echo %code%
::echo %run%

set name="main"

set path=%path%;..\bin\

set CC65_HOME=..\

IF DEFINED audio (
	MUSIC\text2data.exe MUSIC\songs.txt -ca65
	MUSIC\nsf2data.exe MUSIC\sounds.nsf -ca65
)

IF DEFINED code (
	REM -g adds debug information, but the end result .nes file is not
	REM affected, so leave it in all the time.
	cc65 -g -Oirs -D VS_SYS_ENABLED=1 -D VS_SRAM_ENABLED=0 %name%.c --add-source
	ca65 -D VS_SYS_ENABLED=1 -D VS_SRAM_ENABLED=0 crt0.s
	ca65 %name%.s -g

	REM -dbgfile does not impact the resulting .nes file.
	ld65 -C nrom_32k_vert.cfg --dbgfile %name%.dbg -o %name%.nes crt0.o %name%.o nes.lib -Ln labels.txt

	del *.o

	move /Y %name%.dbg BUILD\ 
	move /Y labels.txt BUILD\ 
	move /Y %name%.s BUILD\ 
	move /Y %name%.nes BUILD\ 
)

if DEFINED run (
	BUILD\%name%.nes
)

::set audio
::set code
::set run