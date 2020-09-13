m equ 3
n equ 3
delta equ 4*((m*m*n*n)+5*m*n)-8



  RowSegNumArray1: dd 1,0,0, 4,3,3, 7,6,6
		   dd 2,2,1, 5,5,4, 8,8,7
  RowSegNumArray2: dd 3,4,5, 0,1,2, 0,1,2
		   dd 6,7,8, 6,7,8, 3,4,5
  ColSegNumArray1: dd 1,0,0, 4,3,3, 7,6,6
		   dd 2,2,1, 5,5,4, 8,8,7
  ColSegNumArray2: dd 3,4,5, 0,1,2, 0,1,2
		   dd 6,7,8, 6,7,8, 3,4,5







SolverSetup:
	push	ebp
	sub	esp,4*10
	mov	ebp,esp

	xor	eax,eax
	bts	eax,m*n-1
	mov	ebx,eax
	dec	ebx
	or	ebx,eax
	mov	[FullBitMask_MSB],eax
	mov	[FullBitMask],ebx

	xor	esi,esi
	mov	[ebp+4*0],esi
	mov	[ebp+4*1],esi
	mov	[ebp+4*2],esi
	mov	[ebp+4*3],eax
	mov	[ebp+4*4],eax
	mov	[ebp+4*5],eax
	mov	[ebp+4*8],eax

	mov	eax,n
@@:
	mov	ebx,m
@@@:
	mov	ecx,m
@@@@:
	mov	edx,n
@@@@@:
	mov	edi,[ebp+4*0]
	mov	[esi+RowIndexTable],edi
	mov	edi,[ebp+4*1]
	mov	[esi+ColIndexTable],edi
	mov	edi,[ebp+4*2]
	mov	[esi+BlkIndexTable],edi
	mov	edi,[ebp+4*3]
	mov	[esi+RowMaskTable],edi
	mov	edi,[ebp+4*4]
	mov	[esi+ColMaskTable],edi
	mov	edi,[ebp+4*5]
	mov	[esi+BlkMaskTable],edi
	add	esi,4*1
	
	add	dword[ebp+4*1],4*1		   ;       col
	ror	dword[ebp+4*4],1		   ;       col
	dec	edx
	jnz	@@@@b
	
	add	dword[ebp+4*2],4*n		   ;             blk
	ror	dword[ebp+4*5],1		   ;             blk
	dec	ecx
	jnz	@@@b
	
	add	dword[ebp+4*0],4*m*n		   ; row
	ror	dword[ebp+4*3],1		   ; row
	xor	edi,edi
	mov	dword[ebp+4*1],edi		    ;       col
	mov	edi,[ebp+4*8]			    ;       col
	mov	dword[ebp+4*4],edi		    ;       col

	sub	dword[ebp+4*2],4*m*n		   ;             blk
	rol	dword[ebp+4*5],m		   ;             blk
	dec	ebx
	jnz	@@b
	
	add	dword[ebp+4*2],4*m*m*n		   ;             blk
	ror	dword[ebp+4*5],m		   ;             blk
	dec	eax
	jnz	@b

	
	mov	dword[ebp+4*0],4*m*n*(m*n-1)
	mov	dword[ebp+4*1],4*(m*n-1)
	mov	dword[ebp+4*2],4*n*((m*m*(n-1))+(m-1))


	xor	ecx,ecx
@@:
	mov	ebx,m
@@@:
	mov	eax,[ebp+4*0]
	mov	[RowMaskPosToIndexTable+4*ecx],eax
	mov	eax,[ebp+4*1]
	mov	[ColMaskPosToIndexTable+4*ecx],eax
	mov	eax,[ebp+4*2]
	mov	[BlkMaskPosToIndexTable+4*ecx],eax

	sub	dword[ebp+4*0],4*m*n
	sub	dword[ebp+4*1],4*1
	sub	dword[ebp+4*2],4*n
	inc	ecx
	dec	ebx
	jnz	@@b
	
	sub	dword[ebp+4*2],4*m*n*(n-1)
	cmp	ecx,m*n
	jb	@b



	lea	esi,[CharacterToMaskTable]
	pxor	xmm0,xmm0
	xor	ecx,ecx
@@:	movdqa	[esi+ecx],xmm0
	add	ecx,16
	cmp	ecx,256
	jb	@b

	lea	edx,[MaskPosToCharacterTable]
	mov	ebx,1
	xor	eax,eax
	xor	ecx,ecx
@@:	mov	al,byte[edx+ecx]
	inc	ecx
	mov	[esi+4*eax],ebx
	add	ebx,ebx
	cmp	ecx,m*n
	jb	@b



	add	esp,4*10
	pop	ebp
	ret











;SolutionGrid equ edi+SolutionStack-GridStack


align 16


