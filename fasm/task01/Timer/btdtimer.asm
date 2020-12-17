;===============================================================================
;Program: BDTTimer
;Version: 1.1.0
;Author: Yeoh HS
;Date: Jan-Feb 2009, edited in December 2017
;Purpose: A Big Text Display timer.
;Flat Assembler version: 1.73.01
;Resources file: btdtimer.res (built with Pelles C resources editor)
;===============================================================================
format PE GUI 4.0
entry start

include 'win32axp.inc'
include '\macro\if.inc'

;-------------------------------------------------------------------------------
struct MYSYSTEMTIME
  wYear         dw ?
  wMonth        dw ?
  wDayOfWeek    dw ?
  wDay          dw ?
  wHour         dw ?
  wMinute       dw ?
  wSecond       dw ?
  wMilliseconds dw ?
ends

;-------------------------------------------------------------------------------
TRIAL = 0 ; 0 for full version, 1 for trial version
ERROR_ALREADY_EXISTS = 183
FONTSCALE            = 200
MAXPATH              = 260
CSIDL_APPDATA        = 26
ES_CONTINUOUS        = 0x80000000
ES_DISPLAY_REQUIRED  = 0x00000002
ES_SYSTEM_REQUIRED   = 0x00000001

SND_ASYNC            = 1
SND_RESOURCE         = 0x00040004

IDI_ICON     = 1
IDT_SPLASH   = 2
IDT_TICKER   = 3

IDD_SPLASH   = 1001
IDD_TIME     = 1002
IDD_PREFS    = 1003
IDD_ABOUT    = 1004
IDD_PLOG     = 1005

IDM_MENU     = 2001
IDM_EXIT     = 2002
IDM_TIME     = 2003
IDM_FONT     = 2004
IDM_TXCLR    = 2005
IDM_BKCLR    = 2006
IDM_PREFS    = 2007
IDM_CONTS    = 2008
IDM_ABOUT    = 2009
IDM_VPLOG    = 2010

IDA_MAIN     = 3001

IDC_START    = 3002
IDC_STOP     = 3004
IDC_RESET    = 3005
IDC_PAUSE    = 3006

IDC_ICON     = 4001
IDC_HH       = 4011
IDC_MM       = 4012
IDC_SS       = 4013

IDC_SPLASHON = 4001
IDC_PLOGON   = 4002

IDC_XSTART   = 4005
IDC_YBUTTONS = 4006
IDC_YCOUNTER = 4007
IDC_ALARMWAV = 4008
IDC_BTNALARMWAV = 4009
IDC_TICKON   = 4012
IDC_ALARMON  = 4003
IDC_TICKWAV  = 4004
IDC_BTNTICKWAV = 4010

IDS_ALARM    = 8001
IDS_TICK     = 8002

IDC_PLOG     = 4002
IDC_BROWSE   = 4003
IDC_DELLOG   = 4004

IDC_WEBSITE  = 4003

;-------------------------------------------------------------------------------
section '.data' data readable writeable
winclass     db 'BTDTimer',0
wintitle     db 'BTDTimer',0
winerror     db 'BTDTimer startup failed!',0
wc           WNDCLASS 0,WindowProc,0,0,NULL,NULL,NULL,COLOR_BTNFACE+1,IDM_MENU,winclass
msg          MSG
ghwnd        dd ?
ginst        dd ?
hacc         dd ?
htimer       dd ?
rc           RECT
hCounter     dd ?
hFont        dd ?
hSound       dd ?
hTick        dd ?
hbStart      dd ?
hbPause      dd ?
hbStop       dd ?
hbReset      dd ?
htCounter    dd ?
mycount      dd 0
pausedcount  dd 0
strmycount   rb 80
tickinterval dd 1000
countmax     dd 300
myfmt        db '%.2d:%.2d:%.2d',0
yButtons     dd ?
yCounter     dd ?
xStart       dd ?
xPause       dd ?
xStop        dd ?
xReset       dd ?
fheight      dd ?
fweight      dd ?
fitalic      dd ?
FontName     rb 32
TimeLimit    dd ?

lgfont       LOGFONT
hcolortx     dd ?
hcolorbk     dd ?
COLORREF     dd 16 dup 0
hbrush       dd ?

lpOperation  db 'open',0
lpHelpFile   db 'http://flatassembler.net/',0  ;obsolete file removed
lpWebsite    db 'http://flatassembler.net/',0  ;obsolete link removed
lpParameters dd NULL
lpDirectory  db '',0

ssActive     dd ?
TimeHH       dd ?
TimeMM       dd ?
TimeSS       dd ?

lpBuffer     rb 16
lpFmt        db '%d',0
exwintitle   rb 80
TimeFmt      db 'BTDTimer: Timer Set - %.2dh:%.2dm:%.2ds',0

MyAppFolder      db 'btdtimer',0
cfgfp            dd 0
cfgwritemode     db 'wb',0
cfgreadmode      db 'rb',0
cfgstr           db '%s',0Ah,0
cfgnum           db '%d',0Ah,0
CfgFolderPath    rb MAXPATH
CfgFile          db '/btdtimer.cfg',0
txtline          rb MAXPATH
txtlen           dd ?
DefaultFontName  db 'Arial',0
NoFileSelected   db 'No file selected',0
AlarmWav         rb MAXPATH
TickWav          rb MAXPATH
lpwavFilter      db 'WAV Files',0,'*.wav',0

SplashOn         dd ?
PLogOn           dd ?
TickOn           dd ?
AlarmOn          dd ?

LogFolderPath    rb MAXPATH
LogFolderFull    rb MAXPATH
LogFile          db '/btdtimerlog.txt',0
DateTimeBuffer   rb 80

logwritemode     db 'ab+',0
logfp            dd ?
logstr           db '%s',0Ah,0
LogText          rb MAXPATH
logline          rb MAXPATH

prefsbuffer      rb MAXPATH
prefsfmt         db 'Maximum exceeded! Value set to %d',0

