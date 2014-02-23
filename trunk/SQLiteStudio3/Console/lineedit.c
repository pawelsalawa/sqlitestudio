/*  based on linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * You can find the latest source code at:
 * 
 
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2011, Arnaud Samama <asamama at trisky dot com>
 *
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ------------------------------------------------------------------------
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Switch to gets() if $TERM is something we can't support.
 * - Filter bogus Ctrl+<char> combinations.
 * - Win32 support
 *
 * Bloat:
 * - Completion?
 * - History search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * CHA (Cursor Horizontal Absolute)
 *    Sequence: ESC [ n G
 *    Effect: moves cursor to column n
 *
 * EL (Erase Line)
 *    Sequence: ESC [ n K
 *    Effect: if n is 0 or missing, clear from cursor to end of line
 *    Effect: if n is 1, clear from beginning of line to cursor
 *    Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward of n chars
 *
 * The following are used to clear the screen: ESC [ H ESC [ 2 J
 * This is actually composed of two sequences:
 *
 * cursorhome
 *    Sequence: ESC [ H
 *    Effect: moves the cursor to upper left corner
 *
 * ED2 (Clear entire screen)
 *    Sequence: ESC [ 2 J
 *    Effect: clear the whole screen
 * 
 */
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#define strcasecmp lstrcmpiA
#define snprintf _snprintf
#define strdup _strdup
#endif


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "lineedit.h"

#ifndef _WIN32
typedef int STD_FILENO_TYPE;
#else
typedef HANDLE STD_FILENO_TYPE;
#endif

#define UNUSED(var) (void)var

typedef enum {
        CURSOR_INVALID,
        CURSOR_MOVE_LEFT,
        CURSOR_MOVE_TO,
        CURSOR_ERASE_FROM,
} CursorMove;

typedef enum {
    TELNET_IDLE,
    TELNET_IAC_STARTED,
    TELNET_IAC_COMPLETE,
}TelnetState;

struct DESC_ {

#ifndef _WIN32
        struct termios raw;
        struct termios orig_termios; /* in order to restore at exit */
#endif

        int  (*init)          (DESC* fd);
        int  (*enableRawMode) (DESC* fd);
        int  (*disableRawMode)(DESC* fd);
        int  (*getcolumns)    (DESC* fd);
        int  (*movecursor)    (DESC* fd, CursorMove move, int x);
        void (*beep)          (DESC* fd);
        int  (*read)          (DESC* fd, char* buff, size_t size);
        int  (*clearScreen)   (DESC* fd);
        int  (*write)         (DESC* fd, const char* buff, size_t size);
        void (*reset)         (DESC* fd);

        /* File descriptor for I/O operations */
        STD_FILENO_TYPE inFile;
        STD_FILENO_TYPE outFile;
    FILE* inFile_fd;
        
        /* To get completion information */
        lineeditCompletionCallback *completionCallback;
    int history_len;
    char **history;
    int history_max_len;
    int columns;
    int rows;

        lineeditConfig config;
        
        /* Use to manage telnet protocol */
    unsigned char telnetCmd[256];
    int  telnetCmdByte;
    TelnetState telnetState;
    int telnetEchoRequested;
    int telnetWSRequested;

};


#define lineedit_DEFAULT_HISTORY_MAX_LEN 100
#define lineedit_MAX_LINE 4096
static char *unsupported_term[] = {"dumb","cons25",NULL};

static int rawmode = 0; /* for atexit() function to check if restore is needed*/
#ifndef _WIN32
static int atexit_registered = 0; /* register atexit just 1 time */

static DESC* globalFd=NULL;

static void lineeditAtExit(void);
#endif
int lineeditHistoryAdd(DESC* fd, const char *line);

int vtClearScreen(DESC* fd)
{
        return fd->write(fd,"\x1b[H\x1b[2J",7);
}

int vtMoveCursor(DESC* fd, CursorMove move, int col)
{
        char seq[64];
        switch (move) {
        case CURSOR_MOVE_TO:
                /* Move cursor to original position. */
                snprintf(seq,64,"\x1b[0G\x1b[%dC", (int)(col));
                if (fd->write(fd,seq,strlen(seq)) == -1) return 0;
                break;
        case CURSOR_MOVE_LEFT:
                snprintf(seq,64,"\x1b[0G");
                if (fd->write(fd,seq,strlen(seq)) == -1) return 0;
                break;
        case CURSOR_ERASE_FROM:
                snprintf(seq,64,"\x1b[0K");
                if (fd->write(fd,seq,strlen(seq)) == -1) return 0;
                break;
        default:
                break;
        }
        return 1;
}
void vtBeep(DESC* fd) {
        fd->write(fd, "\x7",1);
}