SudokuSolve:
	      ; in:
	      ;      eax    input buffer    boards should use '1'-'9' for digits and '.' for spaces
	      ;      ebx    output buffer
	      ;      ecx    number of grids to solve
	      ;      edx    options  bit 0: print solutions, bit 1: verify uniqueness
	      ; out:
	      ;      eax    end of output buffer
	      ;      ebx    number of solutions
	      ;      ecx    estimated difficulty

   localvarnum=24+n*(n+1)  ; keep it aligned

 local.eight	     equ ebp+4*3
 LogicReturn	     equ ebp+4*4
 LogicFalseReturn    equ ebp+4*5
 InputBufferCaret    equ ebp+4*6
 OutputBufferCaret   equ ebp+4*7
 SolutionsRemaining  equ ebp+4*8
 Option 	     equ ebp+4*9
 NumberOfGrids	     equ ebp+4*10
 TotalSolutionCount  equ ebp+4*11
 SingleRowMask	     equ ebp+4*12
 SingleColMask	     equ ebp+4*13
 SingleBlkMask	     equ ebp+4*14
 ChangedRowMask      equ ebp+4*15
 ChangedColMask      equ ebp+4*16
 ChangedBlkMask      equ ebp+4*17
 ChangedRowLC	     equ ebp+4*18
 ChangedColLC	     equ ebp+4*19
 Difficulty	     equ ebp+4*20
 seg		     equ ebp+4*21



	push	ebp
	sub	esp,4*localvarnum
	mov	ebp,esp

	and	edx,3
	shl	edx,2
	mov	[InputBufferCaret],eax
	mov	[OutputBufferCaret],ebx
	mov	[NumberOfGrids],ecx

	mov	dword[local.eight],8
	mov	dword[TotalSolutionCount],0
	mov	dword[Difficulty],0

	jmp	dword[@f+edx]

      @@:   dd Setup.Option.00
	    dd Setup.Option.01
	    dd Setup.Option.10
	    dd Setup.Option.11


Solve.EndOfFile:
	mov	ebx,[TotalSolutionCount]
	mov	eax,[OutputBufferCaret]
	add	esp,4*localvarnum
	pop	ebp
	ret

Setup.Option.00:
	mov	dword[Option],Solve.Option.00
	jmp	Solve.Start
Setup.Option.01:
	mov	dword[Option],Solve.Option.01
	jmp	Solve.Start
Setup.Option.10:

	mov	dword[Option],Solve.Option.10
	jmp	Solve.Start
Setup.Option.11:
	mov	dword[Option],Solve.Option.11
	jmp	Solve.Start


Solve.Start:
	mov	dword[SolutionsRemaining],2
	lea	edi,[GridStack]
	mov	dword[edi-4*1],m*m*n*n

	mov	ecx,[InputBufferCaret]
@@:	mov	al,[ecx]
	inc	ecx
	test	al,al
	jz	Solve.EndOfFile
	cmp	al,'.'
	jb	@b
	dec	ecx

	xor	esi,esi
	xor	eax,eax
	mov	edx,m*m*n*n
@@:	mov	al,[ecx]
	mov	ebx,[CharacterToMaskTable+4*eax]
	inc	ecx
	mov	eax,ebx
	test	ebx,ebx
	mov	[esp-1*4],esi
	mov	[esp-2*4],ebx
	mov	ebx,[FullBitMask]
	mov	[edi+esi],ebx
	cmovnz	eax,[local.eight]
	xor	ebx,ebx
	mov	[SolutionGrid+esi],ebx
	sub	esp,eax
	lea	esi,[esi+4*1]
	dec	edx
	jnz	@b

	mov	[InputBufferCaret],ecx
	xor	eax,eax
	lea	edx,[edi+4*m*m*n*n]
	mov	ecx,3*m*n
@@:	mov	[edx],eax
	add	edx,4*1
	dec	ecx
	jnz	@b
	mov	dword[LogicReturn],@f
	mov	dword[LogicFalseReturn],Solve.End
	jmp	Logic
@@:	mov	dword[LogicReturn],Solve.MakeBranch
	mov	dword[LogicFalseReturn],Solve.Analyze

Solve.MakeBranch:

;call PrintGrid

	call	FindGuess
	mov	edx,[edi+eax]
	mov	ebx,edi
	mov	ecx,edx
	neg	ecx
	and	ecx,edx
	xor	edx,ecx
	mov	[edi-4*2],eax
	mov	[edi-4*3],ecx
Solve.CopyBranch:
	add	edi,delta
	mov	ecx,edx
	neg	ecx
	and	ecx,edx

	inc	dword[Difficulty]

	mov	eax,[ebx-4*1]
	mov	[edi-4*1],eax
	mov	eax,[ebx-4*2]
	mov	[edi-4*2],eax
	mov	[edi-4*3],ecx

	xor	eax,eax