;-------------------------------------------------------------------------------
section '.code' code readable executable
start:
     invoke  GetModuleHandleA,0
     mov     [wc.hInstance],eax
     mov     [ginst], eax
     invoke CreateMutexA,NULL,TRUE,'BTDTimerInstanceIsRunning'
     invoke GetLastError
     .if eax = ERROR_ALREADY_EXISTS
         invoke MessageBoxA,0,'An instance of this program is already running.',\
                             'Attention',\
                             MB_ICONINFORMATION+MB_OK+MB_SYSTEMMODAL
         invoke FindWindowA,winclass,NULL
         invoke SetForegroundWindow,eax
         jmp end_loop
     .endif
     invoke  LoadIconA,[ginst],IDI_ICON
     mov     [wc.hIcon],eax
     invoke  LoadCursorA,0,IDC_ARROW
     mov     [wc.hCursor],eax
     invoke  RegisterClassA,wc
     test    eax,eax
     jz      error
     invoke  LoadAcceleratorsA,[ginst],IDA_MAIN
     mov     [hacc],eax
     invoke  SystemParametersInfoA,SPI_GETWORKAREA,0,rc,0
     invoke  CreateWindowExA,0,winclass,wintitle,\
             WS_CAPTION+WS_SYSMENU+WS_MINIMIZEBOX,\
             0,0,[rc.right],[rc.bottom],NULL,NULL,[wc.hInstance],NULL
     test    eax,eax
     jz      error
     mov     [ghwnd],eax
     invoke  ShowWindow, [ghwnd], SW_SHOW
     invoke  UpdateWindow, [ghwnd]
msg_loop:
     invoke  GetMessageA,msg,NULL,0,0
     or      eax,eax
     jz      end_loop
     invoke  TranslateAccelerator,[ghwnd],[hacc],msg
     or      eax,eax
     jnz     msg_loop
     cmp     [msg.message],WM_KEYDOWN
     je      msg_dispatch
     invoke  TranslateMessage,msg
msg_dispatch:
     invoke  DispatchMessageA,msg
     jmp     msg_loop
error:
     invoke  MessageBoxA,NULL,winerror,NULL,MB_ICONERROR+MB_OK
end_loop:
     invoke  ExitProcess,[msg.wParam]

;-------------------------------------------------------------------------------
proc WindowProc hwnd,wmsg,wparam,lparam
     push    ebx esi edi
     cmp     [wmsg],WM_CREATE
     je      .wmcreate
     cmp     [wmsg],WM_TIMER
     je      .wmtimer
     cmp     [wmsg],WM_COMMAND
     je      .wmcommand
     cmp     [wmsg],WM_CTLCOLORSTATIC
     je      .wmctlcolorstatic
     cmp     [wmsg],WM_ACTIVATE
     je      .wmactivate
     cmp     [wmsg],WM_DESTROY
     je      .wmdestroy
     jmp     .defwndproc
.wmcreate:
     ;
     invoke SHGetSpecialFolderPathA,[ghwnd],CfgFolderPath,CSIDL_APPDATA,0
     invoke PathAppendA,CfgFolderPath,MyAppFolder
     invoke CreateDirectoryA,CfgFolderPath,NULL
     cinvoke strcat,CfgFolderPath,CfgFile
     stdcall readcfgfile

     ;
     invoke SHGetSpecialFolderPathA,[ghwnd],LogFolderPath,CSIDL_APPDATA,0
     invoke PathAppendA,LogFolderPath,MyAppFolder
     invoke CreateDirectoryA,LogFolderPath,NULL
     cinvoke strcpy,LogFolderFull,LogFolderPath
     cinvoke strcat,LogFolderFull,LogFile
     ;
     .if [SplashOn] = TRUE
         invoke DialogBoxParamA,[ginst],IDD_SPLASH,NULL,splashproc,NULL
     .endif
     ;
     if TRIAL
        invoke MessageBoxA,0,'Thank you for trying out this program.','Trial Version Notice',MB_ICONINFORMATION+MB_OK
     end if
     ;
     .if [PLogOn] = TRUE
         stdcall LogSessionStart
     .endif
     ;
     invoke SystemParametersInfoA,SPI_GETSCREENSAVEACTIVE,0,ssActive,0
     .if [ssActive] = TRUE
         invoke SystemParametersInfoA,SPI_SETSCREENSAVEACTIVE,FALSE,0,0
     .else
         mov [ssActive], FALSE
     .endif
     ;
     invoke SetThreadExecutionState, ES_CONTINUOUS+ES_DISPLAY_REQUIRED+ES_SYSTEM_REQUIRED
     ;
     mov     ebx,[xStart]
     add     ebx,90
     mov     [xPause],ebx
     add     ebx,90
     mov     [xStop],ebx
     add     ebx,90
     mov     [xReset],ebx

     stdcall HHMMSS2Secs,[TimeHH],[TimeMM],[TimeSS]
     mov     [TimeLimit],eax
     cinvoke sprintf,exwintitle,TimeFmt,[TimeHH],[TimeMM],[TimeSS]
     invoke SetWindowTextA,[hwnd],exwintitle

     mov     [hSound],IDS_ALARM
     mov     [hTick],IDS_TICK

     invoke  CreateWindowExA,0,'button','Start',WS_CHILD+WS_VISIBLE+WS_TABSTOP+BS_PUSHBUTTON,\
                            [xStart],[yButtons],80,40,[hwnd],IDC_START,[ginst],NULL
     mov     [hbStart],eax
     invoke  CreateWindowExA,0,'button','Pause',WS_CHILD+WS_VISIBLE+WS_TABSTOP+BS_PUSHBUTTON,\
                            [xPause],[yButtons],80,40,[hwnd],IDC_PAUSE,[ginst],NULL
     mov     [hbPause],eax
     invoke  CreateWindowExA,0,'button','Stop',WS_CHILD+WS_VISIBLE+WS_TABSTOP+BS_PUSHBUTTON,\
                            [xStop],[yButtons],80,40,[hwnd],IDC_STOP,[ginst],NULL
     mov     [hbStop],eax
     invoke  CreateWindowExA,0,'button','Reset',WS_CHILD+WS_VISIBLE+WS_TABSTOP+BS_PUSHBUTTON,\
                            [xReset],[yButtons],80,40,[hwnd],IDC_RESET,[ginst],NULL
     mov     [hbReset],eax
     invoke  CreateWindowExA,0,'edit',NULL,WS_CHILD+WS_VISIBLE+ES_READONLY+ES_CENTER,\
                            0,[yCounter],[rc.right],[rc.bottom],[hwnd],-1,[ginst],NULL
     mov     [hCounter],eax
     invoke  CreateFontA,[fheight],0,0,0,[fweight],[fitalic],0,0, ANSI_CHARSET,\
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,\
                        DEFAULT_QUALITY,DEFAULT_PITCH or FF_DONTCARE,\
                        FontName
     mov     [hFont],eax
     invoke  SendMessageA,[hCounter],WM_SETFONT,[hFont],FALSE

     invoke  EnableWindow,[hbStart],TRUE
     invoke  EnableWindow,[hbPause],FALSE
     invoke  EnableWindow,[hbStop],FALSE
     invoke  EnableWindow,[hbReset],FALSE

     mov     [mycount],0
     stdcall Secs2HHMMSS,[mycount],strmycount
     invoke  SetWindowTextA,[hCounter],strmycount

     stdcall SetWinColor,[hwnd],[hcolorbk]
     invoke InvalidateRect,[hwnd],NULL,TRUE

     jmp    .finish
