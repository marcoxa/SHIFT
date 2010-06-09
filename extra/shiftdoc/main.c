/* -*- Mode: C -*- */

/* main.c -- */

/*
 * Copyright (c) 1996, 1997 Regents of the University of California.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and the California PATH Program.
 * 4. Neither the name of the University nor of the California PATH
 *    Program may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * This file is part of the SHIFT compiler.
 *
 * Copyright (c) 1995-96 Luigi Semenzato, Berkeley PATH
 * luigi@fiat.its.berkeley.edu
 * January 1996
 */


#ifndef COMPILER_MAIN_I
#define COMPILER_MAIN_I


static char* version = "0.9";
static char* date = "Wed Sep 24 13:18:56 PDT 1997";


/* Do not discard, PLEASE!!!! 'prcs' still has a bug that - in some
 * cases - causes the obliteration the 'Format' lines.
 * The manifestation of the bug is an 'undefined variable' at compile
 * time.  We leave the two declarations below for ready correction of
 * this bug.

 static char* version = "2.33";
 static char* date = "Fri, 24 Oct 1996 15:23:04 -0700";

 * Marco Antoniotti 19970225
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "shic.h"
#include "dcScanaux.h"


/* YACC variables */

extern int yydebug;
extern FILE *yyin;

/* Size of the input file, rouded to the upper 512 bites. This is
 * needed by the parser.
 *
 * Marco Zandonadi 19970918
 */
long yyinSize;

char *docBuffer[NUM_PARSE_BUFFERS];
long yyinSize;

#define CPP "/lib/cpp -B"


/* get_path -- get the path (including final '/', if any) of the
 * filename passed.
 *
 * Used below to help locate "shiftprecpp" when a non-
 * ordinary path is used, on the assumption that if the
 * compiler is being dragged from an explicitly-named directory,
 * the appropriate version of shiftprecpp will be in that same
 * directory.
 *
 * CAVEAT: has its own buffer space, so don't count on results
 *         staying around long;
 */
static char *
get_path(char *filename)
{
  int i;
  static char buf[256];

  assert(filename != 0);
  assert(filename[0] != 0);

  i = strlen(filename);
  strcpy (buf, filename);

  while (--i >= 0)
    if (buf[i] == '/')
      {
	buf[i+1] = '\0';
	return buf;
      }

  return "";
}


lv *Program;
FILE *cfile, *hfile;
char* base_file_name;
char* cfilename;
char* hfilename;
char* precpp_file_name;
char* cpreproc_file_name;
int base_file_name_len;