j0 = 0
j1 = 2
j2 = 4
j3 = 6
lim = 4*((m*m*n*n)+3*m*n)

 rept 10 {


      if j0 < lim
	 movaps xmm0,dqword[eax+16*0+ebx]
      end if
      if j1 < lim
	 movaps xmm1,dqword[eax+16*1+ebx]
      end if
      if j2 < lim
	 movaps xmm2,dqword[eax+16*2+ebx]
      end if
      if j3 < lim
	 movaps xmm3,dqword[eax+16*3+ebx]
      end if

;      if j0 < lim
;         movaps xmm4,dqword[eax+16*0+ebx+SolutionStack-GridStack]
;      end if
;      if j1 < lim
;         movaps xmm5,dqword[eax+16*1+ebx+SolutionStack-GridStack]
;      end if
;      if j2 < lim
;         movaps xmm6,dqword[eax+16*2+ebx+SolutionStack-GridStack]
;      end if
;      if j3 < lim
;         movaps xmm7,dqword[eax+16*3+ebx+SolutionStack-GridStack]
;      end if


      if j0 < lim
	 movaps dqword[eax+16*0+edi],xmm0
      end if
      if j1 < lim
	 movaps dqword[eax+16*1+edi],xmm1
      end if
      if j2 < lim
	 movaps dqword[eax+16*2+edi],xmm2
      end if
      if j3 < lim
	 movaps dqword[eax+16*3+edi],xmm3
      end if

;      if j0 < lim
;         movaps dqword[eax+16*0+edi+SolutionStack-GridStack],xmm4
;      end if
;      if j1 < lim
;         movaps dqword[eax+16*1+edi+SolutionStack-GridStack],xmm5
;      end if
;      if j2 < lim
;         movaps dqword[eax+16*2+edi+SolutionStack-GridStack],xmm6
;      end if
;      if j3 < lim
;         movaps dqword[eax+16*3+edi+SolutionStack-GridStack],xmm7
;      end if

j0 = j0+16*4
j1 = j1+16*4
j2 = j2+16*4
j3 = j3+16*4

	add	eax,8*8

 }



	xor	edx,ecx
	jnz	Solve.CopyBranch
Solve.Analyze:
	push	dword[edi-4*2] ; position
	push	dword[edi-4*3] ; single
	jmp	Logic
Solve.End:
	dec	dword[NumberOfGrids]
	jnz	Solve.Start
	mov	eax,[OutputBufferCaret]
	mov	ebx,[TotalSolutionCount]
	mov	ecx,[Difficulty]
	add	esp,4*localvarnum
	pop	ebp
	ret


Solve.Option.00:
	sub	edi,delta
	inc	dword[TotalSolutionCount]
	dec	dword[NumberOfGrids]
	jnz	Solve.Start
	mov	eax,[OutputBufferCaret]
	mov	ebx,[TotalSolutionCount]
	mov	ecx,[Difficulty]
	add	esp,4*localvarnum
	pop	ebp
	ret

Solve.Option.01:
	mov	ebx,[OutputBufferCaret]
	lea	esi,[SolutionGrid]
	mov	ecx,m*m*n*n
@@:	mov	eax,[esi]
	add	esi,4
	bsf	eax,eax
	mov	al,byte[MaskPosToCharacterTable+eax]
	mov	byte[ebx],al
	inc	ebx
	dec	ecx
	jnz	@b
	mov	dword[ebx],0x0d0a0d0a
	add	ebx,4
	mov	[OutputBufferCaret],ebx
	sub	edi,delta
	inc	dword[TotalSolutionCount]
	dec	dword[NumberOfGrids]
	jnz	Solve.Start
	mov	eax,[OutputBufferCaret]
	mov	ebx,[TotalSolutionCount]
	mov	ecx,[Difficulty]
	add	esp,4*localvarnum
	pop	ebp
	ret

Solve.Option.10:

	sub	edi,delta
	inc	dword[TotalSolutionCount]
	dec	dword[SolutionsRemaining]
	jz	@f
	cmp	edi,GridStack
	jnb	Solve.Analyze
@@:	dec	dword[NumberOfGrids]
	jnz	Solve.Start
	mov	eax,[OutputBufferCaret]
	mov	ebx,[TotalSolutionCount]
	mov	ecx,[Difficulty]
	add	esp,4*localvarnum
	pop	ebp
	ret

Solve.Option.11:
	mov	ebx,[OutputBufferCaret]
	lea	esi,[SolutionGrid]
	mov	ecx,m*m*n*n
@@:	mov	eax,[esi]
	add	esi,4
	bsf	eax,eax
	mov	al,byte[MaskPosToCharacterTable+eax]
	mov	byte[ebx],al
	inc	ebx
	dec	ecx
	jnz	@b
	mov	dword[ebx],0x0d0a0d0a
	add	ebx,4
	mov	[OutputBufferCaret],ebx
	sub	edi,delta
	inc	dword[TotalSolutionCount]
	dec	dword[SolutionsRemaining]
	jz	@f
	cmp	edi,GridStack
	jnb	Solve.Analyze
@@:	dec	dword[NumberOfGrids]
	jnz	Solve.Start
	mov	eax,[OutputBufferCaret]
	mov	ebx,[TotalSolutionCount]
	mov	ecx,[Difficulty]
	add	esp,4*localvarnum
	pop	ebp
	ret


;PrintGrid:         ; for debugging purposes
;        push ebp
;        push eax
;        push ebx
;        push ecx
;        push edx
;        push esi
;;int 3
;        sub  esp,8+32+((m*m*n*n) and 0xFFFFFFF0)
;        mov  ebx,esp
;
;        xor  esi,esi
;        mov  ecx,m*n
;@@@@:
; rept m*n {
;        mov  edx,[edi+esi]
;        mov  eax,[SolutionGrid+esi]
;        add  esi,4
;        test edx,edx
;        jnz  @f
;        bsf  eax,eax
;        mov  al,[MaskPosToCharacterTable+eax]
;        mov  [ebx],al
;        inc  ebx
;        jmp  @@f
;@@:     mov  byte[ebx],0x2E
;        inc  ebx
;@@@:
; }
;        mov  byte[ebx],10
;        inc  ebx
;        dec  ecx
;        jnz  @@@b
;        mov  byte[ebx],10
;        inc  ebx
;        mov  byte[ebx],10
;        inc  ebx
;        invoke sprintf,ebx,PrintGrid.string,dword[edi-4*1]
;        add  esp,4*3
;        mov  eax,esp
;        invoke MessageBoxA,NULL,eax,PrintGrid.caption,NULL
;
;        add  esp,8+32+((m*m*n*n) and 0xFFFFFFF0)
;
;        pop  esi
;        pop  edx
;        pop  ecx
;        pop  ebx
;        pop  eax
;        pop  ebp
;        ret
;
;PrintGrid.string  db '%i blanks',0
;PrintGrid.caption db 'current grid:',0





