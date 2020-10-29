format PE console
entry start

include 'win32a.inc'

section '.data' data readable writable

        strInput      db 'Enter x: ', 0
        strWrongInput db 'Incorrect value of x', 10, 0
        fmt           db '%lf', 0
        fmtp          db '%lf', 13, 10, 0
        outputStr     db 'exp(%lf) = %lf', 10, 0

        x             dq 0
        maxR          dd 700.0
        result        dq 1.0
        component     dq 1.0
        tmpStack      dd ?
        m             dq 1.0
        i             dd 0
        eps           dq 0.0000000000001

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

        wrongInput:
                push strWrongInput
                call [printf]
                jmp finish

;------------------------------------------------

        Input:
                mov [tmpStack], esp

                invoke printf, strInput
                invoke scanf, fmt, x

                finit
                fld [x]         ; st0 = x
                fcomp [maxR]    ; compare x with 700.0
                fstsw ax
                sahf
                ja wrongInput

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
                fcomp [eps]      ; compare st0 with 0.001
                fstsw ax
                sahf
                ja expLoop

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