.wmcommand:
     mov    eax,[wparam]
     mov    ebx,eax
     and    eax, 0FFFFh
     shr    ebx,16
     .if eax = IDM_EXIT
         invoke SendMessageA,[hwnd],WM_CLOSE,0,0
     .elseif eax = IDM_TIME
         invoke DialogBoxParamA,[ginst],IDD_TIME,[hwnd],timeproc,NULL
     .elseif eax = IDM_FONT
         stdcall SelectFont
         invoke DeleteObject,[hFont]  ; to avoid memory leak
         invoke CreateFontA,[fheight],0,0,0,[fweight],[fitalic],0,0,ANSI_CHARSET,\
                           OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,\
                           DEFAULT_QUALITY,DEFAULT_PITCH or FF_DONTCARE,\
                           FontName
         mov [hFont],eax
         invoke  SendMessageA,[hCounter],WM_SETFONT,[hFont],FALSE
         stdcall writecfgfile
         stdcall readcfgfile
     .elseif eax = IDM_TXCLR
         stdcall SelectTextColor
         stdcall writecfgfile
         stdcall readcfgfile
     .elseif eax = IDM_BKCLR
         stdcall SelectBackColor
         stdcall writecfgfile
         stdcall readcfgfile
         stdcall SetWinColor,[ghwnd],[hcolorbk]
         invoke  InvalidateRect,[hwnd],NULL,TRUE
     .elseif eax = IDM_VPLOG
         invoke DialogBoxParamA,[ginst],IDD_PLOG,[hwnd],plogproc,NULL
     .elseif eax = IDM_PREFS
         invoke DialogBoxParamA,[ginst],IDD_PREFS,[hwnd],prefsproc,NULL
     .elseif eax = IDM_CONTS
         invoke ShellExecuteA,[hwnd],lpOperation,lpHelpFile,lpParameters,lpDirectory,SW_SHOWMAXIMIZED
         .if eax <= 32
             invoke MessageBoxA,0,"Unable to open the program's help file",'Error!',MB_ICONERROR+MB_OK
         .endif
     .elseif eax = IDM_ABOUT
         invoke DialogBoxParamA,[ginst],IDD_ABOUT,[hwnd],aboutproc,NULL
     .elseif eax = IDC_START
         .if [TimeLimit] = 0
             invoke MessageBoxA,0,'Please set the timer.','Error! Timer not set.',MB_ICONERROR+MB_OK
             invoke DialogBoxParamA,[ginst],IDD_TIME,[hwnd],timeproc,NULL
             jmp .finish
         .endif
         .if [pausedcount] > 0
             mov ebx,[pausedcount]
             mov [mycount],ebx
         .else
             mov [mycount],0
         .endif
         stdcall Secs2HHMMSS,[mycount],strmycount
         invoke SetWindowTextA,[hCounter],strmycount
         invoke SetTimer,[hwnd],IDT_TICKER,[tickinterval],0
         mov    [htCounter],eax
         invoke EnableWindow,[hbPause],TRUE
         invoke EnableWindow,[hbStop],TRUE
         invoke EnableWindow,[hbStart],FALSE
         invoke EnableWindow,[hbReset],FALSE
         .if [PLogOn] = TRUE
             stdcall MakeTimeStamp,DateTimeBuffer
             stdcall MakeLogEntry,DateTimeBuffer,' - Start ',exwintitle
             stdcall WriteLog
         .endif
     .elseif eax = IDC_PAUSE
         mov    ebx,[mycount]
         mov    [pausedcount],ebx
         invoke KillTimer,[hwnd],[htCounter]
         invoke EnableWindow,[hbPause],FALSE
         invoke EnableWindow,[hbStop],FALSE
         invoke EnableWindow,[hbStart],TRUE
         invoke EnableWindow,[hbReset],TRUE
         .if [PLogOn] = TRUE
             stdcall MakeTimeStamp,DateTimeBuffer
             stdcall MakeLogEntry,DateTimeBuffer,' - Timer paused.',''
             stdcall WriteLog
         .endif
     .elseif eax = IDC_STOP
         mov    [pausedcount],0
         invoke KillTimer,[hwnd],[htCounter]
         invoke EnableWindow,[hbPause],FALSE
         invoke EnableWindow,[hbStop],FALSE
         invoke EnableWindow,[hbStart],TRUE
         invoke EnableWindow,[hbReset],TRUE
         .if [PLogOn] = TRUE
             stdcall MakeTimeStamp,DateTimeBuffer
             stdcall MakeLogEntry,DateTimeBuffer,' - Timer stopped.',''
             stdcall WriteLog
         .endif
     .elseif eax = IDC_RESET
         invoke KillTimer,[hwnd],[htCounter]
         invoke EnableWindow,[hbPause],FALSE
         invoke EnableWindow,[hbStop],FALSE
         invoke EnableWindow,[hbStart],TRUE
         invoke EnableWindow,[hbReset],FALSE
         invoke SetWindowTextA,[hCounter],NULL
         mov    [mycount],0
         mov    [pausedcount],0
         stdcall Secs2HHMMSS,[mycount],strmycount
         invoke SetWindowTextA,[hCounter],strmycount
         .if [PLogOn] = TRUE
             stdcall MakeTimeStamp,DateTimeBuffer
             stdcall MakeLogEntry,DateTimeBuffer,' - Timer reset.',''
             stdcall WriteLog
         .endif
     .elseif ebx = EN_SETFOCUS   ;if user clicks on edit control
         invoke SetFocus,[hwnd]  ;avoid focus in edit control
     .endif
     jmp .finish