align 16
Logic:
	xor	eax,eax
	mov	[ChangedRowMask],eax
	mov	[ChangedColMask],eax
	mov	[ChangedBlkMask],eax
	mov	[ChangedRowLC],eax
	mov	[ChangedColLC],eax
NakedSingles:
	sub	ebp,4*2*2
	cmp	esp,ebp
	lea	ebp,[ebp+4*2*2]
;        jb      MultipleSingles               MultipleSingles is not nec. but does work
	cmp	esp,ebp
	mov	dword[local.eight],8
	jae	HiddenSingles
NS.Start:
	pop	edx
	pop	esi
	mov	eax,[edi+esi]
	test	eax,eax
	jz	NS.End
	test	eax,edx
	jz	Logic.Illogical
	mov	[SolutionGrid+esi],edx
	dec	dword[edi-4*1]
	xor	eax,eax
	mov	[edi+esi],eax
	mov	eax,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
	mov	ecx,[esi+BlkMaskTable]
	or	[ChangedRowMask],eax
	or	[ChangedColMask],ebx
	or	[ChangedBlkMask],ecx
	bsf	eax,eax
	bsf	ebx,ebx
	bsf	ecx,ecx
	or	[edi+4*m*m*n*n+0*4*m*n+4*eax],edx
	or	[edi+4*m*m*n*n+1*4*m*n+4*ebx],edx
	or	[edi+4*m*m*n*n+2*4*m*n+4*ecx],edx
NS.Row:
	mov	[ebp+4*0],esi
	mov	esi,[esi+RowIndexTable]
	mov	ebx,[FullBitMask_MSB]
rept m*n j:1 {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	mov	ecx,eax
	dec	eax
	mov	[edi+esi],ecx
	and	eax,ecx
	push	esi
	push	ecx
	cmovnz	eax,[local.eight]
	mov	ecx,[esi+BlkMaskTable]
	or	[ChangedColMask],ebx
	or	[ChangedBlkMask],ecx
	lea	esp,[esp+eax]
@@:  if J mod (m*n)
	add	esi,4*1
	shr	ebx,1
     end if
  forward
}
NS.Col:
	mov	esi,[ebp+4*0]
	mov	esi,[esi+ColIndexTable]
	mov	ebx,[FullBitMask_MSB]
rept m*n j {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	mov	ecx,eax
	dec	eax
	mov	[edi+esi],ecx
	and	eax,ecx
	push	esi
	push	ecx
	cmovnz	eax,[local.eight]
	mov	ecx,[esi+BlkMaskTable]
	or	[ChangedRowMask],ebx
	or	[ChangedBlkMask],ecx
	lea	esp,[esp+eax]
@@:  if J mod (m*n)
	add	esi,4*m*n
	shr	ebx,1
     end if
 forward
}
NS.Blk:
	mov	esi,[ebp+4*0]
	mov	esi,[esi+BlkIndexTable]
	mov	ecx,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
rept m*n j:1 {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	push	esi
	push	eax
	push	ecx
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	mov	[edi+esi],ecx
	pop	ecx
	cmovnz	eax,[local.eight]
	or	[ChangedRowMask],ecx
	or	[ChangedColMask],ebx
	lea	esp,[esp+eax]
@@:  if J mod n
	ror	ebx,1
	add	esi,4*1
     else
	rol	ebx,n-1
	add	esi,4*(((m-1)*n)+1)
	ror	ecx,1
     end if
  forward
}
NS.End:
	cmp	esp,ebp
	jb	NS.Start
	cmp	dword[edi-4*1],0
	je	Logic.Solved
HiddenSingles:
HS.Row.Start:
	bsf	ecx,[ChangedRowMask]
	jz	HS.Col.Start
HS.Row: mov	esi,[RowMaskPosToIndexTable+4*ecx]
	add	esi,edi
	btr	[ChangedRowMask],ecx
	bts	[ChangedRowLC],ecx
	push	ecx
	xor	edx,edx
	xor	ebx,ebx
 rept m*n j:0 {
	mov	eax,[esi+4*j]
	mov	ecx,edx
	and	ecx,eax
	or	edx,eax
	or	ebx,ecx    }
	pop	ecx
	mov	eax,edx
	or	edx,[edi+4*m*m*n*n+0*4*m*n+4*ecx]
	cmp	edx,[FullBitMask]
	jne	Logic.Illogical
	xor	ebx,eax
	jz	HS.Row.Start
	bts	[ChangedRowMask],ecx
	mov	esi,[RowMaskPosToIndexTable+4*ecx]
 rept m*n {
	mov	eax,[edi+esi]
	and	eax,ebx
	jz	@f
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	jnz	Logic.Illogical
	cmp	ecx,ebx
	push	esi
	push	ecx
	je	NakedSingles
	xor	ebx,ecx
@@:	lea	esi,[esi+4*1]	 }
	bsf	ecx,[ChangedRowMask]
	jnz	HS.Row