void telnetReset(DESC* fd)
{
    fd->telnetEchoRequested = 0;
    fd->telnetWSRequested  = 0;

}

#ifndef _WIN32
int unixConinit(DESC* fd)
{
   
    if (!isatty(fd->inFile)) goto fatal;
    if (!atexit_registered) {
        atexit(lineeditAtExit);
        atexit_registered = 1;
    }
    if (tcgetattr(fd->inFile,&fd->orig_termios) == -1) goto fatal;

    fd->raw = fd->orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    fd->raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    fd->raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    fd->raw.c_cflag |= (CS8);
    /* local modes - echoing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    fd->raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    fd->raw.c_cc[VMIN] = 1; fd->raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd->inFile,TCSAFLUSH,&fd->raw) < 0) goto fatal;
    rawmode = 1;
    return 1;

fatal:
    errno = ENOTTY;
    return 0;
}
int unixConDisableRawMode(DESC* fd)
{
        /* Don't even check the return value as it's too late. */
        if (rawmode && tcsetattr(fd->inFile,TCSAFLUSH,&fd->orig_termios) != -1)
                rawmode = 0;
        return 1;
}

int unixConEnableRawMode(DESC* fd)
{
        /* Don't even check the return value as it's too late. */
        if (rawmode && tcsetattr(fd->inFile,TCSAFLUSH,&fd->orig_termios) != -1)
                rawmode = 0;
        return 1;
}

int unixConGetColumns(DESC* fd)
{
        struct winsize ws;
        if (ioctl(fd->outFile, TIOCGWINSZ, &ws) == -1) return 80;
        return ws.ws_col;
}

void unixBeep(DESC* fd) {
        char seq[64];
        snprintf(seq,64,"\x7");
        fd->write(fd,seq,strlen(seq));
}

int unixRead(DESC* fd, char* buff, size_t size)
{
        return read(fd->inFile,buff,size);
}

int unixWrite(DESC* fd,const char* buff, size_t size)
{
        return write(fd->outFile,buff,size);
}

#else
int winConinit(DESC* fd)
{
        fd->inFile  = GetStdHandle(STD_INPUT_HANDLE);
        fd->outFile = GetStdHandle(STD_OUTPUT_HANDLE);
        
        /* Change Code Page to OEM USA */
        SetConsoleCP(437);

        return 1;
}
int winConEnableRawMode(DESC* fd)
{
        UNUSED(fd);
        // Do nothing
        rawmode = 1;
        return 0;
}

int winConDisableRawMode(DESC* fd)
{
        UNUSED(fd);
        // Do nothing
        return 0;
}
int winConGetcolums(DESC* fd)
{
        CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
        GetConsoleScreenBufferInfo(fd->outFile, &bufferinfo);
        return bufferinfo.dwSize.X;
}

int winConMovecursor(DESC* fd, CursorMove move, int x)
{
        CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
        DWORD written;
        DWORD length;
        GetConsoleScreenBufferInfo(fd->outFile , &bufferinfo);
        switch (move) {
        case CURSOR_MOVE_TO:
                bufferinfo.dwCursorPosition.X=x;
                SetConsoleCursorPosition(fd->outFile, bufferinfo.dwCursorPosition);
                break;
        case CURSOR_MOVE_LEFT:
                bufferinfo.dwCursorPosition.X=0;
                SetConsoleCursorPosition(fd->outFile, bufferinfo.dwCursorPosition);
                break;
        case CURSOR_ERASE_FROM:
                length = bufferinfo.dwSize.X - bufferinfo.dwCursorPosition.X;
                FillConsoleOutputCharacterA(fd->outFile, ' ',length,bufferinfo.dwCursorPosition,&written );
                break;
        default:
                break;
        }

        return 1;

}

void winConBeep(DESC* fd)
{
        UNUSED(fd);
        MessageBeep(0xFFFFFFFF);
}

int winConClearScreen(DESC* fd)
{
        CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
        COORD orig;
        DWORD written;
        orig.X = 0;
        orig.Y = 0;
        GetConsoleScreenBufferInfo(fd->outFile , &bufferinfo);
        FillConsoleOutputCharacterA(fd->outFile, ' ',
                bufferinfo.dwSize.X * bufferinfo.dwSize.Y,
                orig,&written);
        SetConsoleCursorPosition(fd->outFile, orig);
    return 1;
}

