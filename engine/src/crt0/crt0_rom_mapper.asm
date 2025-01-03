; ____________________________
; ██▀███▀██▀▀▀▀▀▀▀█▀▀█        │   ▄▄       ▄▄   ▄▄
; ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │  ██ ▀ ██▄▀ ██▀ █ ██
; █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │  ▀█▄▀ ██   ▀█▄ ▀▄█▀
; ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘
;  by Guillaume 'Aoineko' Blanchard under CC-BY-AS license
;──────────────────────────────────────────────────────────────────────────────
; crt0 header for mapped ROM program
;──────────────────────────────────────────────────────────────────────────────
; Code address: 0x4010 (right after the header)
; Data address: 0xC000
;──────────────────────────────────────────────────────────────────────────────
.z80
.module	crt0

.include "defines.asm"
.include "macros.asm"

;==============================================================================
; ROM
;==============================================================================
	.area	_HEADER (ABS)
	.org	START_ADDR

;------------------------------------------------------------------------------
; Header
	.area	_HOME
	.area	_CODE

_g_FirstAddr::
_g_HeaderAddr::
	; ROM header
	ROM_HEADER crt0_init

;------------------------------------------------------------------------------
; Initialization code
crt0_init:
	; Delay startup using STKE hook (if ROM_DELAY is 1)
	DELAY_BOOT

	di
	; Set stack address at the top of free memory
	ld		sp, (HIMEM)

	; Initialize heap address
	ld		hl, #s__HEAP
	ld		(#_g_HeapStartAddress), hl

	ld 		a,(MSXVER)
	ld		(#_g_MSXVersion),a

	ld		hl,#0x0180
	ld		a,(hl)
	cp		#0xc3
	jr		nz,check_wsx
	ld		a,#0x81
	call	0x0180
	jr		end_turbo_switch	
check_wsx:
	ld		bc,#0x0840
	out		(c),b
	in		a,(c)
	cpl
	cp	    b
	jr		nz,check_exp
	in		a,(#0x41)
	and	    #0xfe
	out		(#0x41),a
	jr		end_turbo_switch
check_exp:
	ld		de,#0xfaa7
	ld		hl,(#0x1387)
	rst		0x20
	jr		nz,end_turbo_switch
	ld		de,#0x1393
	ld		hl,(#0x1389)
	rst		0x20
	jr		nz,end_turbo_switch
	ld		de,#0xb6db
	ld		hl,(#0x1393)
	rst		0x20
	jr		nz,end_turbo_switch
	in		a,(#0xb6)
	or		#0x80
	out		(#0xb6),a

end_turbo_switch:

	ld 		a,(VDPREA)
	ld 		(#_g_VDPRead),a

	ld 		a,(VDPWRT)
	ld 		(#_g_VDPWrite),a

	di

	; Set Page 2 slot equal to Page 1 slot
	INIT_P1_TO_P2
	
	; Install ISR in RAM (if ROM_RAMISR is set)
	INSTALL_RAM_ISR

	; Initialize ROM mapper segment
	INIT_MAPPER
ctr0_init_globals::
	; Initialize globals
	INIT_GLOBALS

crt0_start:
	; start main() function
	ei
	call	_main
	rst		0

	; Add banked call trampoline
	SUPPORT_BANKED_CALL

;------------------------------------------------------------------------------
; Ordering of segments for the linker
	.area	_RODATA
	.area	_INITIALIZER 
	.area	_GSINIT
	.area	_GSFINAL
_g_LastAddr::

;==============================================================================
; RAM
;==============================================================================
	.area	_DATA

_g_MSXVersion::
	.ds		1

_g_VDPRead::
	.ds		1
_g_VDPWrite::
	.ds		1

_g_HeapStartAddress::
	.ds		2

	.if ROM_RAMISR
		ALLOC_ROMINFO
	.endif

	ALLOC_MAPPER

;------------------------------------------------------------------------------
; Ordering of segments for the linker
	.area	_INITIALIZED
	.area	_BSEG
	.area	_BSS
	.area	_HEAP