.wmtimer:
     mov ebx,[TimeLimit]
     inc [mycount]
     .if [mycount] = ebx
         stdcall Secs2HHMMSS,[mycount],strmycount
         invoke  SetWindowTextA,[hCounter],strmycount
         invoke KillTimer,[hwnd],[htCounter]
         invoke EnableWindow,[hbStop],FALSE
         invoke EnableWindow,[hbStart],TRUE
         invoke EnableWindow,[hbReset],TRUE
         .if [AlarmOn] = TRUE
             stdcall SoundOut,[hSound],AlarmWav
         .endif
         invoke ShowWindow,[hwnd],SW_RESTORE
         .if [PLogOn] = TRUE
             stdcall MakeTimeStamp,DateTimeBuffer
             stdcall MakeLogEntry,DateTimeBuffer,' - Timer limit reached.',''
             stdcall WriteLog
         .endif
     .else
         .if [TickOn] = TRUE
             stdcall SoundOut,[hTick],TickWav
         .endif
         stdcall Secs2HHMMSS,[mycount],strmycount
         invoke  SetWindowTextA,[hCounter],strmycount
     .endif
     jmp    .finish
.wmctlcolorstatic:
     mov ebx,[lparam]
     .if ebx = [hCounter]
         invoke SetBkColor,[wparam],[hcolorbk]
         invoke SetTextColor,[wparam],[hcolortx]  ; set edit control's text color
         invoke DeleteObject,[hbrush] ; to avoid memory leak
         invoke CreateSolidBrush,[hcolorbk]; set edit control's background color
         mov    [hbrush],eax ; store object so that it can be deleted later to avoid memory leak
     .endif
     jmp .finish
.wmactivate:
     stdcall SetWinColor,[hwnd],[hcolorbk]
     invoke InvalidateRect,[hwnd],NULL,TRUE
     jmp .finish
.defwndproc:
     invoke  DefWindowProcA,[hwnd],[wmsg],[wparam],[lparam]
     jmp     .finish
.wmdestroy:
     .if [PLogOn] = TRUE
         stdcall LogSessionEnd
     .endif
     .if [ssActive] = TRUE
         invoke SystemParametersInfoA,SPI_SETSCREENSAVEACTIVE,TRUE,0,0
     .endif
     invoke  KillTimer,[hwnd],[htCounter]
     invoke  DeleteObject,[hbrush]
     invoke  DeleteObject,[hFont]  ; to avoid memory leak
     ;
     if TRIAL
        invoke MessageBoxA,0,'Thank you for trying out this program.','Trial Version Notice',MB_ICONINFORMATION+MB_OK
     end if
     invoke  PostQuitMessage,0
     xor     eax,eax
.finish:
     pop     edi esi ebx
     ret
endp

;-------------------------------------------------------------------------------
proc plogproc,hwnd,umsg,wparam,lparam
     push edi esi ebx
     mov  eax,[umsg]
     cmp  eax,WM_INITDIALOG
     je   .wminitdlg
     cmp  eax,WM_COMMAND
     je   .wmcommand
     cmp  eax,WM_CLOSE
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.wminitdlg:
     invoke SendDlgItemMessageA,[hwnd],IDC_PLOG,LB_SETHORIZONTALEXTENT,2048,0
     stdcall ReadLog,[hwnd]
     jmp .processed
.wmcommand:
     mov eax,[wparam]
     cmp eax,IDC_BROWSE
     je  .onBrowse
     cmp eax,IDC_DELLOG
     je  .onDeleteLog
     cmp eax,IDOK
     je  .onIDOK
     xor eax,eax
     jmp .finish
.onBrowse:
     invoke ShellExecuteA,[hwnd],'explore',LogFolderPath,NULL,NULL,SW_SHOWNORMAL
     jmp .finish
.onDeleteLog:
     invoke MessageBoxA,0,'Are you sure?','Confirm deletion of Program Log',MB_ICONWARNING+MB_OKCANCEL
     .if eax = IDOK
         invoke DeleteFileA,LogFolderFull
         stdcall MakeTimeStamp,DateTimeBuffer
         stdcall MakeLogEntry,DateTimeBuffer,' - New log started.',''
         stdcall WriteLog
         invoke SendDlgItemMessageA,[hwnd],IDC_PLOG,LB_RESETCONTENT,0,0
         stdcall ReadLog,[hwnd]
     .endif
     jmp .finish
.onIDOK:
     invoke EndDialog,[hwnd],0
.wmclose:
     invoke EndDialog,[hwnd],0
.processed:
     mov eax,1
.finish:
     pop ebx esi edi
     ret
endp

;-------------------------------------------------------------------------------
proc splashproc,hwnd,umsg,wparam,lparam
local SplashDelay:DWORD
     push edi esi ebx
     mov  eax,[umsg]
     cmp  eax,WM_INITDIALOG
     je   .wminitdlg
     cmp  eax,WM_COMMAND
     je   .wmcommand
     cmp  eax,WM_TIMER
     je   .wmtimer
     cmp  eax,WM_CLOSE
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.wminitdlg:
     mov [SplashDelay],1500
     invoke GetDlgItem,[hwnd],IDC_ICON
     invoke SendMessageA,eax,STM_SETICON,[wc.hIcon],0
     invoke SetTimer,[hwnd],IDT_SPLASH,[SplashDelay],0
     mov    [htimer],eax
     jmp   .processed
.wmcommand:
     mov eax,[wparam]
     xor eax,eax
     jmp .finish
.wmtimer:
     invoke SendMessageA,[hwnd],WM_CLOSE,0,0
     jmp .processed
.wmclose:
     invoke KillTimer,[hwnd],[htimer]
     invoke EndDialog,[hwnd],0
.processed:
     mov eax,1
.finish:
     pop ebx esi edi
     ret
endp

;-------------------------------------------------------------------------------
proc timeproc,hwnd,umsg,wparam,lparam
     push edi esi ebx
     mov  eax,[umsg]
     cmp  eax,WM_INITDIALOG
     je   .wminitdlg
     cmp  eax,WM_COMMAND
     je   .wmcommand
     cmp  eax,WM_CLOSE
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.wminitdlg:
     invoke SetDlgItemInt,[hwnd],IDC_HH,[TimeHH],FALSE
     invoke SetDlgItemInt,[hwnd],IDC_MM,[TimeMM],FALSE
     invoke SetDlgItemInt,[hwnd],IDC_SS,[TimeSS],FALSE
     jmp  .processed
.wmcommand:
     mov  eax,[wparam]
     cmp  eax,IDOK
     je   .onIDOK
     xor  eax,eax
     jmp  .finish
