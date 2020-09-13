



; solver options
PRINT_SOLUTIONS   = 0x01
VERIFY_UNIQUENESS = 0x02


LEVELS fix 4

ID_SetupDialog			fix	  0x2000
ID_SetupDialog_LEVEL0		fix	  0x2080
ID_SetupDialog_LEVEL1		fix	  0x2081
ID_SetupDialog_LEVEL2		fix	  0x2082
ID_SetupDialog_LEVEL3		fix	  0x2083
ID_SetupDialog_LEVEL4		fix	  0x2084
ID_SetupDialog_OK		fix	  0x20F0
ID_SetupDialog_CANCEL		fix	  0x20F1

	     ;   GenerateTries + (RemoveThreashold shl 16) + (TargetDifficulty shl 32)
LEVEL0_PARM fix  (10		+ ( 4 shl 16)		    + (   0 shl 32) )
LEVEL1_PARM fix  (20		+ ( 8 shl 16)		    + (   2 shl 32) )
LEVEL2_PARM fix  (40		+ (16 shl 16)		    + (   7 shl 32) )
LEVEL3_PARM fix  (100		+ (64 shl 16)		    + (  12 shl 32) )
LEVEL4_PARM fix  (1000		+ (80 shl 16)		    + ( 500 shl 32) )



DIB_RGB_COLORS = 0

macro mov a,b,c {
 if c eq
  mov a,b
 else
  mov b,c
  mov a,b
 end if
}

macro @@@ colon
{ @@b equ @@f 
  @@b: 
  local ..new 
  @@f equ ..new }

macro @@@@ colon
{ @@@b equ @@@f
  @@@b:
  local ..new 
  @@@f equ ..new }

macro @@@@@ colon
{ @@@@b equ @@@@f
  @@@@b:
  local ..new 
  @@@@f equ ..new }

macro @@@@@@ colon
{ @@@@@b equ @@@@@f
  @@@@@b:
  local ..new 
  @@@@@f equ ..new }




purge .if
macro .if
{
  __CURRENT equ 1
  __IF equ
  local ..endif
  __ENDIF equ ..endif
  local ..else
  __ELSE equ ..else
}

purge .else
macro .else
{
  jmp __ENDIF
  __ELSE:
  restore __IF
  __IF equ ,
}

purge .elseif
macro .elseif
{
  jmp __ENDIF
  __ELSE:
  restore __ELSE
  local ..else
  __ELSE equ ..else
}

purge .endif
macro .endif
{
  if __IF eq
   __ELSE:
  end if
  __ENDIF:
  restore __ELSE
  restore __ENDIF
  restore __IF
}

macro .e
{ jne __ELSE }

macro .a
{ jna __ELSE }

macro .ae
{ jb __ELSE }

macro .c
{ jnc __ELSE }

macro .b
{ jnb __ELSE }

macro .be
{ ja __ELSE }

macro .z
{ jnz __ELSE }

macro .ne
{ je __ELSE }

macro .ns
{ js __ELSE }

macro .nz
{ jz __ELSE }

macro .ge
{ jl __ELSE }

macro .g
{ jle __ELSE }

macro .le
{ jg __ELSE }

macro .l
{ jge __ELSE }



purge .while
macro .while
{
  __CURRENT equ 2
  local ..while
  __WHILE equ ..while
  local ..endw
  __ENDW equ ..endw
  __WHILE:
}

purge .endw
macro .endw
{
  jmp __WHILE
  __ENDW:
  restore __ENDW
  restore __WHILE
}

macro .we
{ jne __ENDW }

macro .wae
{ jb __ENDW }

macro .wz
{ jnz __ENDW }

macro .wne
{ je __ENDW }

macro .wns
{ js __ENDW }

macro .wnz
{ jz __ENDW }

macro .wle
{ jg __ENDW }



macro .loop
{
  local ..loop
  __LOOP equ ..loop
  __LOOP:
}

macro .lp
{ jmp __LOOP
  restore __LOOP
}

macro .lpnz
{ jnz __LOOP
  restore __LOOP
}

macro .lpne
{ jne __LOOP
  restore __LOOP
}

macro .lpns
{ jns __LOOP
  restore __LOOP
}

macro .lpa
{ ja __LOOP
  restore __LOOP
}

macro .lpna
{ jna __LOOP
  restore __LOOP
}


macro .lpae
{ jae __LOOP
  restore __LOOP
}

macro .lpb
{ jb __LOOP
  restore __LOOP
}

macro .lpbe
{ jbe __LOOP
  restore __LOOP
}


macro .lpge
{ jge __LOOP
  restore __LOOP
}

macro .lpl
{ jl __LOOP
  restore __LOOP
}

macro .lple
{ jle __LOOP
  restore __LOOP
}

macro .lple
{ jle __LOOP
  restore __LOOP
}