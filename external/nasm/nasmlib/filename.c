/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 1996-2016 The NASM Authors - All Rights Reserved
 *   See the file AUTHORS included with the NASM distribution for
 *   the specific copyright holders.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following
 *   conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *     
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----------------------------------------------------------------------- */

/*
 * nasmlib.c	library routines for the Netwide Assembler
 */

#include "compiler.h"
#include "nasmlib.h"
#include "error.h"

void standard_extension(char *inname, char *outname, char *extension)
{
    char *p, *q;

    if (*outname)               /* file name already exists, */
        return;                 /* so do nothing */
    q = inname;
    p = outname;
    while (*q)
        *p++ = *q++;            /* copy, and find end of string */
    *p = '\0';                  /* terminate it */
    while (p > outname && *--p != '.') ;        /* find final period (or whatever) */
    if (*p != '.')
        while (*p)
            p++;                /* go back to end if none found */
    if (!strcmp(p, extension)) {        /* is the extension already there? */
        if (*extension)
            nasm_error(ERR_WARNING | ERR_NOFILE,
		       "file name already ends in `%s': "
		       "output will be in `nasm.out'", extension);
        else
            nasm_error(ERR_WARNING | ERR_NOFILE,
		       "file name already has no extension: "
		       "output will be in `nasm.out'");
        strcpy(outname, "nasm.out");
    } else
        strcpy(p, extension);
}