.onIDOK:
     invoke GetDlgItemInt,[hwnd],IDC_HH,NULL,FALSE
     mov [TimeHH],eax
     .if [TimeHH] > 99
         invoke MessageBoxA,0,'Maximum value is 99 hours','Maximum Hours Value',MB_ICONWARNING+MB_OK
         mov [TimeHH],99
     .endif
     invoke GetDlgItemInt,[hwnd],IDC_MM,NULL,FALSE
     mov [TimeMM],eax
     .if [TimeMM] > 59
         invoke MessageBoxA,0,'Maximum value is 59 minutes!','Maximum Minutes Value',MB_ICONWARNING+MB_OK
         mov [TimeMM],59
     .endif
     invoke GetDlgItemInt,[hwnd],IDC_SS,NULL,FALSE
     mov [TimeSS],eax
     .if [TimeSS] > 59
         invoke MessageBoxA,0,'Maximum value is 59 seconds!','Maximum Seconds Value',MB_ICONWARNING+MB_OK
         mov [TimeSS], 59
     .endif
     stdcall HHMMSS2Secs,[TimeHH],[TimeMM],[TimeSS]
     mov [TimeLimit],eax
     cinvoke sprintf,exwintitle,TimeFmt,[TimeHH],[TimeMM],[TimeSS]
     invoke SetWindowTextA,[ghwnd],exwintitle
     stdcall writecfgfile
     stdcall readcfgfile
     invoke EndDialog,[hwnd],0
.wmclose:
     invoke EndDialog,[hwnd],0
.processed:
     mov eax,1
.finish:
     pop ebx esi edi
     ret
endp

;-------------------------------------------------------------------------------
proc ShowInt,IntVal,DescVal
     cinvoke sprintf,lpBuffer,lpFmt,[IntVal]
     invoke MessageBoxA,0,lpBuffer,[DescVal],MB_OK
     ret
endp

;-------------------------------------------------------------------------------
proc prefsproc,hwnd,umsg,wparam,lparam
     push edi esi ebx
     mov  eax,[umsg]
     cmp  eax,WM_INITDIALOG
     je   .wminitdlg
     cmp  eax,WM_COMMAND
     je   .wmcommand
     cmp  eax,WM_CLOSE
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.wminitdlg:
     invoke SendDlgItemMessageA,[hwnd],IDC_SPLASHON,BM_SETCHECK,[SplashOn],0
     invoke SendDlgItemMessageA,[hwnd],IDC_PLOGON,BM_SETCHECK,[PLogOn],0
     invoke SendDlgItemMessageA,[hwnd],IDC_TICKON,BM_SETCHECK,[TickOn],0
     invoke SendDlgItemMessageA,[hwnd],IDC_ALARMON,BM_SETCHECK,[AlarmOn],0

     invoke SetDlgItemInt,[hwnd],IDC_XSTART,[xStart],FALSE
     invoke SetDlgItemInt,[hwnd],IDC_YBUTTONS,[yButtons],FALSE
     invoke SetDlgItemInt,[hwnd],IDC_YCOUNTER,[yCounter],FALSE
     invoke SetDlgItemTextA,[hwnd],IDC_ALARMWAV,AlarmWav
     invoke SetDlgItemTextA,[hwnd],IDC_TICKWAV,TickWav
     jmp  .processed
.wmcommand:
     mov  eax,[wparam]
     cmp  eax,IDC_BTNTICKWAV
     je   .onBtnTickWav
     cmp  eax,IDC_BTNALARMWAV
     je   .onBtnAlarmWav
     cmp  eax,IDOK
     je   .onIDOK
     xor  eax,eax
     jmp  .finish
.onBtnTickWav:
     stdcall SelectFile,lpwavFilter,TickWav
     .if eax = 1
         invoke SetDlgItemTextA,[hwnd],IDC_TICKWAV,TickWav
     .else
         invoke SetDlgItemTextA,[hwnd],IDC_TICKWAV,NoFileSelected
     .endif
     jmp  .processed
.onBtnAlarmWav:
     stdcall SelectFile,lpwavFilter,AlarmWav
     .if eax = 1
         invoke SetDlgItemTextA,[hwnd],IDC_ALARMWAV,AlarmWav
     .else
         invoke SetDlgItemTextA,[hwnd],IDC_ALARMWAV,NoFileSelected
     .endif
     jmp  .processed
.onIDOK:
     invoke SendDlgItemMessageA,[hwnd],IDC_SPLASHON,BM_GETCHECK,0,0
     mov    [SplashOn],eax
     invoke SendDlgItemMessageA,[hwnd],IDC_PLOGON,BM_GETCHECK,0,0
     mov    [PLogOn],eax
     invoke SendDlgItemMessageA,[hwnd],IDC_TICKON,BM_GETCHECK,0,0
     mov    [TickOn],eax
     invoke SendDlgItemMessageA,[hwnd],IDC_ALARMON,BM_GETCHECK,0,0
     mov    [AlarmOn],eax

     invoke GetDlgItemInt,[hwnd],IDC_XSTART,NULL,FALSE
     mov  [xStart],eax
     mov  ebx,[rc.right]
     sub  ebx,360
     .if  [xStart] > ebx
          mov [xStart],ebx
          cinvoke wsprintfA,prefsbuffer,prefsfmt,[xStart]
          invoke MessageBoxA,0,prefsbuffer,'Buttons position from left',MB_ICONINFORMATION+MB_OK
     .endif
     invoke GetDlgItemInt,[hwnd],IDC_YBUTTONS,NULL,FALSE
     mov  [yButtons],eax
     mov  ebx,[yCounter]
     sub  ebx,45
     .if  [yButtons] > ebx
          mov [yButtons],ebx
          cinvoke wsprintfA,prefsbuffer,prefsfmt,[yButtons]
          invoke MessageBoxA,0,prefsbuffer,'Buttons position from top',MB_ICONINFORMATION+MB_OK
     .endif
     invoke GetDlgItemInt,[hwnd],IDC_YCOUNTER,NULL,FALSE
     mov  [yCounter],eax
     mov  ebx,[rc.bottom]
     sub  ebx,350
     .if  [yCounter] > ebx
          mov [yCounter], ebx
          cinvoke wsprintfA,prefsbuffer,prefsfmt,[yCounter]
          invoke MessageBoxA,0,prefsbuffer,'Timer display position from top',MB_ICONINFORMATION+MB_OK
     .endif
     invoke GetDlgItemTextA,[hwnd],IDC_ALARMWAV,AlarmWav,MAXPATH
     invoke GetDlgItemTextA,[hwnd],IDC_TICKWAV,TickWav,MAXPATH
     mov     ebx,[xStart]
     add     ebx,90
     mov     [xPause],ebx
     add     ebx,90
     mov     [xStop],ebx
     add     ebx,90
     mov     [xReset],ebx
     invoke MoveWindow,[hbStart],[xStart],[yButtons],80,40,TRUE
     invoke MoveWindow,[hbPause],[xPause],[yButtons],80,40,TRUE
     invoke MoveWindow,[hbStop], [xStop], [yButtons],80,40,TRUE
     invoke MoveWindow,[hbReset],[xReset],[yButtons],80,40,TRUE
     invoke MoveWindow,[hCounter],0,[yCounter],[rc.right],[rc.bottom],TRUE
     if TRIAL
         invoke MessageBoxA,0,'Saving of Preferences disabled!','Trial Version Limitation',MB_ICONINFORMATION+MB_OK
     else
         stdcall writecfgfile
         stdcall readcfgfile
     end if
     invoke EndDialog,[hwnd],0
