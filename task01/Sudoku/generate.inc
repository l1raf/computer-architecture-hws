

		      align  16
GetRand:	     movdqa  xmm0,dqword[RandSeed]
		     movdqa  xmm1,xmm0
		     movdqa  xmm2,xmm0
		     pslldq  xmm1,1
		     psrldq  xmm2,15
			por  xmm1,xmm2
		     movdqa  xmm2,xmm0
		      psllq  xmm2,1
			por  xmm1,xmm2
		     movdqa  xmm2,xmm0
		     movdqa  xmm3,xmm0
		     psrldq  xmm2,1
		     pslldq  xmm3,15
			por  xmm2,xmm3
		     movdqa  xmm3,xmm0
		      psrlq  xmm3,1
		       pxor  xmm2,xmm3
			por  xmm0,xmm1
		       pxor  xmm0,xmm2
		     movdqa  dqword[RandSeed],xmm0
		   pmovmskb  eax,xmm0
			ret




SudokuGenerate:
		;  first randomly generate a filled board
		;  then randomly remove clues until removing any more clues gives multiple solutions

		     ;   mov  dword[RandSeed+4*0],0x75006bcd
		     ;   mov  dword[RandSeed+4*1],0x78f17a34
		     ;   mov  dword[RandSeed+4*2],0x5043e662
		     ;   mov  dword[RandSeed+4*3],0x00012460

			sub  esp,256

virtual at esp
.RandPerm1 rb 16
.RandPerm2 rb 16
.RandPerm3 rb 16
.ListPerm  rb 16
.index	  dd ?
end virtual


; randomly generate a filled board

.TryAgain:
	      ; generate 3 random permutations of 1,..,9
			xor  edi,edi
.RandPermL1:		mov  dword[.index],edi
			lea  edi,[edi+.RandPerm1]
		     movaps  xmm0,dqword[IdentityPerm]
		     movups  dqword[.ListPerm],xmm0
		       call  GetRand
		       push  eax
		       call  GetRand
			pop  ecx
			shl  ecx,16
			 or  eax,ecx
			mov  ecx,9*8*7*6*5*4*3*2*1
			xor  edx,edx
		       idiv  ecx
			mov  ebx,8
			mov  eax,edx
.RandPermL2:		xor  edx,edx
		       idiv  dword[Factorial+4*ebx]
			mov  ecx,eax
			mov  al,byte[.ListPerm+eax]
		      stosb
			mov  al,byte[.ListPerm+ebx]
			mov  byte[.ListPerm+ecx],al
			mov  eax,edx
			sub  ebx,1
			jns  .RandPermL2
			mov  edi,dword[.index]
			add  edi,16
			cmp  edi,16*3
			 jb  .RandPermL1

	       ; fill in the three diagonal 3x3 blocks
			mov  al,'.'
			mov  ecx,81
			mov  edi,GenerateBoard
		  rep stosb
			mov  edi,GenerateBoard

			mov  byte[edi+00],al,byte[.RandPerm1+0]
			mov  byte[edi+01],al,byte[.RandPerm1+1]
			mov  byte[edi+02],al,byte[.RandPerm1+2]
			mov  byte[edi+09],al,byte[.RandPerm1+3]
			mov  byte[edi+10],al,byte[.RandPerm1+4]
			mov  byte[edi+11],al,byte[.RandPerm1+5]
			mov  byte[edi+18],al,byte[.RandPerm1+6]
			mov  byte[edi+19],al,byte[.RandPerm1+7]
			mov  byte[edi+20],al,byte[.RandPerm1+8]

			mov  byte[edi+30],al,byte[.RandPerm2+0]
			mov  byte[edi+31],al,byte[.RandPerm2+1]
			mov  byte[edi+32],al,byte[.RandPerm2+2]
			mov  byte[edi+39],al,byte[.RandPerm2+3]
			mov  byte[edi+40],al,byte[.RandPerm2+4]
			mov  byte[edi+41],al,byte[.RandPerm2+5]
			mov  byte[edi+48],al,byte[.RandPerm2+6]
			mov  byte[edi+49],al,byte[.RandPerm2+7]
			mov  byte[edi+50],al,byte[.RandPerm2+8]

			mov  byte[edi+60],al,byte[.RandPerm3+0]
			mov  byte[edi+61],al,byte[.RandPerm3+1]
			mov  byte[edi+62],al,byte[.RandPerm3+2]
			mov  byte[edi+69],al,byte[.RandPerm3+3]
			mov  byte[edi+70],al,byte[.RandPerm3+4]
			mov  byte[edi+71],al,byte[.RandPerm3+5]
			mov  byte[edi+78],al,byte[.RandPerm3+6]
			mov  byte[edi+79],al,byte[.RandPerm3+7]
			mov  byte[edi+80],al,byte[.RandPerm3+8]

		 ; see if it is solvable
			mov  eax,GenerateBoard
			mov  ebx,SolutionBoard
			mov  ecx,1
			mov  edx,VERIFY_UNIQUENESS
		       call  SudokuSolve
			cmp  ebx,1
			 jb  .TryAgain	     ; if no solution then try again
			 je  .CopySolution   ; if solution was unique, then copy the filled board

