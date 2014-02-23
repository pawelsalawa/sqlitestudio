/* based on linenoise.h -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * See lineedit.c for more information.
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
 */

#ifndef __lineedit_H
#define __lineedit_H
#ifdef __cplusplus
extern "C" {
#endif


typedef struct lineeditCompletions {
  size_t len;
  char **cvec;
} lineeditCompletions;

typedef enum  {
        lineedit_INVALID,
        lineedit_CONSOLE,
        lineedit_USER
} lineeditTermType;


/* Opaque structure returned to the user containing the lineedit context */
struct DESC_;
typedef struct DESC_ DESC;

/* User handler to be defined to use user provided read/write functions */
typedef int (*lineeditWriteFnPtr)(void* userContext, DESC* fd, const char* buff, size_t size);
typedef int (*lineeditReadFnPtr)(void* userContext, DESC* fd, char* buff, size_t size);

/* Used to config lineedit */
typedef struct lineeditConfig {
    lineeditTermType termType;     /* Specify the type of terminal used, either standard console, or user defined */
    lineeditWriteFnPtr lineeditWrite; /* Set to different of NULL when lineedit must use it to write data */
    lineeditReadFnPtr  lineeditRead;  /* Set to different of NULL when lineedit must use it to read data */
    void* userContext;              /* Context pass back to the user, when using non-NUL read/write function */
} lineeditConfig;

DESC* lineeditInit(const lineeditConfig* config);

typedef void(lineeditCompletionCallback)(DESC* fd,const char *, lineeditCompletions *);
void lineeditSetCompletionCallback(DESC* fd, lineeditCompletionCallback *fn);
void lineeditAddCompletion(DESC* fd, lineeditCompletions *, char *);

char *lineedit( DESC* fd, const char *prompt);
int lineeditHistoryAdd(DESC* fd, const char *line);
int lineeditHistorySetMaxLen(DESC* fd, int len);
int lineeditHistorySave(DESC* fd, const char *filename);
int lineeditHistoryLoad(DESC* fd, const char *filename);
void lineeditClearScreen(DESC* fd);


#ifdef __cplusplus
}
#endif

#endif /* __lineedit_H */