int winConRead(DESC* fd, char* buff, size_t size)
{
        size_t nbInput=0;
        BOOL success = TRUE;
        while ( success && nbInput < size ) {
                INPUT_RECORD input;
                DWORD numberOfEvents;
                success = ReadConsoleInput(fd->inFile, &input, 1,&numberOfEvents);
                // We catch only key event, and just when the key is pressed down (release is discarded)
                if ( success && input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown==TRUE) {
                        // FIXME key repeat
                        switch ( input.Event.KeyEvent.wVirtualKeyCode ) {
                        case VK_LEFT:
                                buff[nbInput] = 2; // map to ctrl-b;
                                break;

                        case VK_UP:
                                buff[nbInput] = 16; // map to ctrl-p;
                                break;

                        case VK_RIGHT:
                                buff[nbInput] = 6; // map to ctrl-b;
                                break;

                        case VK_DOWN:
                                buff[nbInput] = 14; // map to ctrl-n;
                                break;

                        default:
                                buff[nbInput]=input.Event.KeyEvent.uChar.AsciiChar;
                        }
                        nbInput++;
                }
        }

        return nbInput;
}

int winConWrite(DESC* fd, const char* buff, size_t size)
{
        DWORD written;
        return WriteConsoleA(fd->outFile,buff,size,&written,NULL);
}

#endif

static int doNothing(DESC* fd)
{
    UNUSED(fd);
    return 1;
}

static int userWrite(DESC* fd, const char* buff, size_t size)
{
    return fd->config.lineeditWrite(fd->config.userContext, fd, buff, size);
}

static int userRead(DESC* fd, char* buff, size_t size)
{
    return fd->config.lineeditRead(fd->config.userContext, fd, buff, size);
}

static int userGetColumns(DESC* fd)
{
    return fd->columns;
}

static int isUnsupportedTerm(DESC* fd) {

    char *term = getenv("TERM");
    int j;

        fd->init(fd);

    if (term == NULL) return 0;
    for (j = 0; unsupported_term[j]; j++)
        if (!strcasecmp(term,unsupported_term[j])) return 1;
    return 0;
}

static void freeHistory(DESC* fd) {
    if (fd->history) {
        int j;

        for (j = 0; j < fd->history_len; j++)
            free(fd->history[j]);
        free(fd->history);
    }
}

static int enableRawMode(DESC* fd) {
        return fd->enableRawMode(fd);
}

static int disableRawMode(DESC* fd) {

        return fd->disableRawMode(fd);

}

static void lineeditCleanup( DESC* fd)
{
    disableRawMode(fd);
    freeHistory(fd);
}

/* At exit we'll try to fix the terminal to the initial conditions. */
#ifndef _WIN32
static void lineeditAtExit(void) {
    if ( globalFd != NULL ) {
        lineeditCleanup(globalFd);
    }
}
#endif



static int getColumns(DESC* fd) {
        return fd->getcolumns(fd);
}

static void refreshLine(DESC* fd, const char *prompt, char *buf, size_t len, size_t pos, size_t cols) 
{
    size_t plen = strlen(prompt);
    
    while((plen+pos) >= cols) {
        buf++;
        len--;
        pos--;
    }
    while (plen+len > cols) {
        len--;
    }

        /* Cursor to left edge */
        if ( fd->movecursor(fd, CURSOR_MOVE_LEFT, 0)  == 0) return;
        /* Write the prompt and the current buffer content */
        if ( fd->write(fd, prompt, strlen(prompt))    ==0 ) return;
        if ( fd->write(fd, buf, len)                  ==0 ) return;
        /* Erase to right */
        if ( fd->movecursor(fd, CURSOR_ERASE_FROM, 0) == 0) return;
        /* Move cursor to original position. */;
        if ( fd->movecursor(fd, CURSOR_MOVE_TO, (pos+plen)) == 0) return;

}

static void beep(DESC* fd) {
        fd->beep(fd);
}