void
main(int argc, char **argv)
{

  char cmd_buffer[1024];	/* Big enough for lengthy options. */
  char *cpp;
  int yyin_close_status;
  int keep_intermediate_files_p = 0; /* Hardwired for the time being */

  int i;

  /* In this structure we keep some informations on the input file
   * Marco Zandonadi 19970918
   */
  struct stat yyinInfo;

  yydebug = 0;	/* For a good time, change this to 1. */

  if (argc == 2) 
    {
      char *suffix, *slash, *path;
      path = get_path (argv[0]);
      if (! strcmp(argv[1], "-v") || ! strcmp(argv[1], "--version"))
	{
	  printf("shiftdoc version %s of %s UTC\n", version, date);
	  exit(0);
	}

      filename = argv[1];

      base_file_name = strdup(filename);

      /* Find the last period following the last slash. */
      suffix = rindex(base_file_name, '.');
      slash = rindex(base_file_name, '/');
      if (slash > suffix) suffix = 0;
      if (! suffix || strcmp(".hs", suffix))
	{
	  fprintf(stderr, "shiftdoc: input file does not have an '.hs'");
	  fprintf(stderr, " or '.hsp' suffix\n");
	  exit(1);
	}

      /* Effectively truncate the filename. */
      suffix[0] = '\0';

      /* Allocate all the other filenames */
      base_file_name_len = strlen(base_file_name);

      cfilename = (char*) malloc(sizeof(char) * base_file_name_len + 3);
      hfilename = (char*) malloc(sizeof(char) * base_file_name_len + 3);
      precpp_file_name = (char*) malloc(sizeof(char) * base_file_name_len + 4);
      cpreproc_file_name = (char*) malloc(sizeof(char) * base_file_name_len + 4);
      if (cfilename == 0
	  || hfilename == 0
	  || precpp_file_name == 0
	  || cpreproc_file_name == 0)
	{
	  fprintf(stderr, "shiftdoc: cannot allocate memory.\n");
	  exit(1);
	}

      strcpy(cfilename, base_file_name);
      strcat(cfilename, ".c");

      strcpy(hfilename, base_file_name);
      strcat(hfilename, ".h");

      strcpy(precpp_file_name, base_file_name);
      strcat(precpp_file_name, ".1p");

      strcpy(cpreproc_file_name, base_file_name);
      strcat(cpreproc_file_name, ".2p");

      cpp = getenv("CPP");
      if (! cpp) cpp = CPP;

      /* Call the SHIFT pre CPP processor. */

      sprintf(cmd_buffer, "%s%s %s > %s",
	      path,
	      "shiftprecpp",
	      filename,
	      precpp_file_name);
      system(cmd_buffer);
      
      /* We call the CPP with the -C option because we want to
       * preserve the comments. 
       *
       * Marco Zandonadi 19970911
       */

      sprintf(cmd_buffer, "%s -C %s > %s",
            cpp, precpp_file_name, cpreproc_file_name);
      system(cmd_buffer);

      yyin = fopen(cpreproc_file_name, "r");

      if (yyin == NULL)
	{
	  fprintf(stderr, "shiftdoc: ");
	  perror(filename);
	  exit(1);
	}
    }
  else
    {
      fprintf(stderr, "usage: shiftdoc [file] or shiftdoc -v\n");
      exit(1);
    }

  /* Front end: parser and type checker. */
  initialize_check();

  fstat(fileno(yyin), &yyinInfo);
  yyinSize = yyinInfo.st_size;

  for (i = 0; i < NUM_PARSE_BUFFERS; i++) 
    {
      /* To be absolutely safe we double the size of the file and add
       * a constant. 19970924 Marco Zandonadi */
      docBuffer[i] = (char*) malloc(sizeof(char) * (yyinSize*2+5000));
      if (docBuffer[i] == 0) 
	{
	  fprintf(stderr, "shiftdoc: cannot allocate memory.\n");
	  exit(1);
	}		
    }

  yyparse();
  /* yyin_close_status = pclose(yyin); */
  fclose(yyin);


  for (i = 0; i < NUM_PARSE_BUFFERS; i++)
    free(docBuffer[i]);

  if (n_errors)
    {
      /* printf("shiftdoc: error: C preprocessor pipe status %d\n",
	 yyin_close_status);
	 */
      exit(1);
    }

  /* Remove intermediate files */

  if (! keep_intermediate_files_p)
    {
      /* We should check for the completion of these commands */
      unlink(precpp_file_name);
      unlink(cpreproc_file_name);
    }

  collect_typedef_sections();
  patch_sync();
  insert_dummy_state();
  check();
  if (n_errors) exit(1);

  /* Back end: transformations and code generation. */

  /* The Back End is not needed in shiftdoc so we comment it.
   *
   * Marco Zandonadi 19970916
   *
   * cfile = fopen(cfilename, "w");
   * hfile = fopen(hfilename, "w");
   * if (cfile == NULL)
   *   {
   *     fprintf(stderr, "shiftdoc: ");
   *     perror(cfilename);
   *      exit(1);
   *   }
   * if (hfile == NULL)
   *   {
   *     fprintf(stderr, "shiftdoc: ");
   *     perror(hfilename);
   *     exit(1);
   *   }
   * initialize_transform();
   * transform(Program);
   * initialize_generate();
   * generate(Program);
   * fclose(cfile);
   * fclose(hfile);
   */
  exit(0);
}

#endif /* COMPILER_MAIN_I */

/* end of file -- main.c -- */