.FillMoreTryAgain:
			mov  esi,GenerateBoard
			mov  edi,BoardCopy
			mov  ecx,81
		  rep movsb
.FillMoreTry:
		  @@:  call  GetRand
			mov  ecx,81
			xor  edx,edx
		       idiv  ecx
			cmp  byte[BoardCopy+edx],'.'
			jne  @b
			mov  ebx,edx

		       call  GetRand
			mov  ecx,9
			xor  edx,edx
		       idiv  ecx
			add  edx,'1'
			mov  byte[BoardCopy+ebx],dl

			mov  eax,BoardCopy
			mov  ebx,SolutionBoard
			mov  ecx,1
			mov  edx,VERIFY_UNIQUENESS
		       call  SudokuSolve
			cmp  ebx,1
			 jb  .FillMoreTryAgain

			mov  esi,BoardCopy
			mov  edi,GenerateBoard
			mov  ecx,81
		  rep movsb
			cmp  ebx,1
			 ja  .FillMoreTry



.CopySolution:		mov  eax,GenerateBoard
			mov  ebx,SolutionBoard
			mov  ecx,1
			mov  edx,PRINT_SOLUTIONS+VERIFY_UNIQUENESS
		       call  SudokuSolve
			mov  ecx,81
			mov  esi,SolutionBoard
			mov  edi,GenerateBoard
		  rep movsb

		;       call  Paint


; check to make sure that board really is valid

		       test  ebx,ebx
			jnz  @f
		     invoke  MessageBoxA,0,'internal error encountered (1)','generate',MB_OK
			@@:

			mov  edi,CheckGroups
	      .1:     xorps  xmm0,xmm0
		     movups  dqword[.ListPerm],xmm0
			xor  esi,esi
	       .2:    movzx  eax,byte[edi+esi]
		      movzx  eax,byte[GenerateBoard+eax]
			sub  eax,'1'
			mov  byte[.ListPerm+eax],-1
			add  esi,1
			cmp  esi,9
			 jb  .2
			mov  ecx,0x00FF
			and  cl,byte[.ListPerm+0]
			and  cl,byte[.ListPerm+1]
			and  cl,byte[.ListPerm+2]
			and  cl,byte[.ListPerm+3]
			and  cl,byte[.ListPerm+4]
			and  cl,byte[.ListPerm+5]
			and  cl,byte[.ListPerm+6]
			and  cl,byte[.ListPerm+7]
			and  cl,byte[.ListPerm+8]
			cmp  cl,0
			jne  .3

		       test  ebx,ebx
			jnz  @f
		     invoke  MessageBoxA,0,'internal error encountered (2)','generate',MB_OK
			@@:
			mov  esi,BoardCopy
			mov  edi,GenerateBoard
			mov  ecx,81
		  rep movsb
			jmp  .RemoveDone

		.3:
			add  edi,16
			cmp  edi,CheckGroupsEnd
			 jb  .1







; remove clues until removing any more clues gives multiple solutions

			mov  eax,dword[RemoveThreashold]
			mov  dword[.index],eax
.RemoveTryAgain:
			mov  esi,GenerateBoard
			mov  edi,BoardCopy
			mov  ecx,81
		  rep movsb
			sub  dword[.index],1
			 js  .RemoveDone

.RemoveTry:	  @@:  call  GetRand
			mov  ecx,81
			xor  edx,edx
		       idiv  ecx
			cmp  byte[BoardCopy+edx],'.'
			 je  @b
			mov  byte[BoardCopy+edx],'.'

			mov  eax,BoardCopy
			mov  ebx,SolutionBoard
			mov  ecx,1
			mov  edx,VERIFY_UNIQUENESS
		       call  SudokuSolve
			cmp  ebx,1
			 ja  .RemoveTryAgain

		       test  ebx,ebx
			jnz  @f
		     invoke  MessageBoxA,0,'internal problem encountered (3)','generate',MB_OK
			jmp  .RemoveDone
			@@:

			mov  esi,BoardCopy
			mov  edi,GenerateBoard
			mov  ecx,81
		  rep movsb
			mov  eax,dword[RemoveThreashold]
			mov  dword[.index],eax
			jmp  .RemoveTry

.RemoveDone:
			add  esp,256
			ret