.wmclose:
     invoke EndDialog,[hwnd],0
.processed:
     mov eax,1
.finish:
     pop ebx esi edi
     ret
endp

;-------------------------------------------------------------------------------
proc aboutproc,hwnd,umsg,wparam,lparam
     push edi esi ebx
     mov  eax,[umsg]
     cmp  eax,WM_INITDIALOG
     je   .wminitdlg
     cmp  eax,WM_COMMAND
     je   .wmcommand
     cmp  eax,WM_CLOSE
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.wminitdlg:
     invoke GetDlgItem,[hwnd],IDC_ICON
     invoke SendMessageA,eax,STM_SETICON,[wc.hIcon],0
     jmp  .processed
.wmcommand:
     mov  eax,[wparam]
     cmp  eax,IDC_WEBSITE
     je   .onwebsite
     cmp  eax,IDOK
     je   .wmclose
     xor  eax,eax
     jmp  .finish
.onwebsite:
     invoke ShellExecuteA,[hwnd],lpOperation,lpWebsite,lpParameters,lpDirectory,SW_SHOWMAXIMIZED
     .if eax <= 32
         invoke MessageBoxA,0,"Are you online now?",'Unable to connect to website!',MB_ICONERROR+MB_OK
     .endif
     jmp  .processed
.wmclose:
     invoke EndDialog,[hwnd],0
.processed:
     mov eax,1
.finish:
     pop ebx esi edi
     ret
endp

;--------------------------------------------------------------------------------
proc SelectFont
local chfont:CHOOSEFONT
     cinvoke strcpy,lgfont.lfFaceName,FontName
     cinvoke memset,addr chfont,0,sizeof.CHOOSEFONT
     mov [chfont.lStructSize],sizeof.CHOOSEFONT
     mov ebx,lgfont
     mov [chfont.lpLogFont],ebx
     mov [chfont.Flags],CF_INITTOLOGFONTSTRUCT+CF_SCREENFONTS+CF_SCALABLEONLY+\
                        CF_NOSTYLESEL+CF_NOSIZESEL+CF_NOVECTORFONTS
     invoke ChooseFontA,addr chfont
     .if eax <> 0
         cinvoke strcpy,FontName,lgfont.lfFaceName
         mov ebx,[lgfont.lfHeight]
         mov [fheight],ebx
         sub [fheight],FONTSCALE
         mov ebx,[lgfont.lfWeight]
         mov [fweight],ebx
         mov bx,word[lgfont.lfItalic]
         mov word[fitalic],bx
     .endif
     ret
endp

;--------------------------------------------------------------------------------
proc SelectTextColor
local c:CHOOSECOLOR
     cinvoke memset,addr c,0,sizeof.CHOOSECOLOR
     mov [c.lStructSize],sizeof.CHOOSECOLOR
     mov ebx, [hcolortx]
     mov [c.rgbResult], ebx
     mov [c.lpCustColors], COLORREF
     mov [c.Flags], CC_RGBINIT or CC_FULLOPEN
     invoke ChooseColorA, addr c
     .if eax <> 0
         mov ebx,[c.rgbResult]
         mov [hcolortx],ebx
     .endif
     ret
endp

;--------------------------------------------------------------------------------
proc SelectBackColor
local c:CHOOSECOLOR
     cinvoke memset,addr c,0,sizeof.CHOOSECOLOR
     mov [c.lStructSize],sizeof.CHOOSECOLOR
     mov ebx,[hcolorbk]
     mov [c.rgbResult], ebx
     mov [c.lpCustColors], COLORREF
     mov [c.Flags], CC_RGBINIT or CC_FULLOPEN
     invoke ChooseColorA,addr c
     .if eax <> 0
         mov ebx,[c.rgbResult]
         mov [hcolorbk],ebx
     .endif
     ret
endp

;--------------------------------------------------------------------------------
proc SetWinColor,hWnd,Kolor
     invoke DeleteObject,[hbrush]
     invoke CreateSolidBrush,[Kolor]
     mov    [hbrush],eax
     invoke SetClassLongA,[hWnd],GCL_HBRBACKGROUND,[hbrush]
     invoke DeleteObject,eax
     invoke RedrawWindow,[hWnd],0,0,0
     ret
endp

;--------------------------------------------------------------------------------
proc Secs2HHMMSS,NumSecs,MyBuffer
local dividend:DWORD,remainder:DWORD,hh:DWORD,mm:DWORD,ssec:DWORD
    mov [dividend],0
    mov [remainder],0
    mov [hh],0
    mov [mm],0
    mov [ssec],0

    xor edx, edx
    mov eax, [NumSecs]
    mov ebx, 60
    div ebx
    mov [dividend], eax
    mov [remainder], edx
    push [remainder]
    pop [ssec]
    .if [dividend] < 60
        push [dividend]
        pop [mm]
    .else
        xor edx,edx
        mov eax,[dividend]
        mov ebx,60
        div ebx
        mov [dividend], eax
        mov [remainder], edx
        push [dividend]
        pop [hh]
        push [remainder]
        pop [mm]
    .endif
    cinvoke sprintf,[MyBuffer],myfmt,[hh],[mm],[ssec]
    ret