HS.Col.Start:
	bsf	ecx,[ChangedColMask]
	jz	HS.Blk.Start
HS.Col: mov	esi,[ColMaskPosToIndexTable+4*ecx]
	add	esi,edi
	btr	[ChangedColMask],ecx
	bts	[ChangedColLC],ecx
	push	ecx
	xor	edx,edx
	xor	ebx,ebx
rept m*n j:0 {
	mov	eax,[esi+4*m*n*j]
	mov	ecx,edx
	and	ecx,eax
	or	edx,eax
	or	ebx,ecx     }
	pop	ecx
	mov	eax,edx
	or	edx,[edi+4*m*m*n*n+1*4*m*n+4*ecx]
	cmp	edx,[FullBitMask]
	jne	Logic.Illogical
	xor	ebx,eax
	jz	HS.Col.Start
	bts	[ChangedColMask],ecx
	mov	esi,[ColMaskPosToIndexTable+4*ecx]
 rept m*n {
	mov	eax,[edi+esi]
	and	eax,ebx
	jz	@f
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	jnz	Logic.Illogical
	cmp	ecx,ebx
	push	esi
	push	ecx
	je	NakedSingles
	xor	ebx,ecx
@@:	add	esi,4*m*n    }
	bsf	ecx,[ChangedColMask]
	jnz	HS.Col
HS.Blk.Start:
	bsf	ecx,[ChangedBlkMask]
	jz	HS.End
HS.Blk: mov	esi,[BlkMaskPosToIndexTable+4*ecx]
	add	esi,edi
	btr	[ChangedBlkMask],ecx
	push	ecx
	xor	edx,edx
	xor	ebx,ebx
 rept m j1:0 {
  rept n j2:0 \{
	mov	eax,[esi+4*j2+4*m*n*j1]
	mov	ecx,edx
	and	ecx,eax
	or	edx,eax
	or	ebx,ecx      \} }
	pop	ecx
	mov	eax,edx
	or	edx,[edi+4*m*m*n*n+2*4*m*n+4*ecx]
	cmp	edx,[FullBitMask]
	jne	Logic.Illogical
	xor	ebx,eax
	jz	HS.Blk.Start
	bts	[ChangedBlkMask],ecx
	mov	esi,[BlkMaskPosToIndexTable+4*ecx]
rept m*n j {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	and	eax,ebx
	jz	@f
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	jnz	Logic.Illogical
	cmp	ecx,ebx
	push	esi
	push	ecx
	je	NakedSingles
	xor	ebx,ecx
@@:
     if J mod n
	add	esi,4*1
     else
	add	esi,4*(((m-1)*n)+1)
     end if
forward
}
	bsf	ecx,[ChangedBlkMask]
	jnz	HS.Blk
HS.End:





LockedCandidates:
	mov	edx,[mMask]
	mov	esi,edi
LC.Row: test	[ChangedRowLC],edx
	jz	@f
	not	edx
	and	[ChangedRowLC],edx
	not	edx
	mov	eax,[esi+4*3*0+4*2]
	mov	ebx,[esi+4*3*1+4*2]
	mov	ecx,[esi+4*3*2+4*2]
	or	eax,[esi+4*3*0+4*1]
	or	ebx,[esi+4*3*1+4*1]
	or	ecx,[esi+4*3*2+4*1]
	or	eax,[esi+4*3*0+4*0]
	or	ebx,[esi+4*3*1+4*0]
	or	ecx,[esi+4*3*2+4*0]
	mov	[seg+4*0],eax
	mov	[seg+4*1],ebx
	mov	[seg+4*2],ecx
	mov	eax,[esi+4*3*3+4*2]
	mov	ebx,[esi+4*3*4+4*2]
	mov	ecx,[esi+4*3*5+4*2]
	or	eax,[esi+4*3*3+4*1]
	or	ebx,[esi+4*3*4+4*1]
	or	ecx,[esi+4*3*5+4*1]
	or	eax,[esi+4*3*3+4*0]
	or	ebx,[esi+4*3*4+4*0]
	or	ecx,[esi+4*3*5+4*0]
	mov	[seg+4*3],eax
	mov	[seg+4*4],ebx
	mov	[seg+4*5],ecx
	mov	eax,[esi+4*3*6+4*2]
	mov	ebx,[esi+4*3*7+4*2]
	mov	ecx,[esi+4*3*8+4*2]
	or	eax,[esi+4*3*6+4*1]
	or	ebx,[esi+4*3*7+4*1]
	or	ecx,[esi+4*3*8+4*1]
	or	eax,[esi+4*3*6+4*0]
	or	ebx,[esi+4*3*7+4*0]
	or	ecx,[esi+4*3*8+4*0]
	mov	[seg+4*6],eax
	mov	[seg+4*7],ebx
	mov	[seg+4*8],ecx
	xor	eax,eax
 rept m*n j:0 {
  rept m-1 i:0 \{
	load x dword from RowSegNumArray1+4*j+4*m*n*i
	load y dword from RowSegNumArray2+4*j+4*m*n*i
     if i <> 0
	or	ebx,[seg+4*x]
	or	ecx,[seg+4*y]
     else
	mov	ebx,[seg+4*x]
	mov	ecx,[seg+4*y]
     end if
  \}
	xor	ebx,ecx
	and	ebx,[seg+4*j]
	mov	eax,4*j
	jnz	LC.RowFound
 }
