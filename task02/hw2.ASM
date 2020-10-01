; Вариант 10
; Разработать программу, которая вводит одномерный массив A[N],
; формирует новый массив B из элементов A в обратном порядке
; Create array B that contains elements from array A in reverse order

format PE console

entry start

include 'win32ax.inc'

section '.data' data readable writable

        strArraySize     db 'Enter array size: ', 0
        strIncorrectSize db 'Incorrect size of array %d', 10, 0
        strInputElem     db 'A[%d] = ', 0
        strOutputElem    db 'B[%d] = %d ', 10, 0
        strOutputArray   db 'Array B: ', 10, 0
        spaceStr         db ' %d', 0
        emptyStr         db '%d', 0
        newLine          db '', 10, 0

        array_size dd 0
        tmp        dd ?
        i          dd ?
        tmpStack   dd ?
        array_A    rd 1000
        array_B    rd 1000

        NULL = 0

section '.code' readable executable

        start:
; Get array A
                call ArrayInput

; Get array B
                call GetArrayB

; Print array B
                call PrintArrayB

        finish:
                call [getch]

                push NULL
                call [ExitProcess]
;-------------------------------------------------

        ArrayInput:
                push strArraySize
                call[printf]
                add esp, 4

                push array_size
                push spaceStr
                call [scanf]
                add esp, 8

                mov eax, [array_size]
                cmp eax, 0
                jg getArray
; Wrong size
                push [array_size]
                push strIncorrectSize
                call [printf]
                jmp finish

; Fill array A
        getArray:
                xor ecx, ecx
                mov ebx, array_A

        getArrayLoop:
                mov [tmp], ebx
                cmp ecx, [array_size]
                jge endInput

                mov [i], ecx

                push ecx
                push strInputElem
                call [printf]
                add esp, 8

                push ebx
                push emptyStr
                call [scanf]
                add esp, 8

                mov ecx, [i]
                inc ecx

                mov ebx, [tmp]
                add ebx, 4
                jmp getArrayLoop

        endInput:
                ret
;-------------------------------------------------

        GetArrayB:
; Fill array B
                xor ecx, ecx
                mov edx, array_B
                sub ebx, 4

        getArrayBLoop:
                cmp ecx, [array_size]
                je endGetArrayB

                mov eax, [ebx]
                mov [edx], eax

                inc ecx
                add edx, 4
                sub ebx, 4
                jmp getArrayBLoop

        endGetArrayB:
                ret
;-------------------------------------------------

        PrintArrayB:
                push newLine
                call [printf]
                add esp, 4

                push strOutputArray
                call [printf]
                add esp, 4

                mov [tmpStack], esp
                xor ecx, ecx
                mov edx, array_B

        printArrayBLoop:
                mov [tmp], edx
                cmp ecx, [array_size]
                je endPrint

                mov [i], ecx

                push dword [edx]
                push ecx
                push strOutputElem
                call [printf]

                mov ecx, [i]
                inc ecx
                mov edx, [tmp]
                add edx, 4
                jmp printArrayBLoop

        endPrint:
                mov esp, [tmpStack]
                ret
;-------------------------------------------------

section '.idata' import data readable

        library kernel, 'kernel32.dll',\
                msvcrt, 'msvcrt.dll'

        import kernel,\
        ExitProcess, 'ExitProcess'

        import msvcrt,\
        printf, 'printf',\
        scanf, 'scanf',\
        getch, '_getch'