endp

;--------------------------------------------------------------------------------
proc HHMMSS2Secs,Hour,Min,Secs
local result1:DWORD, result2:DWORD, result3:DWORD
     mov [result1],0
     mov [result2],0
     mov [result3],0

     mov eax, [Hour]
     mov edx, 3600
     mul edx
     mov [result1], eax

     mov eax, [Min]
     mov edx, 60
     mul edx
     mov [result2], eax

     push [Secs]
     pop [result3]

     mov eax, [result1]
     mov ebx, [result2]
     add [result1], ebx

     mov eax, [result1]
     mov ebx, [result3]
     add [result1], ebx
     mov eax, [result1]
     ret
endp

;-------------------------------------------------------------------------------
proc readcfgfile
     cinvoke fopen, CfgFolderPath, cfgreadmode
     mov [cfgfp], eax
     .if [cfgfp] = NULL ; if there is no config file
         stdcall LoadDefaultPreferences
         stdcall writecfgfile
         jmp .finished
     .endif
     cinvoke fscanf, [cfgfp], cfgnum, SplashOn
     cinvoke fscanf, [cfgfp], cfgnum, PLogOn
     cinvoke fscanf, [cfgfp], cfgnum, TickOn
     cinvoke fscanf, [cfgfp], cfgnum, AlarmOn
     cinvoke fscanf, [cfgfp], cfgnum, xStart
     cinvoke fscanf, [cfgfp], cfgnum, yButtons
     cinvoke fscanf, [cfgfp], cfgnum, yCounter
     cinvoke fscanf, [cfgfp], cfgnum, TimeHH
     cinvoke fscanf, [cfgfp], cfgnum, TimeMM
     cinvoke fscanf, [cfgfp], cfgnum, TimeSS
     cinvoke fscanf, [cfgfp], cfgnum, hcolortx
     cinvoke fscanf, [cfgfp], cfgnum, hcolorbk
     cinvoke fscanf, [cfgfp], cfgnum, fheight
     cinvoke fscanf, [cfgfp], cfgnum, fweight
     cinvoke fscanf, [cfgfp], cfgnum, fitalic
     cinvoke fgets,  txtline,MAXPATH,[cfgfp]  ; fgets adds newline char
     cinvoke strlen, txtline
     sub eax,1
     cinvoke memmove, FontName,txtline,eax  ;removes trailing newline char
     cinvoke fgets,  txtline,MAXPATH,[cfgfp]  ; fgets adds newline char
     cinvoke strlen, txtline
     sub eax,1
     cinvoke memmove, AlarmWav,txtline,eax  ;removes trailing newline char
     cinvoke fgets,  txtline,MAXPATH,[cfgfp]  ; fgets adds newline char
     cinvoke strlen, txtline
     sub eax,1
     cinvoke memmove, TickWav,txtline,eax  ;removes trailing newline char
     cinvoke fclose, [cfgfp]
.finished:
     ret
endp

;-------------------------------------------------------------------------------
proc LoadDefaultPreferences
     mov [SplashOn], TRUE
     mov [PLogOn],TRUE
     mov [TickOn],TRUE
     mov [AlarmOn],TRUE
     mov [xStart],430
     mov [yButtons],10
     mov [yCounter],100
     mov [TimeHH],0
     mov [TimeMM],0
     mov [TimeSS],0
     mov [hcolortx],0000000h   ;0BBGGRRh  default is black text
     mov [hcolorbk],0FFFFFFh   ;white background
     mov [fheight],320
     mov [fweight],700
     mov [fitalic],0
     cinvoke strcpy,FontName,DefaultFontName
     cinvoke strcpy,AlarmWav,NoFileSelected
     cinvoke strcpy,TickWav,NoFileSelected
     ret
endp

;-------------------------------------------------------------------------------
proc writecfgfile
     cinvoke fopen,CfgFolderPath,cfgwritemode
     mov [cfgfp],eax
     .if [cfgfp] = NULL
         invoke MessageBoxA,0,'Cannot write to user preferences file!',\
                             'An unexpected error has occurred!',MB_SYSTEMMODAL+MB_OK+MB_ICONERROR
         jmp .finished
     .endif
     cinvoke fprintf, [cfgfp], cfgnum, [SplashOn]
     cinvoke fprintf, [cfgfp], cfgnum, [PLogOn]
     cinvoke fprintf, [cfgfp], cfgnum, [TickOn]
     cinvoke fprintf, [cfgfp], cfgnum, [AlarmOn]
     cinvoke fprintf, [cfgfp], cfgnum, [xStart]
     cinvoke fprintf, [cfgfp], cfgnum, [yButtons]
     cinvoke fprintf, [cfgfp], cfgnum, [yCounter]
     cinvoke fprintf, [cfgfp], cfgnum, [TimeHH]
     cinvoke fprintf, [cfgfp], cfgnum, [TimeMM]
     cinvoke fprintf, [cfgfp], cfgnum, [TimeSS]
     cinvoke fprintf, [cfgfp], cfgnum, [hcolortx]
     cinvoke fprintf, [cfgfp], cfgnum, [hcolorbk]
     cinvoke fprintf, [cfgfp], cfgnum, [fheight]
     cinvoke fprintf, [cfgfp], cfgnum, [fweight]
     cinvoke fprintf, [cfgfp], cfgnum, [fitalic]
     cinvoke fprintf, [cfgfp], cfgstr, FontName
     cinvoke fprintf, [cfgfp], cfgstr, AlarmWav
     cinvoke fprintf, [cfgfp], cfgstr, TickWav
     cinvoke fclose,  [cfgfp]
.finished:
     ret
endp

;-------------------------------------------------------------------------------
proc SoundOut,hSoundType,SoundFile
     cinvoke strcmp,[SoundFile],NoFileSelected
     .if eax <> 0
         invoke PlaySound,[SoundFile],NULL,SND_ASYNC
     .else
         invoke PlaySound,[hSoundType],[ginst],SND_ASYNC+SND_RESOURCE
     .endif
     ret
endp