@@:	add	esi,4*m*m*n
	shr	edx,n
	jnz	LC.Row

	mov	edx,[nMask]
	mov	esi,edi
LC.Col:
	test	[ChangedColLC],edx
	jz	@f
	not	edx
	and	[ChangedColLC],edx
	not	edx
	mov	eax,[esi+4*27*0+4*0+4*9*2]
	mov	ebx,[esi+4*27*1+4*0+4*9*2]
	mov	ecx,[esi+4*27*2+4*0+4*9*2]
	or	eax,[esi+4*27*0+4*0+4*9*1]
	or	ebx,[esi+4*27*1+4*0+4*9*1]
	or	ecx,[esi+4*27*2+4*0+4*9*1]
	or	eax,[esi+4*27*0+4*0+4*9*0]
	or	ebx,[esi+4*27*1+4*0+4*9*0]
	or	ecx,[esi+4*27*2+4*0+4*9*0]
	mov	[seg+4*0],eax
	mov	[seg+4*1],ebx
	mov	[seg+4*2],ecx
	mov	eax,[esi+4*27*0+4*1+4*9*2]
	mov	ebx,[esi+4*27*1+4*1+4*9*2]
	mov	ecx,[esi+4*27*2+4*1+4*9*2]
	or	eax,[esi+4*27*0+4*1+4*9*1]
	or	ebx,[esi+4*27*1+4*1+4*9*1]
	or	ecx,[esi+4*27*2+4*1+4*9*1]
	or	eax,[esi+4*27*0+4*1+4*9*0]
	or	ebx,[esi+4*27*1+4*1+4*9*0]
	or	ecx,[esi+4*27*2+4*1+4*9*0]
	mov	[seg+4*3],eax
	mov	[seg+4*4],ebx
	mov	[seg+4*5],ecx
	mov	eax,[esi+4*27*0+4*2+4*9*2]
	mov	ebx,[esi+4*27*1+4*2+4*9*2]
	mov	ecx,[esi+4*27*2+4*2+4*9*2]
	or	eax,[esi+4*27*0+4*2+4*9*1]
	or	ebx,[esi+4*27*1+4*2+4*9*1]
	or	ecx,[esi+4*27*2+4*2+4*9*1]
	or	eax,[esi+4*27*0+4*2+4*9*0]
	or	ebx,[esi+4*27*1+4*2+4*9*0]
	or	ecx,[esi+4*27*2+4*2+4*9*0]
	mov	[seg+4*6],eax
	mov	[seg+4*7],ebx
	mov	[seg+4*8],ecx
 rept m*n j:0 {
  rept m-1 i:0 \{
	load x dword from ColSegNumArray1+4*j+4*m*n*i
	load y dword from ColSegNumArray2+4*j+4*m*n*i
     if i <> 0
	or	ebx,[seg+4*x]
	or	ecx,[seg+4*y]
     else
	mov	ebx,[seg+4*x]
	mov	ecx,[seg+4*y]
     end if
  \}
	xor	ebx,ecx
	and	ebx,[seg+4*j]
	mov	eax,4*j
	jnz	LC.ColFound
 }
@@:	add	esi,4*n
	shr	edx,n
	jnz	LC.Col
	mov	esp,ebp
	jmp	dword[LogicReturn]
LC.RowFound:
	mov	[ebp+4*1],eax
	xor	eax,eax
	mov	[ebp+4*0],eax
	mov	[ebp+4*2],esi
	mov	ecx,ebx
	not	ebx
 rept 4 j:0 {
	mov	eax,[ebp+4*1]
	mov	esi,[ebp+4*2]
	sub	esi,edi
	add	esi,[RowSegIndexArray+4*m*n*j+eax]
  rept n \{
	mov	edx,[edi+esi]
	test	edx,ecx
	jz	@f
	and	edx,ebx
	jz	Logic.Illogical
	mov	eax,[RowMaskTable+esi]
	or	[ChangedRowMask],eax
	mov	eax,[ColMaskTable+esi]
	or	[ChangedColMask],eax
	mov	eax,[BlkMaskTable+esi]
	or	[ChangedBlkMask],eax
	mov	[edi+esi],edx
	mov	eax,edx
	dec	edx
	and	edx,eax
	mov	dword[ebp+4*0],1
	cmovnz	edx,[local.eight]
	push	esi
	push	eax
	lea	esp,[esp+edx]
@@:	lea	esi,[esi+4*1]
  \}
 }
	mov	eax,[ebp+4*0]
	test	eax,eax
	jnz	NakedSingles
	mov	esp,ebp
	jmp	dword[LogicReturn]
