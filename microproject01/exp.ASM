; Разработать программу, вычисляющую с
; помощью степенного ряда с точностью не
; хуже 0,1% значение функции exp(x) для
; заданного параметра x (использовать FPU)

format PE console
entry start

include 'win32a.inc'

section '.data' data readable writable

        strInput       db 'Enter x: ', 0
        strOutOfBounds db 'The input value is too small.', 10, 0
        strWrongFormat db 'The input value must be a number.', 10, 0
        strInfRes      db 'The value of exp(x) is too big.', 10, 0
        fmt            db '%lf', 0
        fmtp           db '%lf', 13, 10, 0
        outputStr      db 'exp(%g) = %lf', 10, 0

        x              dq ?
        maxL           dd -6.0
        result         dq 1.0      ; result of calculation exp(x)
        x_n            dq 1.0
        tmpStack       dd ?
        i              dd 0
        eps            dq 0.001

        NULL = 0

section '.code' readable executable

        start:
        ; Gets x and checks the value
                call Input

        ; Calcs exp(x)
                call CalcExp

        ; Print result
                invoke printf, outputStr, dword [x], dword [x + 4],\
                                          dword [result], dword [result + 4]

        finish:
                call [getch]
                push NULL
                call [ExitProcess]

        inf:
                push strInfRes
                call [printf]
                jmp finish

        outOfBounds:
                push strOutOfBounds
                call [printf]
                jmp finish

        wrongFormat:
                push strWrongFormat
                call [printf]
                jmp finish

;------------------------------------------------

        Input:
                mov [tmpStack], esp

                invoke printf, strInput
                invoke scanf, fmt, x

                cmp eax, 1
                jne wrongFormat ; jump to wrongFormat if input value isn't a number

                finit
                fld [x]         ; st0 = x
                fcomp [maxL]    ; compare x with left border -6.0
                fstsw ax
                sahf
                jb outOfBounds  ; jump to outOfBounds if x less than -6.0

                mov esp, [tmpStack]
                ret

;------------------------------------------------

        CalcExp:
                mov [tmpStack], esp

        expLoop:
        ; check if result is infinity
                fstsw ax
                and ax, 1000b
                cmp ax, 0
                jg inf

                inc [i]          ; i++

                ; x_n *= x / i
                fld [x]          ; st0 = x
                fidiv [i]        ; st0 = x / i
                fmul [x_n]       ; x_n *= st0
                fstp [x_n]       ; x_n = st0

                ; result += x_n
                fld [result]     ; st0 = result
                fadd [x_n]       ; st0 += x_n
                fstp [result]    ; result = st0

                fld [x_n]        ; st0 = x_n
                fabs             ; st0 = |x_n|
                fcomp [eps]      ; compare |x_n| with 0.001
                fstsw ax
                sahf
                ja expLoop       ; continue loop if result isn't accurate enough

                mov esp, [tmpStack]
                ret

;------------------------------------------------

section '.idata' import data readable

        library kernel, 'kernel32.dll',\
                msvcrt, 'msvcrt.dll'

        import kernel,\
        ExitProcess, 'ExitProcess'

        import msvcrt,\
        printf, 'printf',\
        scanf, 'scanf',\
        getch, '_getch'