static void freeCompletions(lineeditCompletions *lc) {
    size_t i;
    for (i = 0; i < lc->len; i++)
        free(lc->cvec[i]);
    if (lc->cvec != NULL)
        free(lc->cvec);
}
static int lineeditRead(DESC* fd, char* buff, size_t size)
{
    return fd->read(fd,buff, size);

}
static int completeLine(DESC* fd, const char *prompt, char *buf, size_t buflen, size_t *len, size_t *pos, size_t cols) {
    lineeditCompletions lc = { 0, NULL };
    int nread, nwritten;
    char c = 0;

    fd->completionCallback(fd, buf,&lc);
    if (lc.len == 0) {
        beep(fd);
    } else {
        size_t stop = 0, i = 0;
        size_t clen;

        while(!stop) {
            /* Show completion or original buffer */
            if (i < lc.len) {
                clen = strlen(lc.cvec[i]);
                refreshLine(fd,prompt,lc.cvec[i],clen,clen,cols);
            } else {
                refreshLine(fd,prompt,buf,*len,*pos,cols);
            }

            nread = lineeditRead(fd,&c,1);
            if (nread <= 0) {
                freeCompletions(&lc);
                return -1;
            }

            switch(c) {
                case 9: /* tab */
                    i = (i+1) % (lc.len+1);
                    if (i == lc.len) beep(fd);
                    break;
                case 27: /* escape */
                    /* Re-show original buffer */
                    if (i < lc.len) {
                        refreshLine(fd,prompt,buf,*len,*pos,cols);
                    }
                    stop = 1;
                    break;
                default:
                    /* Update buffer and return */
                    if (i < lc.len) {
                        nwritten = snprintf(buf,buflen,"%s",lc.cvec[i]);
                        *len = *pos = nwritten;
                    }
                    stop = 1;
                    break;
            }
        }
    }

    freeCompletions(&lc);
    return c; /* Return last read character */
}

void lineeditClearScreen(DESC* fd) {
        fd->clearScreen(fd);
}

#define IAC 255      // Interpret as command
#define SE      240          // End of subnegotiation parameters.
#define NOP     241          // No operation
#define DM      242          // Data mark. Indicates the position of a Synch event within the data stream. This should always be accompanied by a TCP urgent notification.
#define BRK     243          // Break. Indicates that the "break" or "attention" key was hit.
#define IP      244          // Suspend, interrupt or abort the process to which the NVT is connected.
#define AO      245          // Abort output. Allows the current process to run to completion but do not send its output to the user.
#define AYT     246          // Are you there. Send back to the NVT some visible evidence that the AYT was received.
#define EC      247          // Erase character. The receiver should delete the last preceding undeleted character from the data stream.
#define EL      248          // Erase line. Delete characters from the data stream back to but not including the previous CRLF.
#define GA      249          // Go ahead. Used, under certain circumstances, to tell the other end that it can transmit.
#define SB       250     // Subnegotiation of the indicated option follows.
#define WILL 251         // Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
#define WONT 252         // Indicates the refusal to perform, or continue performing, the indicated option.
#define DO       253     // Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
#define DONT 254         // Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.

#define OPT_ECHO            1   // echo 857
#define OPT_GO_AHEAD        3   // suppress go ahead    858
#define OPT_STATUS          5   // status       859
#define OPT_TIMING_         6   // timing mark  860
#define OPT_TTY             24  // terminal type        1091
#define OPT_NAWS            31  // window size  1073
#define OPT_TS              32  // terminal speed       1079
#define OPT_FLOW_CONTROL    33  // remote flow control  1372
#define OPT_LINEMODE        34  // linemode     1184
#define OPT_ENVVAR          36  // environment variables        1408

#define BYTE_IAC        0
#define BYTE_OPERATION  1
#define BYTE_OPTION     2

