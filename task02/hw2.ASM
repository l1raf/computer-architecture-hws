; Вариант 10
; Разработать программу, использующую динамическое выделение памяти под массив,
; которая вводит одномерный массив A[N], формирует из элементов массива A новый
; массив B из элементов A в обратном порядке

format PE console

entry start

include 'win32ax.inc'

section '.data' data readable writable

        strArraySize db 'Enter array size: ', 0
        strIncorrectSize db 'Incorrect array size %d', 10, 0
        strInputElem db 'A[%d] = ', 0
        strOutputElem db 'B[%d] = %d ', 10, 0

        spaceStr db ' %d', 0
        emptyStr db '%d', 0
        newLine db '', 10, 0

        infinity db 'infinity', 0

        array_size dd 0
        tmp dd ?
        num dd ?
        array_A rd 100
        array_B rd 100

        NULL = 0

section '.code' readable executable

        start:
; Get array size
                push strArraySize
                call[printf]

                push array_size
                push spaceStr
                call [scanf]

                mov eax, [array_size]
                cmp eax, 0
                jg .getArray
; Wrong size
                push array_size
                push strIncorrectSize
                call [printf]
                jmp finish

        .getArray:
                xor ecx, ecx
                mov ebx, array_A

        .getArrayLoop:
                mov [tmp], ebx
                cmp ecx, [array_size]
                jge .endInput

                mov [num], ecx

                push ecx
                push strInputElem
                call [printf]

                push ebx
                push emptyStr
                call [scanf]

                mov ecx, [num]
                inc ecx

                mov ebx, [tmp]
                add ebx, 4
                jmp .getArrayLoop
        .endInput:

        .getArrayB:
                xor ecx, ecx
                mov edx, array_B
                sub ebx, 4

        .getArrayBLoop:
                cmp ecx, [array_size]
                je .endGetArrayB

                mov eax, [ebx]
                mov [edx], eax

                inc ecx
                add edx, 4
                sub ebx, 4
                jmp .getArrayBLoop
        .endGetArrayB:

        .printArrayB:
                push newLine
                call [printf]

                xor ecx, ecx
                mov edx, array_B

        .printArrayBLoop:
                mov [tmp], edx
                cmp ecx, [array_size]
                je .endPrint

                mov [num], ecx
                push dword [edx]
                push ecx
                push strOutputElem
                call [printf]

                mov ecx, [num]
                inc ecx
                mov edx, [tmp]
                add edx, 4
                jmp .printArrayBLoop
        .endPrint:

        finish:
                call [getch]

                push NULL
                call [ExitProcess]

section '.idata' import data readable

        library kernel, 'kernel32.dll',\
                msvcrt, 'msvcrt.dll'

        import kernel,\
        ExitProcess, 'ExitProcess'

        import msvcrt,\
        printf, 'printf',\
        scanf, 'scanf',\
        getch, '_getch'