;-------------------------------------------------------------------------------
proc SelectFile,FileFilter,FileSelected
local ofn:OPENFILENAME
     cinvoke memset,addr ofn,0,sizeof.OPENFILENAME
     mov [ofn.lStructSize],sizeof.OPENFILENAME
     mov ebx, [ghwnd]
     mov [ofn.hwndOwner], ebx
     mov ebx, [FileFilter]
     mov [ofn.lpstrFilter], ebx
     mov [ofn.nFilterIndex], 1
     mov ebx,[FileSelected]
     mov [ofn.lpstrFile],ebx
     mov [ofn.nMaxFile],MAXPATH
     mov [ofn.lpstrFileTitle],NULL
     mov [ofn.nMaxFileTitle],0
     mov [ofn.lpstrInitialDir],NULL
     mov [ofn.Flags],OFN_FILEMUSTEXIST+OFN_PATHMUSTEXIST+\
                     OFN_LONGNAMES+OFN_EXPLORER+OFN_HIDEREADONLY
     invoke GetOpenFileNameA, addr ofn
     .if eax <> 0
          mov eax,1
     .else
          mov eax,0
     .endif
     ret
endp

;-------------------------------------------------------------------------------
proc LogSessionStart
     stdcall MakeTimeStamp,DateTimeBuffer
     stdcall MakeLogEntry,DateTimeBuffer,' - Session Start',''
     stdcall WriteLog
     ret
endp

;-------------------------------------------------------------------------------
proc LogSessionEnd
     stdcall MakeTimeStamp,DateTimeBuffer
     stdcall MakeLogEntry,DateTimeBuffer,' - Session End',''
     stdcall WriteLog
     ret
endp

;-------------------------------------------------------------------------------
proc LogTurnedOn
     stdcall MakeTimeStamp,DateTimeBuffer
     stdcall MakeLogEntry,DateTimeBuffer,' - Logging is enabled',''
     stdcall WriteLog
     ret
endp

;-------------------------------------------------------------------------------
proc LogTurnedOff
     stdcall MakeTimeStamp,DateTimeBuffer
     stdcall MakeLogEntry,DateTimeBuffer,' - Logging is disabled',''
     stdcall WriteLog
     ret
endp

;-------------------------------------------------------------------------------
proc MakeTimeStamp,TimeBuffer
local mst:MYSYSTEMTIME,\
      dYear:DWORD,dMonth:DWORD,dDay:DWORD,\
      dHour:DWORD,dMinute:DWORD,dSecond:DWORD

     invoke GetLocalTime, addr mst
     xor eax,eax
     mov ax,word[mst.wYear]
     mov [dYear],eax
     xor eax,eax
     mov ax,word[mst.wMonth]
     mov [dMonth],eax
     xor eax, eax
     mov ax,word[mst.wDay]
     mov [dDay],eax
     xor eax, eax
     mov ax,word[mst.wHour]
     mov [dHour],eax
     xor eax, eax
     mov ax,word[mst.wMinute]
     mov [dMinute],eax
     xor eax, eax
     mov ax,word[mst.wSecond]
     mov [dSecond],eax
     cinvoke wsprintfA,[TimeBuffer],'%.2d-%.2d-%.2d %.2d:%.2d:%.2d',\
                      [dYear],[dMonth],[dDay],[dHour],[dMinute],[dSecond]
     ret
endp

;-------------------------------------------------------------------------------
proc MakeLogEntry,TimeStampBuffer,Action,Count
     cinvoke strcpy,LogText,[TimeStampBuffer]
     cinvoke strcat,LogText,[Action]
     cinvoke strcat,LogText,[Count]
     ret
endp

;-------------------------------------------------------------------------------
proc WriteLog
     cinvoke fopen,LogFolderFull,logwritemode
     mov [logfp],eax
     .if [logfp] = NULL
         invoke MessageBoxA,0,'Cannot write to program log!',\
                             'An unexpected error has occurred!',MB_SYSTEMMODAL+MB_OK+MB_ICONERROR
         jmp .finished
     .endif
     cinvoke fprintf, [logfp], logstr, LogText
     cinvoke fclose,  [logfp]
.finished:
     ret
endp

;-------------------------------------------------------------------------------
proc ReadLog,hwnd
local eof:DWORD
     cinvoke fopen,LogFolderFull,'rb'
     mov [logfp],eax
     .if [logfp] = NULL
         invoke MessageBoxA,0,'Cannot read program log!',\
                             'An unexpected error has occurred!',MB_SYSTEMMODAL+MB_OK+MB_ICONERROR
         jmp .finished
     .endif
     .repeat
     cinvoke memset,logline,0,MAXPATH
     cinvoke fgets,txtline,MAXPATH,[cfgfp]  ; fgets adds newline char
     mov [eof],eax
     .if [eof] = NULL
         jmp .EOF
     .endif
     cinvoke strlen,txtline
     sub eax,1
     cinvoke memmove, logline,txtline,eax  ;removes trailing newline char
     invoke SendDlgItemMessageA,[hwnd],IDC_PLOG,LB_ADDSTRING,0,logline
     .until [eof] = NULL
.EOF:
     cinvoke fclose,  [logfp]
.finished:
     ret
endp

;-------------------------------------------------------------------------------
section '.idata' import data readable writeable
library kernel32, 'KERNEL32.DLL',\
        user32,   'USER32.DLL',\
        comctl32, 'COMCTL32.DLL',\
        shell32,  'SHELL32.DLL',\
        advapi32, 'ADVAPI32.DLL',\
        comdlg32, 'COMDLG32.DLL',\
        gdi32,    'GDI32.DLL',\
        wsock32,  'WSOCK32.DLL',\
        msvcrt,   'MSVCRT.DLL',\
        shlwapi,  'SHLWAPI.DLL',\
        winmm,    'WINMM.DLL'

include '\api\kernel32.inc'
include '\api\user32.inc'
include '\api\comctl32.inc'
include '\api\shell32.inc'
include '\api\advapi32.inc'
include '\api\comdlg32.inc'
include '\api\gdi32.inc'
include '\api\wsock32.inc'
 
import  msvcrt,\
        strcat,    'strcat',\
        strcpy,    'strcpy',\
        strcmp,    'strcmp',\
        strlen,    'strlen',\
        memset,    'memset',\
        memmove,   'memmove',\
        fopen,     'fopen',\
        fclose,    'fclose',\
        sprintf,   'sprintf',\
        fprintf,   'fprintf',\
        fscanf,    'fscanf',\
        fgets,     'fgets'

import  shlwapi,\
        PathAppendA, 'PathAppendA'

import  winmm,\
        PlaySound, 'PlaySound'

;-------------------------------------------------------------------------------
section '.rsrc' data readable resource from 'btdtimer.res'

; end of file ==================================================================