LC.ColFound:
	mov	[ebp+4*1],eax
	xor	eax,eax
	mov	[ebp+4*0],eax
	mov	[ebp+4*2],esi
	mov	ecx,ebx
	not	ebx
 rept 4 j:0 {
	mov	eax,[ebp+4*1]
	mov	esi,[ebp+4*2]
	sub	esi,edi
	add	esi,[ColSegIndexArray+4*m*n*j+eax]
  rept n \{
	mov	edx,[edi+esi]
	test	edx,ecx
	jz	@f
	and	edx,ebx
	jz	Logic.Illogical
	mov	eax,[RowMaskTable+esi]
	or	[ChangedRowMask],eax
	mov	eax,[ColMaskTable+esi]
	or	[ChangedColMask],eax
	mov	eax,[BlkMaskTable+esi]
	or	[ChangedBlkMask],eax
	mov	[edi+esi],edx
	mov	eax,edx
	dec	edx
	and	edx,eax
	mov	dword[ebp+4*0],1
	cmovnz	edx,[local.eight]
	push	esi
	push	eax
	lea	esp,[esp+edx]
@@:	add	esi,4*m*n
  \}
 }
	mov	eax,[ebp+4*0]
	test	eax,eax
	jnz	NakedSingles

Logic.End:
	mov	esp,ebp
	jmp	dword[LogicReturn]
Logic.Illogical:
;int3
	cmp	edi,GridStack
	mov	esp,ebp
	je	Solve.End
	sub	edi,delta
	jmp	dword[LogicFalseReturn]
Logic.Solved:
	mov	esp,ebp
	jmp	dword[Option]

align 16
FindGuess:
	mov	edx,m*n+1
	mov	esi,4*(m*m*n*n-1)
@@@:
	mov	eax,[edi+esi]
	test	eax,eax
	jz	@f

	movzx	eax,byte[myPopCnt+eax]

	cmp	eax,edx
	mov	[ebp+4*0],eax
	ja	@f
	jb	doit
	je	try
@@:	sub	esi,4
	jae	@@b
	mov	eax,ecx
	ret
doit:
	mov	eax,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
	mov	ecx,[esi+BlkMaskTable]
	bsf	eax,eax
	bsf	ebx,ebx
	bsf	ecx,ecx

	mov  eax,[edi+4*m*m*n*n+0*4*m*n+4*eax]
	mov  ebx,[edi+4*m*m*n*n+1*4*m*n+4*ebx]
	mov  ecx,[edi+4*m*m*n*n+2*4*m*n+4*ecx]
	movzx	ebx,byte[myPopCnt+ebx]
	movzx	ecx,byte[myPopCnt+ecx]
	movzx	eax,byte[myPopCnt+eax]

	add	ebx,ecx
	add	eax,ebx
	mov	ecx,esi
	mov	edx,[ebp+4*0]
	mov	ebx,eax
	sub	esi,4
	jae	@@b
	mov	eax,ecx
	ret
try:
	push	ebx
	push	ecx
	mov	eax,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
	mov	ecx,[esi+BlkMaskTable]
	bsf	eax,eax
	bsf	ebx,ebx
	bsf	ecx,ecx

	mov	eax,[edi+4*m*m*n*n+0*4*m*n+4*eax]
	mov	ebx,[edi+4*m*m*n*n+1*4*m*n+4*ebx]
	mov	ecx,[edi+4*m*m*n*n+2*4*m*n+4*ecx]
	movzx	ebx,byte[myPopCnt+ebx]
	movzx	ecx,byte[myPopCnt+ecx]
	movzx	eax,byte[myPopCnt+eax]

	add	ebx,ecx
	add	eax,ebx
	pop	ecx
	pop	ebx
	cmp	eax,ebx
	cmovna	ecx,esi
	cmovna	edx,[ebp+4*0]
	cmovna	ebx,eax
	sub	esi,4
	jae	@@b
	mov	eax,ecx
	ret





	align 16
MultipleSingles:

;int3
	xor	eax,eax
	mov	[SingleRowMask],eax
	mov	[SingleColMask],eax
	mov	[SingleBlkMask],eax
MS_NS_Loop:
	pop	edx
	pop	esi
	mov	eax,[edi+esi]
	test	eax,eax
	jz	MS_NS_Loop_Cont
	test	eax,edx
	jz	Logic.Illogical
	mov	[SolutionGrid+esi],edx
	dec	dword[edi-4*1]
	mov	dword[edi+esi],0
	mov	eax,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
	mov	ecx,[esi+BlkMaskTable]
	or	[SingleRowMask],eax
	or	[SingleColMask],ebx
	or	[SingleBlkMask],ecx
	bsf	eax,eax
	bsf	ebx,ebx
	bsf	ecx,ecx
	or	[edi+4*m*m*n*n+0*4*m*n+4*eax],edx
	or	[edi+4*m*m*n*n+1*4*m*n+4*ebx],edx
	or	[edi+4*m*m*n*n+2*4*m*n+4*ecx],edx


MS.NS.Row:
	mov	[ebp+4*0],esi
	mov	esi,[esi+RowIndexTable]
	mov	ebx,[FullBitMask_MSB]
rept m*n j:1 {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	mov	ecx,eax
	dec	eax
	mov	[edi+esi],ecx
	and	eax,ecx
	push	esi
	push	ecx
	cmovnz	eax,[local.eight]
	mov	ecx,[esi+BlkMaskTable]
	or	[ChangedColMask],ebx
	or	[ChangedBlkMask],ecx
	lea	esp,[esp+eax]
@@:  if J mod (m*n)
	add	esi,4*1
	shr	ebx,1
     end if
  forward
}
MS.NS.Col:
	mov	esi,[ebp+4*0]
	mov	esi,[esi+ColIndexTable]
	mov	ebx,[FullBitMask_MSB]