int processTelnetCommand(DESC* fd )
{
    unsigned char answer[3];
    int no_answer = 0; /* By default there is always an answer */
    answer[BYTE_IAC] =  IAC;


    if ( fd->telnetCmd[BYTE_OPERATION] == WILL ) {
        switch ( fd->telnetCmd[BYTE_OPTION]) {

            /* Manage echo command */
            case OPT_ECHO:
                answer[BYTE_OPERATION]  = DO;
                answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];
                fd->telnetEchoRequested = 1;
                break;
            /* To get the size of the windows */
            case OPT_NAWS:
                answer[BYTE_OPERATION]  = DO;
                answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];
                fd->telnetWSRequested = 1;
                break;

            default:
                answer[BYTE_OPERATION]  = DONT;
                answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];

        }

    } else if ( fd->telnetCmd[BYTE_OPERATION] == WONT  ) {

        answer[BYTE_OPERATION]  = DONT;
        answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];

    } else if ( fd->telnetCmd[BYTE_OPERATION] == DO  ) {

        switch ( fd->telnetCmd[BYTE_OPTION]) {

            case OPT_ECHO:
                /* If we have not asked for doing the echo, this is a request from
                 * the other end, but we want to do it by ourself */
                if ( ! fd->telnetEchoRequested ) {
                    /* Fall through to the default : */
                } else {
                    no_answer = 1;
                    break;
                }
                
            default:
                answer[BYTE_OPERATION]  = WONT;
                answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];

        }


    } else if ( fd->telnetCmd[BYTE_OPERATION] == DONT  ) {
        answer[BYTE_OPERATION]  = WONT;
        answer[BYTE_OPTION]     = fd->telnetCmd[BYTE_OPTION];
    
    /* Subnegotiation */

    } else if ( fd->telnetCmd[BYTE_OPERATION] == SB  ) {
        switch ( fd->telnetCmd[BYTE_OPTION]) {
        
            case OPT_NAWS: {
                /* We need to get 6 more characters we have already IAC SB NAWS from:
                 *  IAC SB NAWS <16-bit value> <16-bit value> IAC SE
                 */
                unsigned char buff[6];
                if ( fd->read(fd,buff, sizeof(buff)) != 0 ) {
                    if ( buff[4] == IAC && buff[5] == SE ) {
                        fd->columns = (buff[0] << 8 | buff[1]);
                        fd->rows    = (buff[2] << 8 | buff[3]);
                    }
                };
                                no_answer = 1;
                }
                break;

        }
    }

    if ( ! no_answer ) fd->write(fd,(const char*)answer, sizeof(answer));

    return 1;
}

int telnetDoEcho( DESC* fd) {

    /* If not already negotiated, we ask to do the echo by ourself */
    if ( ! fd->telnetEchoRequested ) {
        unsigned char query[3];
        query[BYTE_IAC]        =  IAC;
        query[BYTE_OPERATION]  =  WILL;
        query[BYTE_OPTION]     =  OPT_ECHO;
        fd->write(fd,(const char*)query, sizeof(query));
        fd->telnetEchoRequested = 1;
    }

    return 1;
 
}

int telnetDoNAWS( DESC* fd) {

    /* If not already negotiated, we ask to do the echo by ourself */
    if ( ! fd->telnetWSRequested ) {
        unsigned char query[3];
        query[BYTE_IAC]        =  IAC;
        query[BYTE_OPERATION]  =  DO;
        query[BYTE_OPTION]     =  OPT_NAWS;
        fd->write(fd,(const char*)query, sizeof(query));
        fd->telnetWSRequested = 1;
    }

    return 1;

}
int processTelnet( DESC* fd, const char c_in) {
    unsigned char c = (unsigned char)c_in;
    
    /* If not done do echo negotiation */
    telnetDoEcho(fd);
    /* Ask for the terminal size */
    telnetDoNAWS(fd);

    /* Fixme discard NUL character */
    if ( c_in == 0 ) {
        return 1;
    }
    
    /* Do state machine transition */
    switch (c) {

        /* Starting a telnet sequence */
        case IAC:
            fd->telnetCmdByte = 0;
            fd->telnetState = TELNET_IAC_STARTED;
            break;

        default:
            break;
    }

    if ( fd->telnetState == TELNET_IAC_STARTED ) {
        fd->telnetCmd[fd->telnetCmdByte] = c;
        fd->telnetCmdByte += 1;
    }

    /* FIXME manage the case, where the correct sequence is not done */

    /* Do we have all the 3 bytes making a command */
    if ( fd->telnetCmdByte == 3 ) {
        fd->telnetState = TELNET_IAC_COMPLETE;
        processTelnetCommand(fd);
        fd->telnetState = TELNET_IDLE;
        fd->telnetCmdByte = 0;
    }

    /* If no telnet command are in processing, we let the call fall through the usual processing */
    if ( fd->telnetState == TELNET_IDLE) {
        return 0;
    } else {
        return 1;
    }
}

static int lineeditWrite(DESC* fd, const char* buff, size_t size)
{
        return fd->write(fd,buff,size);
}

