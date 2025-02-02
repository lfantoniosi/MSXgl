;-----------------------------------------------------------------------------
; WYZ file merged for MSXgl on 09/11/2024 14:46:20,37 
;-----------------------------------------------------------------------------
.module wyz_jinj_med 
_jinj_med_Song:: .DW SONG00 

; Tabla de instrumentos
_jinj_med_Inst:: .DW PAUTA_0,PAUTA_1,PAUTA_2,PAUTA_3

; Tabla de efectos
_jinj_med_FX:: .DW 0

;Pautas (instrumentos)
;Instrumento 'Piano'
PAUTA_0:	.DB	14,0,13,0,12,0,12,0,11,0,11,0,10,0,10,0,9,0,9,0,8,0,8,0,7,0,7,0,6,0,6,0,7,0,7,0,6,0,6,0,5,0,5,0,129
;Instrumento 'PICC'
PAUTA_1:	.DB	4,0,6,0,7,0,7,0,6,0,5,0,4,0,4,0,3,0,129
;Instrumento 'Flauta vol bajo'
PAUTA_2:	.DB	7,0,9,0,10,0,10,0,10,0,10,0,9,0,7,0,7,0,7,0,7,0,7,0,7,0,7,0,7,0,6,0,6,0,6,0,6,0,6,0,6,0,140
;Instrumento 'Flauta'
PAUTA_3:	.DB	10,0,12,0,13,0,13,0,13,0,12,0,11,0,11,0,11,0,10,0,10,0,10,0,10,0,10,0,9,0,9,0,9,0,9,0,9,0,138

;Efectos

;Frecuencias para las notas
_jinj_med_Freq:: .DW 0,0
.DW 1711,1614,1524,1438,1358,1281,1210,1142,1078,1017
.DW 960,906,855,807,762,719,679,641,605,571
.DW 539,509,480,453,428,404,381,360,339,320
.DW 302,285,269,254,240,227,214,202,190,180
.DW 170,160,151,143,135,127,120,113,107,101
.DW 95,90,85,80,76,71,67,64,60,57
; Song data 
SONG00: 
; MSXbin 1.4.1 by Guillaume "Aoineko" Blanchard (2022) under CC BY-SA free license
; File generated on Sat Nov  9 14:46:20 2024
; Soure file: D:\Dev\Private\MSX\MSXgl\projects\samples\datasrc\wyz\jinj_med.mus
_jinj_med_Data::
.db 0x05,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x3F,0x00,0x00,0xA3
.db 0xE6,0x9E,0xE1,0xD9,0xDC,0xFE,0xFE,0xDE,0xFE,0xFE,0xA3,0xE6,0x9E,0xE1,0xD9,0xDC
.db 0xFE,0xFE,0xDE,0xFE,0xFE,0xA3,0xDE,0x9E,0xA1,0xA3,0xDC,0xDC,0x60,0xA0,0xBE,0xDE
.db 0xFE,0xA5,0xA3,0xDE,0x9E,0xA1,0xA3,0xDC,0xDC,0x60,0xA0,0xBE,0xDE,0xFE,0xA5,0xD7
.db 0xEF,0xD5,0xF1,0xDA,0xEF,0x9C,0xBE,0xAC,0xBE,0xA8,0xD7,0xEF,0xD5,0xF1,0xDA,0xEF
.db 0x9C,0xBE,0xAC,0xBE,0xA8,0x00,0x3F,0x00,0x00,0xD7,0xA3,0xA6,0xE1,0xFE,0x9C,0xBE
.db 0xD7,0xE1,0xBE,0xD9,0xFE,0xD7,0xA3,0xA6,0xE1,0xFE,0x9C,0xBE,0xD7,0xE1,0xBE,0xD9
.db 0xFE,0xD7,0x9A,0x7E,0x60,0x7E,0xA5,0xA0,0xA1,0xBE,0x97,0x97,0xBE,0x7E,0xA1,0x7E
.db 0x9C,0xBE,0xD9,0xAA,0xD7,0x9A,0x7E,0x60,0x7E,0xA5,0xA0,0xA1,0xBE,0x97,0x97,0xBE
.db 0x7E,0xA1,0x7E,0x9C,0xBE,0xD9,0xEA,0xDE,0xED,0xA8,0xBE,0xAF,0xAC,0xBE,0xAA,0xBE
.db 0xB2,0xAF,0xA3,0x7E,0xAD,0x7E,0xE1,0xDE,0xED,0xA8,0xBE,0xAF,0xAC,0xBE,0xAA,0xBE
.db 0xB2,0xAF,0xA3,0x7E,0xAD,0x7E,0xA1,0x00,0x42,0x3F,0x00,0x00,0xDE,0xA5,0xBE,0xE5
.db 0xE1,0xFE,0x9C,0xA0,0xE0,0xA1,0x9E,0xA5,0xBE,0xDE,0xA5,0xBE,0xE5,0xE1,0xFE,0x9C
.db 0xA0,0xE0,0xA1,0x9E,0xA5,0xBE,0xDA,0x5E,0x61,0x5E,0x61,0xA5,0x9C,0xBE,0xDE,0xDC
.db 0xDE,0xA1,0xDE,0xBE,0xDA,0x5E,0x61,0x5E,0x61,0xA5,0x9C,0xBE,0xDE,0xDC,0xDE,0xA1
.db 0xDE,0xFE,0xBE,0xE6,0xFE,0xFE,0xAD,0xBE,0xED,0xBE,0xAD,0xBE,0xF1,0xA8,0xEC,0xFE
.db 0xBE,0xE6,0xFE,0xFE,0xAD,0xBE,0xED,0xBE,0xAD,0xBE,0xF1,0xA8,0xEC,0xBE,0x00,0xC1
.db 0x00,0x00
; Total bytes: 274
; End of file 
_jinj_med_End:: 