rept m*n j {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	mov	ecx,eax
	dec	eax
	mov	[edi+esi],ecx
	and	eax,ecx
	push	esi
	push	ecx
	cmovnz	eax,[local.eight]
	mov	ecx,[esi+BlkMaskTable]
	or	[ChangedRowMask],ebx
	or	[ChangedBlkMask],ecx
	lea	esp,[esp+eax]
@@:  if J mod (m*n)
	add	esi,4*m*n
	shr	ebx,1
     end if
 forward
}

MS.NS.Blk:
	mov	esi,[ebp+4*0]
	mov	esi,[esi+BlkIndexTable]
	mov	ecx,[esi+RowMaskTable]
	mov	ebx,[esi+ColMaskTable]
rept m*n j:1 {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,edx
	jz	@f
	xor	eax,edx
	jz	Logic.Illogical
	push	esi
	push	eax
	push	ecx
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	mov	[edi+esi],ecx
	pop	ecx
	cmovnz	eax,[local.eight]
	or	[ChangedRowMask],ecx
	or	[ChangedColMask],ebx
	lea	esp,[esp+eax]
@@:  if J mod n
	ror	ebx,1
	add	esi,4*1
     else
	rol	ebx,n-1
	add	esi,4*(((m-1)*n)+1)
	ror	ecx,1
     end if
  forward
}
MS.NS.End:




MS_NS_Loop_Cont:
	cmp	esp,ebp
	jb	MS_NS_Loop


MS.Row:
	bsf	eax,[SingleRowMask]
	jz	MS.Col
@@@:
	mov	esi,[RowMaskPosToIndexTable+4*eax]
	mov	ebx,[edi+4*m*m*n*n+0*4*m*n+4*eax]
	btr	[SingleRowMask],eax
 rept m*n {
	mov  eax,[edi+esi]
	test eax,ebx
	jz   @f
	not  ebx
	and  eax,ebx
	not  ebx
	mov  [edi+esi],eax
	jz   Logic.Illogical
	mov  ecx,eax
	dec  eax
	and  eax,ecx
	push esi
	push ecx
	cmovnz eax,[local.eight]
	mov  ecx,[RowMaskTable+esi]
	or   [ChangedRowMask],ecx
	mov  ecx,[ColMaskTable+esi]
	or   [ChangedColMask],ecx
	mov  ecx,[BlkMaskTable+esi]
	or   [ChangedBlkMask],ecx
	lea  esp,[esp+eax]
@@:	add  esi,4*1		     }
	bsf  eax,[SingleRowMask]
	jnz  @@b

MS.Col:
	bsf  eax,[SingleColMask]
	jz   MS.Blk
@@@:
	mov  esi,[ColMaskPosToIndexTable+4*eax]
	mov  ebx,[edi+4*m*m*n*n+1*4*m*n+4*eax]
	btr  [SingleColMask],eax
 rept m*n {
	mov  eax,[edi+esi]
	test eax,ebx
	jz   @f
	not  ebx
	and  eax,ebx
	not  ebx
	mov  [edi+esi],eax
	jz   Logic.Illogical
	mov  ecx,eax
	dec  eax
	and  eax,ecx
	push esi
	push ecx
	cmovnz eax,[local.eight]
	mov  ecx,[RowMaskTable+esi]
	or   [ChangedRowMask],ecx
	mov  ecx,[ColMaskTable+esi]
	or   [ChangedColMask],ecx
	mov  ecx,[BlkMaskTable+esi]
	or   [ChangedBlkMask],ecx
	lea  esp,[esp+eax]
@@:	add  esi,4*m*n		     }
	bsf  eax,[SingleColMask]
	jnz  @@b

MS.Blk:
	bsf  eax,[SingleBlkMask]
	jz   MS.End
@@@:
	mov  esi,[BlkMaskPosToIndexTable+4*eax]
	mov  ebx,[edi+4*m*m*n*n+2*4*m*n+4*eax]
	btr  [SingleBlkMask],eax
 rept m*n j {
  common
    local J
  forward
     J = j
	mov	eax,[edi+esi]
	test	eax,ebx
	jz	@f
	not	ebx
	and	eax,ebx
	not	ebx
	mov	[edi+esi],eax
	jz	Logic.Illogical
	mov	ecx,eax
	dec	eax
	and	eax,ecx
	push	esi
	push	ecx
	cmovnz	eax,[local.eight]
	mov	ecx,[RowMaskTable+esi]
	or	[ChangedRowMask],ecx
	mov	ecx,[ColMaskTable+esi]
	or	[ChangedColMask],ecx
	mov	ecx,[BlkMaskTable+esi]
	or	[ChangedBlkMask],ecx
	lea	esp,[esp+eax]
@@:
     if J mod n
	add	esi,4*1
     else if 1
	add	esi,4*(((m-1)*n)+1)
     end if

 }
	bsf	eax,[SingleBlkMask]
	jnz	@@b
MS.End:

;int3
	cmp	dword[edi-4*1],0
	je	Logic.Solved
	sub	ebp,4*2*2
	cmp	esp,ebp
	lea	ebp,[ebp+4*2*2]
	jb	MultipleSingles
	cmp	esp,ebp
	jae	HiddenSingles
	jmp	NS.Start