static int lineeditPrompt(DESC* fd, char *buf, size_t buflen, const char *prompt) {
    size_t plen = strlen(prompt);
    size_t pos = 0;
    size_t len = 0;
    size_t cols = getColumns(fd);
    int history_index = 0;

    buf[0] = '\0';
    buflen--; /* Make sure there is always space for the nulterm */

    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    lineeditHistoryAdd(fd, "");
    
        /* Move to the beginning of the line */
         //fd->movecursor(fd, CURSOR_MOVE_LEFT, 0);
    //if (lineeditWrite(fd,prompt,plen) == -1) return -1;
    while(1) {
        char c;
        int nread;
        char seq[2], seq2[2];
        nread = lineeditRead(fd,&c,1);

        /* If we have detected and processed a telnet sequence, the character
         * has already been managed, so we move directly to the next one
         */
        if ( (nread > 0) && processTelnet(fd, c) ) continue;

        /* No character receive, we probably need to reset the connection */
        if (nread == 0) return -2;

        if (nread <= 0) return len;

        /* Only autocomplete when the callback is set. It returns < 0 when
         * there was an error reading from fd. Otherwise it will return the
         * character that should be handled next. */
        if (c == 9 && fd->completionCallback != NULL) {
            c = completeLine(fd,prompt,buf,buflen,&len,&pos,cols);
            /* Return on errors */
            if (c < 0) return len;
            /* Read next character when 0 */
            if (c == 0) continue;
        }

        switch(c) {
        case 13:    /* enter */
            fd->history_len--;
            free(fd->history[fd->history_len]);
            fd->write(fd,"\n\r",2);
            return (int)len;
        case 3:     /* ctrl-c */
            errno = EAGAIN;
            return -1;
        case 127:   /* backspace */
        case 8:     /* ctrl-h */
            if (pos > 0 && len > 0) {
                memmove(buf+pos-1,buf+pos,len-pos);
                pos--;
                len--;
                buf[len] = '\0';
                refreshLine(fd,prompt,buf,len,pos,cols);
            }
            break;
        case 4:     /* ctrl-d, remove char at right of cursor */
            if (len > 1 && pos < (len-1)) {
                memmove(buf+pos,buf+pos+1,len-pos);
                len--;
                buf[len] = '\0';
                refreshLine(fd,prompt,buf,len,pos,cols);
            } else if (len == 0) {
                fd->history_len--;
                free(fd->history[fd->history_len]);
                return -1;
            }
            break;
        case 20:    /* ctrl-t */
            if (pos > 0 && pos < len) {
                int aux = buf[pos-1];
                buf[pos-1] = buf[pos];
                buf[pos] = aux;
                if (pos != len-1) pos++;
                refreshLine(fd,prompt,buf,len,pos,cols);
            }
            break;
        case 2:     /* ctrl-b */
            goto left_arrow;
        case 6:     /* ctrl-f */
            goto right_arrow;
        case 16:    /* ctrl-p */
            seq[1] = 65;
            goto up_down_arrow;
        case 14:    /* ctrl-n */
            seq[1] = 66;
            goto up_down_arrow;
            break;
        case 27:    /* escape sequence */
            if (lineeditRead(fd,seq,2) == -1) break;
            if (seq[0] == 91 && seq[1] == 68) {
left_arrow:
                /* left arrow */
                if (pos > 0) {
                    pos--;
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && seq[1] == 67) {
right_arrow:
                /* right arrow */
                if (pos != len) {
                    pos++;
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && (seq[1] == 65 || seq[1] == 66)) {
up_down_arrow:
                /* up and down arrow: history */
                if (fd->history_len > 1) {
                    /* Update the current history entry before to
                     * overwrite it with tne next one. */
                    free(fd->history[fd->history_len-1-history_index]);
                    fd->history[fd->history_len-1-history_index] = strdup(buf);
                    /* Show the new entry */
                    history_index += (seq[1] == 65) ? 1 : -1;
                    if (history_index < 0) {
                        history_index = 0;
                        break;
                    } else if (history_index >= fd->history_len) {
                        history_index = fd->history_len-1;
                        break;
                    }
                    strncpy(buf,fd->history[fd->history_len-1-history_index],buflen);
                    buf[buflen] = '\0';
                    len = pos = strlen(buf);
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && seq[1] > 48 && seq[1] < 55) {
                /* extended escape */
                if (lineeditRead(fd,seq2,2) == -1) break;
                if (seq[1] == 51 && seq2[0] == 126) {
                    /* delete */
                    if (len > 0 && pos < len) {
                        memmove(buf+pos,buf+pos+1,len-pos-1);
                        len--;
                        buf[len] = '\0';
                        refreshLine(fd,prompt,buf,len,pos,cols);
                    }
                }
            }
            break;
        default:
            if (len < buflen) {
                if (len == pos) {
                    buf[pos] = c;
                    pos++;
                    len++;
                    buf[len] = '\0';
                    if (plen+len < cols) {
                        /* Avoid a full update of the line in the
                         * trivial case. */
                        if (lineeditWrite(fd,&c,1) == -1) return -1;
                    } else {
                        refreshLine(fd,prompt,buf,len,pos,cols);
                    }
                } else {
                    memmove(buf+pos+1,buf+pos,len-pos);
                    buf[pos] = c;
                    len++;
                    pos++;
                    buf[len] = '\0';
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            }
            break;
        case 21: /* Ctrl+u, delete the whole line. */
            buf[0] = '\0';
            pos = len = 0;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 11: /* Ctrl+k, delete from current to end of line. */
            buf[pos] = '\0';
            len = pos;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 1: /* Ctrl+a, go to the start of the line */
            pos = 0;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 5: /* ctrl+e, go to the end of the line */
            pos = len;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 12: /* ctrl+l, clear screen */
            lineeditClearScreen(fd);
            refreshLine(fd,prompt,buf,len,pos,cols);
        }
    }
    return len;
}

static int lineeditRaw(DESC* fd, char *buf, size_t buflen, const char *prompt) {
   
    int count;

    if (buflen == 0) {
        errno = EINVAL;
        return -1;
    }
#ifndef _WIN32
    if (!isatty(fd->inFile)) {
        fd->inFile_fd = fdopen(fd->inFile, "a");
        if (fgets(buf, buflen, fd->inFile_fd) == NULL) return -1;
        count = strlen(buf);
        if (count && buf[count-1] == '\n') {
            count--;
            buf[count] = '\0';
        }
    } 
        else 
#endif
    {
        if (enableRawMode(fd) == -1) return -1;
        count = lineeditPrompt(fd, buf, buflen, prompt);
        disableRawMode(fd);
        //fd->write(fd,"\n\r",2);
    }
    return count;
}

char *lineedit(DESC* fd, const char *prompt) {
    char buf[lineedit_MAX_LINE];
    int count;

    if (isUnsupportedTerm(fd)) {
        size_t len;

        //printf("%s",prompt);
        //fflush(stdout);
        if (fgets(buf,lineedit_MAX_LINE,stdin) == NULL) return NULL;
        len = strlen(buf);
        while(len && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
            len--;
            buf[len] = '\0';
        }
        return strdup(buf);
    } else {
        count = lineeditRaw(fd, buf,lineedit_MAX_LINE,prompt);
        if (count == -2) {
            fd->reset(fd);
            return NULL;
        }
        if (count == -1) return NULL;
        return strdup(buf);
    }
}

/* Register a callback function to be called for tab-completion. */
void lineeditSetCompletionCallback(DESC* fd, lineeditCompletionCallback *fn) {
    fd->completionCallback = fn;
}

void lineeditAddCompletion(DESC* fd, lineeditCompletions *lc, char *str) {
    UNUSED(fd);
    size_t len = strlen(str);
    char *copy = malloc(len+1);
    memcpy(copy,str,len+1);
    lc->cvec = realloc(lc->cvec,sizeof(char*)*(lc->len+1));
    lc->cvec[lc->len++] = copy;
}

/* Using a circular buffer is smarter, but a bit more complex to handle. */
int lineeditHistoryAdd(DESC* fd, const char *line) {
    char *linecopy;

    if (fd->history_max_len == 0) return 0;
    if (fd->history == NULL) {
        fd->history = malloc(sizeof(char*)*fd->history_max_len);
        if (fd->history == NULL) return 0;
        memset(fd->history,0,(sizeof(char*)*fd->history_max_len));
    }
    linecopy = strdup(line);
    if (!linecopy) return 0;
    if (fd->history_len == fd->history_max_len) {
        free(fd->history[0]);
        memmove(fd->history,fd->history+1,sizeof(char*)*(fd->history_max_len-1));
        fd->history_len--;
    }
    fd->history[fd->history_len] = linecopy;
    fd->history_len++;
    return 1;
}

int lineeditHistorySetMaxLen(DESC* fd, int len) {
    char **new;

    if (len < 1) return 0;
    if (fd->history) {
        int tocopy = fd->history_len;

        new = malloc(sizeof(char*)*len);
        if (new == NULL) return 0;
        if (len < tocopy) tocopy = len;
        memcpy(new,fd->history+(fd->history_max_len-tocopy), sizeof(char*)*tocopy);
        free(fd->history);
        fd->history = new;
    }
    fd->history_max_len = len;
    if (fd->history_len > fd->history_max_len)
        fd->history_len = fd->history_max_len;
    return 1;
}

/* Save the history in the specified file. On success 0 is returned
 * otherwise -1 is returned. */
int lineeditHistorySave(DESC* fd, const char *filename) {
    FILE *fp = fopen(filename,"w");
    int j;
    
    if (fp == NULL) return -1;
    for (j = 0; j < fd->history_len; j++)
        fprintf(fp,"%s\n",fd->history[j]);
    fclose(fp);
    return 0;
}

/* Load the history from the specified file. If the file does not exist
 * zero is returned and no operation is performed.
 *
 * If the file exists and the operation succeeded 0 is returned, otherwise
 * on error -1 is returned. */
int lineeditHistoryLoad(DESC* fd, const char *filename) {
    FILE *fp = fopen(filename,"r");
    char buf[lineedit_MAX_LINE];
    
    if (fp == NULL) return -1;

    while (fgets(buf,lineedit_MAX_LINE,fp) != NULL) {
        char *p;
        
        p = strchr(buf,'\r');
        if (!p) p = strchr(buf,'\n');
        if (p) *p = '\0';
        lineeditHistoryAdd(fd, buf);
    }
    fclose(fp);
    return 0;
}

DESC* lineeditInit( const lineeditConfig* config )
{
        DESC* fd = malloc(sizeof(DESC));
    if (fd == NULL ) {
        return NULL;
    }
        memset(fd, 0, sizeof(DESC) );

    /* Default configuration */
    fd->config.lineeditRead  = NULL;
    fd->config.lineeditWrite = NULL;
    fd->config.termType     = lineedit_CONSOLE;
    fd->config.userContext  = NULL;
    fd->telnetCmdByte       = 0;
    fd->telnetState         = TELNET_IDLE;
    fd->telnetEchoRequested = 0;
    /* Until we have more information we start with 80 columns */
    fd->columns             = 80; 

    fd->history_max_len = lineedit_DEFAULT_HISTORY_MAX_LEN;

    /* Override default configuration according to the user */
    if ( config != NULL ) {
        fd->config = *config;

        /* When in lineedit_USER mode, the user must specify I/O functions */
        if (fd->config.termType == lineedit_USER && 
            (fd->config.lineeditRead == NULL ||  fd->config.lineeditWrite == NULL ) ) {
                return NULL;
        }
    } 
 

#ifndef _WIN32
        switch(fd->config.termType) {
        case lineedit_CONSOLE:
                fd->init           = unixConinit;
                fd->beep           = vtBeep;
                fd->clearScreen    = vtClearScreen;
                fd->disableRawMode = unixConDisableRawMode;
                fd->enableRawMode  = unixConEnableRawMode;
                fd->getcolumns     = unixConGetColumns;
                fd->movecursor     = vtMoveCursor;
                fd->read           = unixRead;
                fd->write          = unixWrite;
                fd->reset          = telnetReset;
                break;

        case lineedit_USER:
                fd->init           = doNothing;
                fd->beep           = vtBeep;
                fd->clearScreen    = vtClearScreen;
                fd->disableRawMode = doNothing;
                fd->enableRawMode  = doNothing;
                fd->getcolumns     = userGetColumns;
                fd->movecursor     = vtMoveCursor;
                fd->read           = userRead;
                fd->write          = userWrite;
                fd->reset          = telnetReset;
                break;
        }
#else
        
        switch(fd->config.termType) {
                case lineedit_CONSOLE:
                        fd->init           = winConinit;
                        fd->beep           = winConBeep;
                        fd->clearScreen    = winConClearScreen;
                        fd->disableRawMode = winConDisableRawMode;
                        fd->enableRawMode  = winConEnableRawMode;
                        fd->getcolumns     = winConGetcolums;
                        fd->movecursor     = winConMovecursor;
                        fd->read           = winConRead;
                        fd->write          = winConWrite;
            fd->reset          = telnetReset;
                        break;

                case lineedit_USER:
                        fd->init           = doNothing;
                        fd->beep           = vtBeep;
                        fd->clearScreen    = vtClearScreen;
                        fd->disableRawMode = doNothing;
                        fd->enableRawMode  = doNothing;
                        fd->getcolumns     = userGetColumns;
                        fd->movecursor     = vtMoveCursor;
                        fd->read           = userRead;
                        fd->write          = userWrite;
            fd->reset          = telnetReset;
                        break;
                default:
                        break;

        }
        return fd;
#endif
}
