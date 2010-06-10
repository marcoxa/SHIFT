/* -*- Mode: C -*- */

/* main.c -- */

/*
 * Copyright (c)1996, 1997, 1998 The Regents of the University of
 * California (Regents). All Rights Reserved. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research, and not-for-profit purposes,
 * without fee and without a signed licensing agreement, is hereby
 * granted, provided that the above copyright notice, this paragraph and
 * the following two paragraphs appear in all copies, modifications, and
 * distributions. 
 *
 * Contact The Office of Technology Licensing, UC Berkeley, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
 * commercial licensing opportunities. 
 *
 * IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 * SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
 * ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION
 * TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS. 
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


static char* version = "2.33";
static char* date = "Mon, 19 May 1997 15:23:04 -0700";


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

#include "lisp.h"
#include "scanner.h"
#include "parser.h"
#include "shic.h"


/* YACC variables */

extern int yydebug;
extern FILE *yyin;


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

int verbose_list_inputs = 0;
int verbose_list_outputs = 0;
int verbose_list_states = 0;
int verbose_list_globals = 0;

void
print_cmd_line_usage(int argc, char **argv)
{
  fprintf(stderr, 
	  "%s: usage: %s [filename] | -v | -l[[i][o][s][g]]\n",
	  argv[0],
	  argv[0]);
}
void 
parse_cmd_line_args(int argc, char *argv[])
{
  int f = 0;

  if(argc > 4)
    {
      print_cmd_line_usage(argc, argv);
      exit (1);
    }
  while(argc-- > 1)
    {
      if(*argv[argc] == '-')
	{
	  char *c;
	  
	  switch(*(++argv[argc]))
	    {
	    case '-':
	      if(strcmp(++argv[argc], "version"))
		{
		  print_cmd_line_usage(argc, argv);
		  exit(1);
		}
	    case 'v':
	      printf("Shift compiler version %s of %s UTC\n",
		     version,
		     date);
	      break;
	    case 'l':
	      while(strcmp(c = ++argv[argc],""))
		{
		  switch(*c)
		    {
		    case 'i':
		      verbose_list_inputs = 1;
		      break;
		    case 'o':
		      verbose_list_outputs = 1;
		      break;
		    case 's':
		      verbose_list_states = 1;
		      break;
		    case 'g':
		      verbose_list_globals = 1;
		      break;
		    default:
		      print_cmd_line_usage(argc, argv);
		      exit(1);
		    }
		}
	      break;
	    default:
	      print_cmd_line_usage(argc, argv);
	      exit(1);
	    }
	}
      else
	{
	  if (! f)
	    {
	      filename = argv[argc];
	      f = 1;
	    }
	  else
	    {
	      fprintf(stderr,
		     "%s: Too many files specified\n",
		     argv[0]);
	      print_cmd_line_usage(argc, argv);
	      exit(1);
	    }
	}
    }
}


int
main(int argc, char **argv)
{

  char cmd_buffer[1024];	/* Big enough for lengthy options. */
  char *cpp;
  int yyin_close_status;
  int keep_intermediate_files_p = 0; /* Hardwired for the time being */
  char *suffix, *slash, *path;
  
  yydebug = 0;	/* For a good time, change this to 1. */



  path = get_path (argv[0]);
  
  if(argc > 1)
    {
      filename = 0;
      parse_cmd_line_args(argc, argv);
      if(! filename)
	exit(0);
      base_file_name = strdup(filename);
      
      /* Find the last period following the last slash. */
      suffix = rindex(base_file_name, '.');
      slash = rindex(base_file_name, '/');
      if (slash > suffix) suffix = 0;
      if (! suffix || strcmp(".hs", suffix))
	{
	  fprintf(stderr, "shic: input file does not have an '.hs'");
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
	  fprintf(stderr, "shic: cannot allocate memory.\n");
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
      
      sprintf(cmd_buffer, "%s %s > %s",
	  cpp, precpp_file_name, cpreproc_file_name);
      system(cmd_buffer);
      
      /*
	sprintf(cmd_buffer, "%s%s %s | %s",
	path,
	"shiftprecpp",
	filename,
	cpp);
	yyin = popen(cmd_buffer, "r");
	*/
      yyin = fopen(cpreproc_file_name, "r");
      if (yyin == NULL)
	{
	  fprintf(stderr, "shic: ");
	  perror(filename);
	  exit(1);
	}
    } /* if(argc > 1) ... */

  /* Front end: parser and type checker. */
  initialize_check();
  yyparse();
  /* yyin_close_status = pclose(yyin); */
  fclose(yyin);

  if (n_errors)
    {
      /* printf("shic: error: C preprocessor pipe status %d\n",
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

  /* patch_differential_variables -- Call.
   * 
   * This is a single call to this function for a temporary relief
   * of inheritance bugs.
   *
   * Tunc Simsek 6th November, 1997
   *
   * patch_differential_variables(Program);
   *
   * The bug has been fixed and the temporary relief is no
   * longer needed!!!
   */

  if (n_errors) exit(1);


  /* Verbose listing of data fields -- Added for
   * Daniel's development of JAWS.
   *
   * Tunc Simsek 10th November, 1997
   */
  verbose_list_data_fields(Program); 
 
  /* Back end: transformations and code generation. */
  cfile = fopen(cfilename, "w");
  hfile = fopen(hfilename, "w");
  if (cfile == NULL)
    {
      fprintf(stderr, "shic: ");
      perror(cfilename);
      exit(1);
    }
  if (hfile == NULL)
    {
      fprintf(stderr, "shic: ");
      perror(hfilename);
      exit(1);
    }
  initialize_transform();
  transform(Program);
  initialize_generate();
  generate(Program);
  fclose(cfile);
  fclose(hfile);
  exit(0);
}

#endif /* COMPILER_MAIN_I */

/* end of file -- main.c -- */
