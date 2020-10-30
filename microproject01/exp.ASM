; Разработать программу, вычисляющую с
; помощью степенного ряда с точностью не
; хуже 0,1% значение функции exp(x) для
; заданного параметра x (использовать FPU)

format PE console
entry start

include 'win32a.inc'

section '.data' data readable writable

        strInput       db 'Enter x: ', 0
        strOutOfBounds db 'Value of x out of bounds.', 10, 0
        strWrongFormat db 'Input value must be number.', 10, 0
        fmt            db '%lf', 0
        fmtp           db '%lf', 13, 10, 0
        outputStr      db 'exp(%g) = %lf', 10, 0

        x              dq 0
        maxR           dd 709.0
        maxL           dd -5.0
        result         dq 1.0
        component      dq 1.0
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
                                          dword[result], dword[result + 4]

        finish:
                call [getch]
                push NULL
                call [ExitProcess]

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
                fcom [maxR]     ; compare x with right border 709.0
                fstsw ax
                sahf
                ja outOfBounds  ; jump to outOfBounds if x more than 709.0

        checkLeftBorder:
                fcomp [maxL]    ; compare x with left border -5.0
                fstsw ax
                sahf
                jb outOfBounds  ; jump to outOfBounds if x less than -5.0

                mov esp, [tmpStack]
                ret

;------------------------------------------------

        CalcExp:
                mov [tmpStack], esp

        expLoop:
                inc [i]          ; i++

                ; component *= x / i
                fld [x]          ; st0 = x
                fidiv [i]        ; st0 = x / i
                fmul [component] ; component *= st0
                fstp [component] ; component = st0

                ; result += component
                fld [result]     ; st0 = result
                fadd [component] ; st0 += component
                fstp [result]    ; result = st0

                fld [component]  ; st0 = component
                fabs             ; st0 = |component|
                fcomp [eps]      ; compare |component| with 0.001
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