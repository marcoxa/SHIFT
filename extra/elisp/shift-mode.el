;;; shift-mode.el --- SHIFT code editing commands for Emacs

;;; Copyright (C) 1985, 86, 87, 92, 94, 95 Free Software Foundation, Inc.

;;; Maintainer: FSF
;;; Keywords: shift

;;; This file is part of GNU Emacs.

;;; GNU Emacs is free software; you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 2, or (at your option)
;;; any later version.

;;; GNU Emacs is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.

;;; You should have received a copy of the GNU General Public License
;;; along with GNU Emacs; see the file COPYING.  If not, write to the
;;; Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;;; Boston, MA 02111-1307, USA.

;;; Commentary:

;;; A smart editing mode for SHIFT code.  It knows a lot about SHIFT
;;; syntax and tries to position the cursor according to SHIFT layout
;;; conventions.  You can change the details of the layout style with
;;; option variables.  Load it and do M-x describe-mode for details.

;;; Code:

(require 'cl)

;;; Let's make some sense out of the values returned by
;;; 'parse-partial-sexp'.

(defstruct (parse-state (:type list) (:conc-name pstate-))
  parens-depth				; 0
  ch-addr-start-innermost-list		; 1
  ch-addr-start-last-complete-sexp	; 2
  within-string-p			; 3
  within-comment-p			; 4
  after-quote-ch-p			; 5
  minimum-parens-depth			; 6
  within-alternate-comment-p		; 7
  )


(defvar shift-mode-abbrev-table nil
  "Abbrev table in use in SHIFT mode.")

(define-abbrev-table 'shift-mode-abbrev-table ())

(defvar shift-mode-map (make-sparse-keymap)
  "Keymap used in SHIFT mode.")

(define-key shift-mode-map "{" 'electric-shift-brace)
(define-key shift-mode-map "}" 'electric-shift-brace)
(define-key shift-mode-map ";" 'electric-shift-semi)
(define-key shift-mode-map "#" 'electric-shift-sharp-sign)
(define-key shift-mode-map ":" 'electric-shift-terminator)
(define-key shift-mode-map "\e\C-h" 'mark-shift-function)
(define-key shift-mode-map "\e\C-q" 'indent-shift-exp)
(define-key shift-mode-map "\ea" 'shift-beginning-of-statement)
(define-key shift-mode-map "\ee" 'shift-end-of-statement)
(define-key shift-mode-map "\C-c\C-n" 'shift-forward-conditional)
(define-key shift-mode-map "\C-c\C-p" 'shift-backward-conditional)
(define-key shift-mode-map "\C-c\C-u" 'shift-up-conditional)
(define-key shift-mode-map "\177" 'backward-delete-char-untabify)
(define-key shift-mode-map "\t" 'shift-indent-command)

(define-key shift-mode-map [menu-bar] (make-sparse-keymap))

;;; "SHIFT-mode" is not strictly the right punctuation--it should be
;;; "SHIFT mode"--but that would look like two menu items.
;;; "SHIFT-mode" is the best alternative I can think of.
(define-key shift-mode-map [menu-bar SHIFT]
  (cons "SHIFT-mode" (make-sparse-keymap "SHIFT-mode")))

(define-key shift-mode-map [menu-bar SHIFT comment-region]
  '("Comment Out Region" . comment-region))

(define-key shift-mode-map [menu-bar SHIFT shift-macro-expand]
  '("Macro Expand Region" . shift-macro-expand))

(define-key shift-mode-map [menu-bar SHIFT shift-backslash-region]
  '("Backslashify" . shift-backslash-region))

(define-key shift-mode-map [menu-bar SHIFT indent-exp]
  '("Indent Expression" . indent-shift-exp))

(define-key shift-mode-map [menu-bar SHIFT indent-line]
  '("Indent Line" . shift-indent-command))

(define-key shift-mode-map [menu-bar SHIFT fill]
  '("Fill Comment Paragraph" . shift-fill-paragraph))

(define-key shift-mode-map [menu-bar SHIFT cpp-highlight-buffer]
  '("Highlight Conditionals" . cpp-highlight-buffer))

(define-key shift-mode-map [menu-bar SHIFT up]
  '("Up Conditional" . shift-up-conditional))

(define-key shift-mode-map [menu-bar SHIFT backward]
  '("Backward Conditional" . shift-backward-conditional))

(define-key shift-mode-map [menu-bar SHIFT forward]
  '("Forward Conditional" . shift-forward-conditional))

(define-key shift-mode-map [menu-bar SHIFT backward-stmt]
  '("Backward Statement" . shift-beginning-of-statement))

(define-key shift-mode-map [menu-bar SHIFT forward-stmt]
  '("Forward Statement" . shift-end-of-statement))

(put 'comment-region 'menu-enable 'mark-active)
(put 'shift-macro-expand 'menu-enable 'mark-active)
(put 'shift-backslash-region 'menu-enable 'mark-active)

(autoload 'shift-macro-expand "cmacexp"
  "Display the result of expanding all CPP macros occurring in the region.
The expansion is entirely correct because it uses the CPP preprocessor."
  t)

(defvar shift-mode-syntax-table nil
  "Syntax table in use in SHIFT-mode buffers.")

(unless shift-mode-syntax-table
  (setq shift-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?\\ "\\" shift-mode-syntax-table)
  (modify-syntax-entry ?/ ". 124b" shift-mode-syntax-table)
  (modify-syntax-entry ?* ". 23" shift-mode-syntax-table)
  (modify-syntax-entry ?+ "." shift-mode-syntax-table)
  (modify-syntax-entry ?- "." shift-mode-syntax-table)
  (modify-syntax-entry ?= "." shift-mode-syntax-table)
  (modify-syntax-entry ?% "." shift-mode-syntax-table)
  (modify-syntax-entry ?< "." shift-mode-syntax-table)
  (modify-syntax-entry ?> "." shift-mode-syntax-table)
  (modify-syntax-entry ?& "." shift-mode-syntax-table)
  (modify-syntax-entry ?| "." shift-mode-syntax-table)
  (modify-syntax-entry ?\n "> b"    shift-mode-syntax-table)
  ;; Give CR the same syntax as newline, for selective-display
  (modify-syntax-entry ?\^m "> b"   shift-mode-syntax-table)
  ;; (modify-syntax-entry ?\' "\"" shift-mode-syntax-table)
  )

(defconst shift-indent-level 2
  "*Indentation of SHIFT statements with respect to containing block.")

(defconst shift-brace-imaginary-offset 0
  "*Imagined indentation of a SHIFT open brace that actually follows a statement.")

(defconst shift-brace-offset 0
  "*Extra indentation for braces, compared with other text in same context.")

(defconst shift-argdecl-indent 5
  "*Indentation level of declarations of SHIFT function arguments.")

(defconst shift-label-offset -2
  "*Offset of SHIFT label lines and case statements relative to usual indentation.")

(defconst shift-continued-statement-offset 2
  "*Extra indent for lines not starting new statements.")

(defconst shift-continued-brace-offset 0
  "*Extra indent for substatements that start with open-braces.
This is in addition to `c-continued-statement-offset'.")

(defconst shift-style-alist
  '(("PATH"
     (shift-indent-level               .  2)
     (shift-argdecl-indent             .  5)
     (shift-brace-offset               .  0)
     (shift-continued-brace-offset     .  0)
     (shift-label-offset               . -2)
     (shift-continued-statement-offset .  2))
    ))

(defconst shift-auto-newline nil
  "*Non-nil means automatically newline before and after braces,
and after colons and semicolons, inserted in SHIFT code.
If you do not want a leading newline before braces then use:
  (define-key shift-mode-map \"{\" 'electric-shift-semi)")

(defconst shift-tab-always-indent t
  "*Non-nil means TAB in SHIFT mode should always reindent the current line,
regardless of where in the line point is when the TAB command is used.")

(defconst shift-type-section-regexp
  "\\(discrete\\|input\\|output\\|\\export\\|flow\\|transition\\|state\\|setup\\)[ \t\n]*"
  "Internal regexp used to recognize SHIFT sections within a type.")


;;; This is derived from the C++ version.

(defvar shift-imenu-generic-expression
  (`
   (("Type"
     (, (concat
	 "^"				; beginning of line is required
	 "\\(type\\|Type\\|TYPE\\)[ \t]+"
	 "\\([a-zA-Z0-9_]+\\)"		; this is the string we want to get
	 "[ \t]*[:{]"
	 )) 2)
    ("Foreign Function"
     (, (concat
	 "^"
	 "\\(function\\|Function\\)[ \t]+"
	 "\\([a-zA-Z0-9_]+\\)"		; this is the string we want to get
	 "[ \t]*(.*$"
	 )) 2)
    ("Global Entities"
     (, (concat
	 "^"
	 "\\(global\\|Global\\)[ \t]+"
	 "\\([a-zA-Z0-9_]+\\)[ \t]+"	; this is the string of the type
	 "\\([a-zA-Z0-9_]+\\)"		; this is the string we want to get.
	 )) 3)
    ))
  "Imenu generic expression for SHIFT mode.  See `imenu-generic-expression'.")

(defun shift-mode ()
  "Major mode for editing SHIFT code.
Expression and list commands understand all SHIFT brackets.
Tab indents for SHIFT code.
Comments are delimited with /* ... */.
Paragraphs are separated by blank lines only.
Delete converts tabs to spaces as it moves back.
\\{shift-mode-map}
Variables controlling indentation style:
 shift-tab-always-indent
    Non-nil means TAB in SHIFT mode should always reindent the current line,
    regardless of where in the line point is when the TAB command is used.
 shift-auto-newline
    Non-nil means automatically newline before and after braces,
    and after colons and semicolons, inserted in SHIFT code.
 shift-indent-level
    Indentation of SHIFT statements within surrounding block.
    The surrounding block's indentation is the indentation
    of the line on which the open-brace appears.
 shift-continued-statement-offset
    Extra indentation given to a substatement, such as the
    then-clause of an if or body of a while.
 shift-continued-brace-offset
    Extra indentation given to a brace that starts a substatement.
    This is in addition to shift-continued-statement-offset.
 shift-brace-offset
    Extra indentation for line if it starts with an open brace.
 shift-brace-imaginary-offset
    An open brace following other text is treated as if it were
    this far to the right of the start of its line.
 shift-argdecl-indent
    Indentation level of declarations of SHIFT function arguments.
 shift-label-offset
    Extra indentation for line that is a label, or case or default.

Settings for the PATH indentation style are
     shift-indent-level                2
     shift-argdecl-indent              5
     shift-brace-offset                0
     shift-continued-brace-offset      0
     shift-label-offset               -2
     shift-continued-statement-offset  2

Turning on SHIFT mode calls the value of the variable shift-mode-hook with no args,
if that value is non-nil."
  (interactive)
  (kill-all-local-variables)
  (use-local-map shift-mode-map)
  (setq major-mode 'shift-mode)
  (setq mode-name "SHIFT")
  (setq local-abbrev-table shift-mode-abbrev-table)
  (set-syntax-table shift-mode-syntax-table)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start (concat "$\\|" page-delimiter))
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'paragraph-ignore-fill-prefix)
  (setq paragraph-ignore-fill-prefix t)
  (make-local-variable 'fill-paragraph-function)
  (setq fill-paragraph-function 'shift-fill-paragraph)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'shift-indent-line)
  (make-local-variable 'indent-region-function)
  (setq indent-region-function 'shift-indent-region)
  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)
  (make-local-variable 'outline-regexp)
  (setq outline-regexp "[^#\n\^M]")
  (make-local-variable 'outline-level)
  (setq outline-level 'shift-outline-level)
  (make-local-variable 'comment-start)
  (setq comment-start "//\\|/\\*")
  (make-local-variable 'comment-end)
  (setq comment-end "\\( */\\)?")
  (make-local-variable 'comment-column)
  (setq comment-column 32)
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *\\|//")
  (make-local-variable 'comment-indent-function)
  (setq comment-indent-function 'shift-comment-indent)
  (make-local-variable 'comment-multi-line)
  (setq comment-multi-line t)
  (make-local-variable 'parse-sexp-ignore-comments)
  (setq parse-sexp-ignore-comments t)
  (make-local-variable 'imenu-generic-expression)
  (setq imenu-generic-expression shift-imenu-generic-expression)
  (run-hooks 'shift-mode-hook))

(defun shift-outline-level ()
  (save-excursion
    (skip-chars-forward "\t ")
    (current-column)))


;;; This is used by indent-for-comment
;;; to decide how much to indent a comment in SHIFT code
;;; based on its context.
(defun shift-comment-indent ()
  (if (looking-at "^/\\*")
      0					; Existing comment at bol stays there.
    (let ((opoint (point)))
      (save-excursion
	(beginning-of-line)
	(cond ((looking-at "[ \t]*}[ \t]*\\($\\|/\\*\\)")
	       ;; A comment following a solitary close-brace
	       ;; should have only one space.
	       (search-forward "}")
	       (1+ (current-column)))
	      ((or (looking-at "^#[ \t]*endif[ \t]*")
		   (looking-at "^#[ \t]*else[ \t]*"))
	       7)			; 2 spaces after #endif
	      ((progn
		 (goto-char opoint)
		 (skip-chars-backward " \t")
		 (and (= comment-column 0) (bolp)))
	       ;; If comment-column is 0, and nothing but space
	       ;; before the comment, align it at 0 rather than 1.
	       0)
	      (t
	       (max (1+ (current-column))	; Else indent at comment column
		    comment-column)))))))	; except leave at
					        ; least one space. 


(defun shift-fill-paragraph (&optional arg)
  "Like \\[fill-paragraph] but handle SHIFT comments.
If any of the current line is a comment or within a comment,
fill the comment or the paragraph of it that point is in,
preserving the comment indentation or line-starting decorations."
  (interactive "P")
  (let* (comment-start-place
	 (first-line
	  ;; Check for obvious entry to comment.
	  (save-excursion
	    (beginning-of-line)
	    (skip-chars-forward " \t\n")
	    (and (looking-at comment-start-skip)
		 (setq comment-start-place (point))))))
    (if (and (eq major-mode 'shift-mode)
	     (save-excursion
	       (beginning-of-line)
	       (looking-at ".*//")))
	(let (fill-prefix
	      (paragraph-start
	       ;; Lines containing just a comment start or just an end
	       ;; should not be filled into paragraphs they are next to.
	       (concat
		paragraph-start
		"\\|[ \t]*/\\*[ \t]*$\\|[ \t]*\\*/[ \t]*$\\|[ \t/*]*$"))
	      (paragraph-separate
	       (concat
		paragraph-separate
		"\\|[ \t]*/\\*[ \t]*$\\|[ \t]*\\*/[ \t]*$\\|[ \t/*]*$")))
	  (save-excursion
	    (beginning-of-line)
	    ;; Move up to first line of this comment.
	    (while (and (not (bobp)) (looking-at "[ \t]*//"))
	      (forward-line -1))
	    (when (not (looking-at ".*//"))
	      (forward-line 1))

	    ;; Find the comment start in this line.
	    (re-search-forward "[ \t]*//[ \t]*")

	    ;; Set the fill-prefix to be what all lines except the first
	    ;; should start with.
	    (let ((endcol (current-column)))
	      (skip-chars-backward " \t")
	      (setq fill-prefix
		    (concat (make-string (- (current-column) 2) ?\ )
			    "//"
			    (make-string (- endcol (current-column)) ?\ ))))
	    (save-restriction
	      ;; Narrow down to just the lines of this comment.
	      (narrow-to-region (point)
				(save-excursion
				  (forward-line 1)
				  (while (looking-at "[ \t]*//")
				    (forward-line 1))
				  (point)))
	      (insert fill-prefix)
	      (fill-paragraph arg)
	      (delete-region (point-min)
			     (+ (point-min) (length fill-prefix))))))

      (if (or first-line
	      ;; t if we enter a comment between start of function and
	      ;; this line.
	      (eq (calculate-shift-indent) t)
	      ;; t if this line contains a comment starter.
	      (setq first-line
		    (save-excursion
		      (beginning-of-line)
		      (prog1
			  (re-search-forward comment-start-skip
					     (save-excursion (end-of-line)
							     (point))
					     t)
			(setq comment-start-place (point))))))
	  ;; Inside a comment: fill one comment paragraph.
	  (let ((fill-prefix
		 ;; The prefix for each line of this paragraph
		 ;; is the appropriate part of the start of this line,
		 ;; up to the column at which text should be indented.
		 (save-excursion
		   (beginning-of-line)

		   (if (looking-at "[ \t]*/\\*.*\\*/")
		       (progn (re-search-forward comment-start-skip)
			      (make-string (current-column) ?\ ))
		     (if first-line (forward-line 1))

		     (let ((line-width (progn (end-of-line) (current-column))))
		       (beginning-of-line)
		       (prog1
			   (buffer-substring
			    (point)

			    ;; How shall we decide where the end of
			    ;; the fill-prefix is?
			    ;; calculate-shift-indent-within-comment
			    ;; bases its value on the indentation of
			    ;; previous lines; if they're indented
			    ;; specially, it could return a column
			    ;; that's well into the current line's
			    ;; text.  So we'll take at most that many
			    ;; space, tab, or * characters, and use
			    ;; that as our fill prefix.
			    (let ((max-prefix-end
				   (progn
				     (move-to-column
				      (calculate-shift-indent-within-comment t)
				      t)
				     (point))))
			      (beginning-of-line)
			      (skip-chars-forward " \t*" max-prefix-end)
			      ;; Don't include part of comment terminator
			      ;; in the fill-prefix.
			      (and (eq (following-char) ?/)
				   (eq (preceding-char) ?*)
				   (backward-char 1))
			      (point)))

			 ;; If the comment is only one line followed
			 ;; by a blank line, calling move-to-column
			 ;; above may have added some spaces and tabs
			 ;; to the end of the line; the fill-paragraph
			 ;; function will then delete it and the
			 ;; newline following it, so we'll lose a
			 ;; blank line when we shouldn't.  So delete
			 ;; anything move-to-column added to the end
			 ;; of the line.  We record the line width
			 ;; instead of the position of the old line
			 ;; end because move-to-column might break a
			 ;; tab into spaces, and the new characters
			 ;; introduced there shouldn't be deleted.

			 ;; If you can see a better way to do this,
			 ;; please make the change.  This seems very
			 ;; messy to me.
			 (delete-region (progn (move-to-column line-width)
					       (point))
					(progn (end-of-line) (point))))))))

		(paragraph-start
		 ;; Lines containing just a comment start or just an end
		 ;; should not be filled into paragraphs they are next to.
		 (concat
		  paragraph-start
		  "\\|[ \t]*/\\*[ \t]*$\\|[ \t]*\\*/[ \t]*$\\|[ \t/*]*$"))
		(paragraph-separate
		 (concat
		  paragraph-separate
		  "\\|[ \t]*/\\*[ \t]*$\\|[ \t]*\\*/[ \t]*$\\|[ \t/*]*$"))
		(chars-to-delete 0))
	    (save-restriction
	      ;; Don't fill the comment together with the code
	      ;; following it.  So temporarily exclude everything
	      ;; before the comment start, and everything after the
	      ;; line where the comment ends.  If comment-start-place
	      ;; is non-nil, the comment starter is there.  Otherwise,
	      ;; point is inside the comment.
	      (narrow-to-region (save-excursion
				  (if comment-start-place
				      (goto-char comment-start-place)
				    (search-backward "/*"))
				  ;; Protect text before the comment start
				  ;; by excluding it.  Add spaces to bring back
				  ;; proper indentation of that point.
				  (let ((column (current-column)))
				    (prog1 (point)
				      (setq chars-to-delete column)
				      (insert-char ?\  column))))
				(save-excursion
				  (if comment-start-place
				      (goto-char (+ comment-start-place 2)))
				  (search-forward "*/" nil 'move)
				  (forward-line 1)
				  (point)))
	      (save-excursion
		(goto-char (point-max))
		(forward-line -1)
		;; And comment terminator was on a separate line before,
		;; keep it that way.
		;; This also avoids another problem:
		;; if the fill-prefix ends in a *, it could eat up
		;; the * of the comment terminator.
		(if (looking-at "[ \t]*\\*/")
		    (narrow-to-region (point-min) (point))))
	      (fill-paragraph arg)
	      (save-excursion
		;; Delete the chars we inserted to avoid clobbering
		;; the stuff before the comment start.
		(goto-char (point-min))
		(if (> chars-to-delete 0)
		    (delete-region (point) (+ (point) chars-to-delete)))
		;; Find the comment ender (should be on last line of buffer,
		;; given the narrowing) and don't leave it on its own line.
		;; Do this with a fill command, so as to preserve sentence
		;; boundaries.
		(goto-char (point-max))
		(forward-line -1)
		(search-forward "*/" nil 'move)
		(beginning-of-line)
		(if (looking-at "[ \t]*\\*/")
		    (let ((fill-column (+ fill-column 9999)))
		      (forward-line -1)
		      (fill-region-as-paragraph (point) (point-max)))))))
	;; Outside of comments: do ordinary filling.
	(fill-paragraph arg)))
    t))


(defun electric-shift-brace (arg)
  "Insert character and correct line's indentation."
  (interactive "P")
  (let ((insertpos nil))
    (when (and (not arg)
	       (eolp)
	       (or (save-excursion
		     (skip-chars-backward " \t")
		     (bolp))
		   (when shift-auto-newline
		     (shift-indent-line)
		     (newline)
		     t)))

      (insert last-command-char)
      (shift-indent-line)
      (when shift-auto-newline
	(newline)
	;; (newline) may have done auto-fill
	(setq insertpos (- (point) 2))
	(shift-indent-line))
      (save-excursion
	(when insertpos (goto-char (1+ insertpos)))
	(delete-char -1)))
    (if insertpos
	(save-excursion
	  (goto-char insertpos)
	  (self-insert-command (prefix-numeric-value arg)))
      (self-insert-command (prefix-numeric-value arg)))))


(defun electric-shift-sharp-sign (arg)
  "Insert character and correct line's indentation."
  (interactive "P")
  (if (save-excursion
	(skip-chars-backward " \t")
	(bolp))
      (let ((shift-auto-newline nil))
	(electric-shift-terminator arg))
    (self-insert-command (prefix-numeric-value arg))))


(defun electric-shift-semi (arg)
  "Insert character and correct line's indentation."
  (interactive "P")
  (if shift-auto-newline
      (electric-shift-terminator arg)
    (self-insert-command (prefix-numeric-value arg))))


(defun electric-shift-terminator (arg)
  "Insert character and correct line's indentation."
  (interactive "P")
  (let (insertpos (end (point)))
    (when (and (not arg) (eolp)
	       (not (save-excursion
		      (beginning-of-line)
		      (skip-chars-forward " \t")
		      (or (= (following-char) ?#)
			  ;; Colon is special only after a label, or case ....
			  ;; So quickly rule out most other uses of colon
			  ;; and do no indentation for them.
			  (and (eq last-command-char ?:)
			       (not (looking-at shift-type-section-regexp))
			       (save-excursion
				 (skip-chars-forward "a-zA-Z0-9_$")
				 (skip-chars-forward " \t")
				 (< (point) end)))
			  (progn
			    (beginning-of-defun)
			    (let ((pps (parse-partial-sexp (point) end)))
			      (or (nth 3 pps) (nth 4 pps) (nth 5 pps))))))))
      (insert last-command-char)
      (shift-indent-line)
      (and shift-auto-newline
	   (not (shift-inside-parens-p))
	   (progn
	     (newline)
	     ;; (newline) may have done auto-fill
	     (setq insertpos (- (point) 2))
	     (shift-indent-line)))
      (save-excursion
	(if insertpos (goto-char (1+ insertpos)))
	(delete-char -1)))
    (if insertpos
	(save-excursion
	  (goto-char insertpos)
	  (self-insert-command (prefix-numeric-value arg)))
      (self-insert-command (prefix-numeric-value arg)))))


(defun shift-inside-parens-p ()
  (condition-case ()
      (save-excursion
	(save-restriction
	  (narrow-to-region (point)
			    (progn (beginning-of-defun) (point)))
	  (goto-char (point-max))
	  (= (char-after (or (scan-lists (point) -1 1) (point-min))) ?\()))
    (error nil)))


(defun shift-indent-command (&optional whole-exp)
  "Indent current line as SHIFT code, or in some cases insert a tab character.
If `shift-tab-always-indent' is non-nil (the default), always indent
current line. Otherwise, indent the current line only if point is at
the left margin or in the line's indentation; otherwise insert a tab.

A numeric argument, regardless of its value, means indent rigidly all the
lines of the expression starting after point so that this line becomes
properly indented.  The relative indentation among the lines of the
expression are preserved."
  (interactive "P")
  (cond (whole-exp
	 ;; If arg, always indent this line as SHIFT
	 ;; and shift remaining lines of expression the same amount.
	 (let ((shift-amt (shift-indent-line))
	       (beg nil)
	       (end nil)
	       )
	   (save-excursion
	     (when shift-tab-always-indent
	       (beginning-of-line))
	     ;; Find beginning of following line.
	     (save-excursion
	       (forward-line 1)
	       (setq beg (point)))
	     ;; Find first beginning-of-sexp for sexp extending past this line.
	     (while (< (point) beg)
	       (forward-sexp 1)
	       (setq end (point))
	       (skip-chars-forward " \t\n")))
	   (when (> end beg)
	     (indent-code-rigidly beg end shift-amt "#"))))

	((and (not shift-tab-always-indent)
	      (save-excursion
		(skip-chars-backward " \t")
		(not (bolp))))
	 (insert-tab))

	(t (shift-indent-line))))


(defun shift-indent-line ()
  "Indent current line as SHIFT code.
Return the amount the indentation changed by."
  (let* ((indent (calculate-shift-indent nil))
	 (beg (point))
	 (case-fold-search nil)
	 (pos (- (point-max) (point)))
	 )
    (beginning-of-line)
    (cond ((null indent)
	   (setq indent (current-indentation)))

	  ((eq indent t)
	   (setq indent (calculate-shift-indent-within-comment)))

	  ((looking-at "[ \t]*#")	; Preprocessor line?!?
	   (setq indent 0))

	  (t
	   (skip-chars-forward " \t")
	   (when (listp indent)
	     (setq indent (first indent)))

	   (cond ((or (looking-at shift-type-section-regexp)
		      (and (looking-at "[A-Za-z]")
			   (save-excursion
			     (forward-sexp 1)
			     (looking-at ":"))))
		  (setq indent (max 1 (+ indent shift-label-offset))))

		 ((and (looking-at "else\\b")
		       (not (looking-at "else\\s_")))
		  (setq indent (save-excursion
				 (shift-backward-to-start-of-if)
				 (current-indentation))))

		 ((= (following-char) ?})
		  (setq indent (- indent shift-indent-level)))

		 ((= (following-char) ?{)
		  (setq indent (+ indent shift-brace-offset)))
		 ))			; t
	  )				; cond

    (skip-chars-forward " \t")
    (let ((shift-amt (- indent (current-column)))
	  (x (- (point-max) pos))
	  )
      (unless (zerop shift-amt)
	(delete-region beg (point))
	(indent-to indent)
	;; If initial point was within line's indentation,
	;; position after the indentation.  Else stay at same
	;; point in text. 
	)
      (when (> x (point)) (goto-char x))
      shift-amt)))


(defun calculate-shift-indent-old (&optional parse-start)
  "Return appropriate indentation for current line as SHIFT code.
In usual case returns an integer: the column to indent to.
Returns nil if line starts inside a string, t if in a comment."
  (save-excursion
    (beginning-of-line)
    (let ((indent-point (point))
	  (case-fold-search nil)
	  (state nil)
	  (containing-sexp nil)
	  )
      (if parse-start
	  (goto-char parse-start)
	  (beginning-of-defun))
      (while (< (point) indent-point)
	(setq parse-start (point))
	(setq state (parse-partial-sexp (point) indent-point 0))
	(setq containing-sexp (pstate-ch-addr-start-innermost-list state)))

      (cond ((or (pstate-within-string-p state)
		 (pstate-within-comment-p state))
	     ;; return nil or t if should not change this line
	     (pstate-within-comment-p state))

	    ((null containing-sexp)
	     ;; Line is at top level.
	     ;; We must have either a 'type', 'function' or 'global'
	     ;; definition.
	     (goto-char indent-point)
	     (skip-chars-forward " \t")
	     (cond ((= (following-char) ?{)
		    ;; We just found a 'type' form.
		    0)

		   (t
		    (shift-backward-to-noncomment (or parse-start (point-min)))

		    ;; Look at previous line that's at column 0
		    ;; to determine whether we are in top-level decls
		    ;; or function's arg decls.  Set basic-indent
		    ;; accordingly.

		    ;; The value of the next form is our 'basic indentation'
		    
		    (save-excursion
		      (re-search-backward "^[^ \^L\t\n#]" nil 'move)
		      (let ((comment nil)
			    (lim nil)
			    )
			(cond ((and (looking-at "\\sw\\|\\s_")
				    ;; This is careful to stop at the first
				    ;; paren if we have
				    ;; int foo Proto ((int, int));
				    (looking-at "[^\"\n=(]*(")
				    (progn
				      (goto-char (1- (match-end 0)))
				      ;; Skip any number of paren-groups.
				      ;; Consider typedef int (*fcn) (int);
				      (while (= (following-char) ?\()
					(setq lim (point))
					(condition-case nil
					    (forward-sexp 1)
					  (error))
					(skip-chars-forward " \t\f"))
				      ;; Have we reached something
				      ;; that shows this isn't a function
				      ;; definition?
				      (and (< (point) indent-point)
					   (not (memq (following-char)
						      '(?\, ?\;)))))
				    ;; Make sure the "function decl" we found
				    ;; is not inside a comment.
				    (progn
				      ;; Move back to the `(' starting arglist
				      (goto-char lim)
				      (beginning-of-line)
				      (while (and (not comment)
						  (or (search-forward "/*"
								      lim
								      t)
						      (search-forward "//"
								      lim
								      t)
						      ))
					(setq comment
					      (or (not (search-forward "*/"
								       lim
								       t))
						  (looking-at "//"))
					      ))
				      (not comment)))
			       shift-argdecl-indent)
			      ((looking-at "->")
			       ;; A foreign function return value.
			       ;; Indent as argdecl for the time being.
			       shift-argdecl-indent)
			      (t 0))))
		    )
		   ))

	    ((/= (char-after containing-sexp) ?{)
	     ;; line is expression, not statement:
	     ;; indent to just after the surrounding open.
	     (goto-char (1+ containing-sexp))
	     (current-column))

	    (t
	     ;; Statement level.  Is it a continuation or a new statement?
	     ;; Find previous non-comment character.
	     (goto-char indent-point)
	     (shift-backward-to-noncomment containing-sexp)
	     ;; Back up over label lines, since they don't
	     ;; affect whether our line is a continuation.
	     (while (or (eq (preceding-char) ?\,)
			(and (eq (preceding-char) ?:)
			     (or (eq (char-after (- (point) 2)) ?\')
				 (memq (char-syntax (char-after (- (point) 2)))
				       '(?w ?_)))))
	       (when (eq (preceding-char) ?\,)
		 (forward-char -1)
		 (shift-backward-to-start-of-continued-exp containing-sexp))

	       (beginning-of-line)
	       (shift-backward-to-noncomment containing-sexp))

	     ;; Check for a preprocessor statement or its continuation lines.
	     ;; Move back to end of previous non-preprocessor line,
	     ;; or possibly beginning of buffer.
	     (let ((found (point))
		   (stop nil)
		   )
	       (while (not stop)
		 (beginning-of-line)
		 (cond ((bobp)
			(setq found (point)
			      stop t))
		       ((save-excursion (forward-char -1)
					(= (preceding-char) ?\\))
			(forward-char -1))
		       ;; This line is not preceded by a backslash.
		       ;; So either it starts a preprocessor command
		       ;; or any following continuation lines
		       ;; should not be skipped.
		       ((= (following-char) ?#)
			(forward-char -1)
			(setq found (point)))
		       (t (setq stop t))))
	       (goto-char found))

	     ;; Now we get the answer.
	     (if (and (not (memq (preceding-char) '(0 ?\, ?\; ?\} ?\{)))
		      ;; But don't treat a line with a close-brace
		      ;; as a continuation.  It is probably the
		      ;; end of an enum type declaration.
		      (save-excursion
			(goto-char indent-point)
			(skip-chars-forward " \t")
			(not (= (following-char) ?}))))
		 ;; This line is continuation of preceding line's statement;
		 ;; indent  shift-continued-statement-offset  more than the
		 ;; previous line of the statement.
		 (progn
		   (shift-backward-to-start-of-continued-exp containing-sexp)
		   (+ shift-continued-statement-offset (current-column)
		      (if (save-excursion (goto-char indent-point)
					  (skip-chars-forward " \t")
					  (eq (following-char) ?{))
			  shift-continued-brace-offset
			  0)))

		 ;; This line starts a new statement.
		 ;; Position following last unclosed open.
		 (goto-char containing-sexp)

		 ;; Is line first statement after an open-brace?
		 (or
		  ;; If no, find that first statement and indent like it.
		  (save-excursion
		    (forward-char 1)
		    (let ((colon-line-end 0))
		      (while (progn (skip-chars-forward " \t\n")
				    (looking-at "#\\|/\\*\\|case[ \t\n'/(].*:\\|[a-zA-Z0-9_$]*:"))
			;; Skip over comments and labels following openbrace.
			(cond ((= (following-char) ?\#)
			       (forward-line 1))
			      ((= (following-char) ?\/)
			       (forward-char 2)
			       (search-forward "*/" nil 'move))
			      ;; case or label:
			      (t
			       (save-excursion (end-of-line)
					       (setq colon-line-end (point)))
			       (search-forward ":"))))

		      ;; The first following code counts
		      ;; if it is before the line we want to indent.
		      (and (< (point) indent-point)
			   (-
			    (if (> colon-line-end (point))
				(- (current-indentation) shift-label-offset)
				(current-column))
			    ;; If prev stmt starts with open-brace, that
			    ;; open brace was offset by shift-brace-offset.
			    ;; Compensate to get the column where
			    ;; an ordinary statement would start.
			    (if (= (following-char) ?\{) shift-brace-offset 0)))))
		  ;; If no previous statement,
		  ;; indent it relative to line brace is on.
		  (calculate-shift-indent-after-brace))))))))



(defun calculate-shift-indent (&optional parse-start)
  "Return appropriate indentation for current line as SHIFT code.
In usual case returns an integer: the column to indent to.
Returns nil if line starts inside a string, t if in a comment."
  (save-excursion
    (beginning-of-line)
    (let ((indent-point (point))
	  (case-fold-search nil)
	  (state nil)
	  (containing-sexp nil)
	  )
      (if parse-start
	  (goto-char parse-start)
	  (beginning-of-defun))
      (while (< (point) indent-point)
	(setq parse-start (point))
	(setq state (parse-partial-sexp (point) indent-point 0))
	(setq containing-sexp (pstate-ch-addr-start-innermost-list state)))

      (cond ((or (pstate-within-string-p state)
		 (pstate-within-comment-p state))
	     ;; return nil or t if should not change this line
	     (pstate-within-comment-p state))

	    ((null containing-sexp)
	     ;; Line is at top level.
	     ;; We must have either a 'type', 'function' or 'global'
	     ;; definition.
	     (goto-char indent-point)
	     (skip-chars-forward " \t")
	     (cond ((= (following-char) ?{)
		    ;; We just found a 'type' form.
		    ;; I.e. something like
		    ;;
		    ;; type xxxx
		    ;; {
		    ;;   ...
		    ;; }
		    ;; and 'indent-point' is on the first open brace.
		    0)

		   (t
		    (shift-backward-to-noncomment (or parse-start (point-min)))

		    ;; Look at previous line that's at column 0
		    ;; to determine whether we are in top-level decls
		    ;; or function's arg decls.  Set basic-indent
		    ;; accordingly.

		    ;; The value of the next form is our 'basic indentation'
		    
		    (save-excursion
		      (re-search-backward "^[^ \^L\t\n#]" nil 'move)
		      (let ((comment nil)
			    (lim nil)
			    )
			(cond ((and (looking-at "\\sw\\|\\s_")
				    ;; This is careful to stop at the first
				    ;; paren if we have
				    ;; int foo Proto ((int, int));
				    (looking-at "[^\"\n=(]*(")
				    (progn
				      (goto-char (1- (match-end 0)))
				      ;; Skip any number of paren-groups.
				      ;; Consider typedef int (*fcn) (int);
				      (while (= (following-char) ?\()
					(setq lim (point))
					(condition-case nil
					    (forward-sexp 1)
					  (error))
					(skip-chars-forward " \t\f"))
				      ;; Have we reached something
				      ;; that shows this isn't a function
				      ;; definition?
				      (and (< (point) indent-point)
					   (not (memq (following-char)
						      '(?\, ?\;)))))
				    ;; Make sure the "function decl" we found
				    ;; is not inside a comment.
				    (progn
				      ;; Move back to the `(' starting arglist
				      (goto-char lim)
				      (beginning-of-line)
				      (while (and (not comment)
						  (or (search-forward "/*"
								      lim
								      t)
						      (search-forward "//"
								      lim
								      t)
						      ))
					(setq comment
					      (or (not (search-forward "*/"
								       lim
								       t))
						  (looking-at "//"))
					      ))
				      (not comment)))
			       shift-argdecl-indent)
			      ((looking-at "->")
			       ;; A foreign function return value.
			       ;; Indent as argdecl for the time being.
			       shift-argdecl-indent)
			      (t 0))))
		    )
		   ))


	    ;; Ok. Line is not at top level.
	    ;; Now we must distinguish among a bunch of different cases.

	    ((/= (char-after containing-sexp) ?{)
	     ;; line is expression, not statement:
	     ;; indent to just after the surrounding open.
	     (goto-char (1+ containing-sexp))
	     (current-column))

	    (t
	     ;; Statement level.  Is it a continuation or a new statement?
	     ;; Find previous non-comment character.
	     (goto-char indent-point)
	     (shift-backward-to-noncomment containing-sexp)
	     ;; Back up over label lines, since they don't
	     ;; affect whether our line is a continuation.
	     (while (or (eq (preceding-char) ?\,)
			(and (eq (preceding-char) ?:)
			     (or (eq (char-after (- (point) 2)) ?\')
				 (memq (char-syntax (char-after (- (point) 2)))
				       '(?w ?_)))))
	       (when (eq (preceding-char) ?\,)
		 (forward-char -1)
		 (shift-backward-to-start-of-continued-exp containing-sexp))

	       (beginning-of-line)
	       (shift-backward-to-noncomment containing-sexp))

	     ;; Check for a preprocessor statement or its continuation lines.
	     ;; Move back to end of previous non-preprocessor line,
	     ;; or possibly beginning of buffer.
	     (let ((found (point))
		   (stop nil)
		   )
	       (while (not stop)
		 (beginning-of-line)
		 (cond ((bobp)
			(setq found (point)
			      stop t))
		       ((save-excursion (forward-char -1)
					(= (preceding-char) ?\\))
			(forward-char -1))
		       ;; This line is not preceded by a backslash.
		       ;; So either it starts a preprocessor command
		       ;; or any following continuation lines
		       ;; should not be skipped.
		       ((= (following-char) ?#)
			(forward-char -1)
			(setq found (point)))
		       (t (setq stop t))))
	       (goto-char found))

	     ;; Now we get the answer.
	     (if (and (not (memq (preceding-char)
				 ;; '(0 ?\, ?\; ?\} ?\{)
				 '(0 ?\; ?\} ?\{)
				 ))
		      ;; Contrary to C mode, we treat the closed brace
		      ;; as a continuation.  Mostly because of 'when' clauses.
		      (save-excursion
			(goto-char indent-point)
			(skip-chars-forward " \t")
			(not (= (following-char) ?}))))
		 ;; This line is continuation of preceding line's statement;
		 ;; indent  shift-continued-statement-offset  more than the
		 ;; previous line of the statement.
		 (progn
		   (shift-backward-to-start-of-continued-exp containing-sexp)
		   (+ shift-continued-statement-offset
		      (current-column)
		      (if (save-excursion (goto-char indent-point)
					  (skip-chars-forward " \t")
					  (eq (following-char) ?{))
			  shift-continued-brace-offset
			  0)))

		 ;; This line starts a new statement.
		 ;; Position following last unclosed open.
		 (goto-char containing-sexp)

		 ;; Is line first statement after an open-brace?
		 (or
		  ;; If no, find that first statement and indent like it.
		  (save-excursion
		    (forward-char 1)
		    (let ((colon-line-end 0))
		      (while (progn (skip-chars-forward " \t\n")
				    (looking-at "#\\|/\\*\\|case[ \t\n'/(].*:\\|[a-zA-Z0-9_$]*:"))
			;; Skip over comments and labels following openbrace.
			(cond ((= (following-char) ?\#)
			       (forward-line 1))
			      ((= (following-char) ?\/)
			       (forward-char 2)
			       (search-forward "*/" nil 'move))
			      ;; case or label:
			      (t
			       (save-excursion (end-of-line)
					       (setq colon-line-end (point)))
			       (search-forward ":"))))

		      ;; The first following code counts
		      ;; if it is before the line we want to indent.
		      (and (< (point) indent-point)
			   (-
			    (if (> colon-line-end (point))
				(- (current-indentation) shift-label-offset)
				(current-column))
			    ;; If prev stmt starts with open-brace, that
			    ;; open brace was offset by shift-brace-offset.
			    ;; Compensate to get the column where
			    ;; an ordinary statement would start.
			    (if (= (following-char) ?\{)
				shift-brace-offset
				0)))))
		  ;; If no previous statement,
		  ;; indent it relative to line brace is on.
		  (calculate-shift-indent-after-brace))))))))


(defun calculate-shift-indent-after-brace ()
  "Return the proper SHIFT indent for the first line after an open-brace.
This function is called with point before the brace."
  ;; For open brace in column zero, don't let statement
  ;; start there too.  If shift-indent-level is zero,
  ;; use shift-brace-offset + shift-continued-statement-offset instead.
  ;; For open-braces not the first thing in a line,
  ;; add in shift-brace-imaginary-offset.
  (+ (if (and (bolp) (zerop shift-indent-level))
	 (+ shift-brace-offset shift-continued-statement-offset)
       shift-indent-level)
     ;; Move back over whitespace before the openbrace.
     ;; If openbrace is not first nonwhite thing on the line,
     ;; add the shift-brace-imaginary-offset.
     (progn (skip-chars-backward " \t")
	    (if (bolp) 0 shift-brace-imaginary-offset))
     ;; If the openbrace is preceded by a parenthesized exp,
     ;; move to the beginning of that;
     ;; possibly a different line
     (progn
       (if (eq (preceding-char) ?\))
	   (forward-sexp -1))
       ;; Get initial indentation of the line we are on.
       (current-indentation))))


(defun calculate-shift-indent-within-comment (&optional after-star)
  "Return the indentation amount for line inside a block comment.
Optional arg AFTER-STAR means, if lines in the comment have a leading star,
return the indentation of the text that would follow this star."
  (let (end star-start)
    (save-excursion
      (beginning-of-line)
      (skip-chars-forward " \t")
      (setq star-start (= (following-char) ?\*))
      (skip-chars-backward " \t\n")
      (setq end (point))
      (beginning-of-line)
      (skip-chars-forward " \t")
      (if after-star
	  (and (looking-at "\\*")
	       (re-search-forward "\\*[ \t]*")))
      (and (re-search-forward "/\\*[ \t]*" end t)
	   star-start
	   (not after-star)
	   (goto-char (1+ (match-beginning 0))))
      (if (and (looking-at "[ \t]*$") (= (preceding-char) ?\*))
	  (1+ (current-column))
	(current-column)))))


(defun shift-backward-to-noncomment (lim)
  (let ((opoint nil)
	(stop nil)
	)
    (while (not stop)
      (skip-chars-backward " \t\n\f" lim)
      (setq opoint (point))
      (cond ((and (>= (point) (+ 2 lim))
		  (save-excursion
		    (forward-char -2)
		    (looking-at "\\*/")))
	     (search-backward "/*" lim 'move))
	    (t
	     (setq stop (or (<= (point) lim)
			    (save-excursion
			      (beginning-of-line)
			      (skip-chars-forward " \t")
			      (not (looking-at "#")))))
	     (or stop (beginning-of-line)))))))


(defun shift-backward-to-start-of-continued-exp (lim)
  (when (memq (preceding-char) '(?\) ?\"))
    (forward-sexp -1))
  (beginning-of-line)
  (when (<= (point) lim)
    (goto-char (1+ lim)))
  (skip-chars-forward " \t"))


(defun shift-backward-to-start-of-if (&optional limit)
  "Move to the start of the last \"unbalanced\" `if'."
  (or limit (setq limit (save-excursion (beginning-of-defun) (point))))
  (let ((if-level 1)
	(case-fold-search nil))
    (while (and (not (bobp)) (not (zerop if-level)))
      (backward-sexp 1)
      (cond ((and (looking-at "else\\b")
		  (not (looking-at "else\\s_")))
	     (setq if-level (1+ if-level)))
	    ((and (looking-at "if\\b")
		  (not (looking-at "if\\s_")))
	     (setq if-level (1- if-level)))
	    ((< (point) limit)
	     (setq if-level 0)
	     (goto-char limit))))))


(defun shift-backward-to-start-of-do (&optional limit)
  "If point follows a `do' statement, move to beginning of it and return t.
Otherwise return nil and don't move point."
  (or limit (setq limit (save-excursion (beginning-of-defun) (point))))
  (let ((first t)
	(startpos (point))
	(done nil))
    (while (not done)
      (let ((next-start (point)))
	(condition-case nil
	    ;; Move back one token or one brace or paren group.
	    (backward-sexp 1)
	  ;; If we find an open-brace, we lose.
	  (error (setq done 'fail)))
	(if done
	    nil
	  ;; If we reached a `do', we win.
	  (if (looking-at "do\\b")
	      (setq done 'succeed)
	    ;; Otherwise, if we skipped a semicolon, we lose.
	    ;; (Exception: we can skip one semicolon before getting
	    ;; to a the last token of the statement, unless that token
	    ;; is a close brace.)
	    (if (save-excursion
		  (forward-sexp 1)
		  (or (and (not first) (= (preceding-char) ?}))
		      (search-forward ";" next-start t
				      (if (and first
					       (/= (preceding-char) ?}))
					  2 1))))
		(setq done 'fail)
	      (setq first nil)
	      ;; If we go too far back in the buffer, we lose.
	      (if (< (point) limit)
		  (setq done 'fail)))))))
    (if (eq done 'succeed)
	t
      (goto-char startpos)
      nil)))


(defun shift-beginning-of-statement (count)
  "Go to the beginning of the innermost SHIFT statement.
With prefix arg, go back N - 1 statements.  If already at the beginning of a
statement then go to the beginning of the preceding one.
If within a string or comment, or next to a comment (only whitespace between),
move by sentences instead of statements."
  (interactive "p")
  (let ((here (point))
	(state ())
	)
    (save-excursion
      (beginning-of-defun)
      (setq state (parse-partial-sexp (point) here nil nil)))
    (cond ((or (pstate-within-string-p state) (pstate-within-comment-p state)
	       (looking-at (concat "[ \t]*" comment-start-skip))
	       (save-excursion (skip-chars-backward " \t")
			       (goto-char (- (point) 2))
			       (looking-at "\\*/")))
	   (forward-sentence (- count)))
	  (t (while (> count 0)
	       (shift-beginning-of-statement-1)
	       (setq count (1- count)))
	     (while (< count 0)
	       (shift-end-of-statement-1)
	       (setq count (1+ count)))))
    ))


(defun shift-end-of-statement (count)
  "Go to the end of the innermost SHIFT statement.
With prefix arg, go forward N - 1 statements.
Move forward to end of the next statement if already at end.
If within a string or comment, move by sentences instead of statements."
  (interactive "p")
  (shift-beginning-of-statement (- count)))


(defun shift-beginning-of-statement-1 ()
  (let ((last-begin (point))
	(first t))
    (condition-case ()
	(progn
	  (while (and (not (bobp))
		      (progn
			(backward-sexp 1)
			(or first
			    (not (re-search-forward "[;{}]" last-begin t)))))
	    (setq last-begin (point) first nil))
	  (goto-char last-begin))
      (error (if first (backward-up-list 1) (goto-char
  last-begin))))))


(defun shift-end-of-statement-1 ()
  (condition-case ()
      (progn
	(while (and (not (eobp))
		    (let ((beg (point)))
		      (forward-sexp 1)
		      (let ((end (point)))
			(save-excursion
			  (goto-char beg)
			  (not (re-search-forward "[;{}]" end t)))))))
	(re-search-backward "[;}]")
	(forward-char 1))
    (error
     (let ((beg (point)))
       (backward-up-list -1)
       (let ((end (point)))
	 (goto-char beg)
	 (search-forward ";" end 'move))))))


(defun mark-shift-function ()
  "Put mark at end of SHIFT function, point at beginning."
  (interactive)
  (push-mark (point))
  (end-of-defun)
  (push-mark (point) nil t)
  (beginning-of-defun)
  (backward-paragraph))


;;; Idea of ENDPOS is, indent each line, stopping when
;;; ENDPOS is encountered.  But it's too much of a pain to make that work.
(defun indent-shift-exp (&optional endpos)
  "Indent each line of the SHIFT grouping following point."
  (interactive)
  (let* ((indent-stack (list nil))
	 (opoint (point))  ;; May be altered below.
	 (contain-stack
	  (list (if endpos
		    (let (funbeg)
		      ;; Find previous fcn-start.
		      (save-excursion (forward-char 1)
				      (beginning-of-defun)
				      (setq funbeg (point)))
		      (setq opoint funbeg)
		      ;; Try to find containing open,
		      ;; but don't scan past that fcn-start.
		      (save-restriction
			(narrow-to-region funbeg (point))
			(condition-case nil
			    (save-excursion
			      (backward-up-list 1)
			      (point))
			  ;; We gave up: must be between fcns.
			  ;; Set opoint to beg of prev fcn
			  ;; since otherwise calculate-shift-indent
			  ;; will get wrong answers.
			  (error (setq opoint funbeg)
				 (point)))))
		  (point))))
	 (case-fold-search nil)
	 restart outer-loop-done inner-loop-done state ostate
	 this-indent last-sexp
	 at-else at-brace at-while
	 last-depth this-point
	 (next-depth 0))
    ;; If the braces don't match, get an error right away.
    (save-excursion
      (forward-sexp 1))
    ;; Realign the comment on the first line, even though we don't reindent it.
    (save-excursion
      (let ((beg (point)))
	(and (re-search-forward
	      comment-start-skip
	      (save-excursion (end-of-line) (point)) t)
	     ;; Make sure this isn't a comment alone on a line
	     ;; (which should be indented like code instead).
	     (save-excursion
	       (goto-char (match-beginning 0))
	       (skip-chars-backward " \t")
	       (not (bolp)))
	     ;; Make sure the comment starter we found
	     ;; is not actually in a string or quoted.
	     (let ((new-state
		    (parse-partial-sexp beg (point)
					nil nil state)))
	       (and (not (nth 3 new-state)) (not (nth 5 new-state))))
	    (progn (indent-for-comment) (beginning-of-line)))))
    (save-excursion
      (setq outer-loop-done nil)
      (while (and (not (eobp))
		  (if endpos (< (point) endpos)
		    (not outer-loop-done)))
	(setq last-depth next-depth)
	;; Compute how depth changes over this line
	;; plus enough other lines to get to one that
	;; does not end inside a comment or string.
	;; Meanwhile, do appropriate indentation on comment lines.
	(setq inner-loop-done nil)
	(while (and (not inner-loop-done)
		    (not (and (eobp) (setq outer-loop-done t))))
	  (setq ostate state)
	  (setq state (parse-partial-sexp (point) (progn (end-of-line) (point))
					  nil nil state))
	  (setq next-depth (car state))
	  (if (and (car (cdr (cdr state)))
		   (>= (car (cdr (cdr state))) 0))
	      (setq last-sexp (car (cdr (cdr state)))))
	  ;; If this line started within a comment, indent it as such.
	  (if (or (nth 4 ostate) (nth 7 ostate))
	      (shift-indent-line))
	  ;; If it ends outside of comments or strings, exit the inner loop.
	  ;; Otherwise move on to next line.
	  (if (or (nth 3 state) (nth 4 state) (nth 7 state))
	      (forward-line 1)
	    (setq inner-loop-done t)))
	(and endpos
	     (while (< next-depth 0)
	       (setq indent-stack (append indent-stack (list nil)))
	       (setq contain-stack (append contain-stack (list nil)))
	       (setq next-depth (1+ next-depth))
	       (setq last-depth (1+ last-depth))
	       (setcar (nthcdr 6 state) (1+ (nth 6 state)))))
	(setq outer-loop-done (and (not endpos) (<= next-depth 0)))
	(if outer-loop-done
	    nil
	  ;; If this line had ..))) (((.. in it, pop out of the levels
	  ;; that ended anywhere in this line, even if the final depth
	  ;; doesn't indicate that they ended.
	  (while (> last-depth (nth 6 state))
	    (setq indent-stack (cdr indent-stack)
		  contain-stack (cdr contain-stack)
		  last-depth (1- last-depth)))
	  (if (/= last-depth next-depth)
	      (setq last-sexp nil))
	  ;; Add levels for any parens that were started in this line.
	  (while (< last-depth next-depth)
	    (setq indent-stack (cons nil indent-stack)
		  contain-stack (cons nil contain-stack)
		  last-depth (1+ last-depth)))
	  (if (null (car contain-stack))
	      (setcar contain-stack (or (car (cdr state))
					(save-excursion (forward-sexp -1)
							(point)))))
	  (forward-line 1)
	  (skip-chars-forward " \t")
	  ;; Don't really reindent if the line is just whitespace,
	  ;; or if it is past the endpos.
	  ;; (The exit test in the outer while
	  ;; does not exit until we have passed the first line
	  ;; past the region.)
	  (if (or (eolp) (and endpos (>= (point) endpos)))
	      nil
	    ;; Is this line in a new nesting level?
	    ;; In other words, is this the first line that
	    ;; starts in the new level?
	    (if (and (car indent-stack)
		     (>= (car indent-stack) 0))
		nil
	      ;; Yes.
	      ;; Compute the standard indent for this level.
	      (let (val)
		(if (= (char-after (car contain-stack)) ?{)
		    (save-excursion
		      (goto-char (car contain-stack))
		      (setq val (calculate-shift-indent-after-brace)))
		  (setq val (calculate-shift-indent
			     (if (car indent-stack)
				 (- (car indent-stack))
			       opoint))))
		;; t means we are in a block comment and should
		;; calculate accordingly.
		(if (eq val t)
		    (setq val (calculate-shift-indent-within-comment)))
		(setcar indent-stack val)))
	    ;; Adjust indent of this individual line
	    ;; based on its predecessor.
	    ;; Handle continuation lines, if, else, while, and so on.
	    (if (/= (char-after (car contain-stack)) ?{)
		(setq this-indent (car indent-stack))
	      ;; Line is at statement level.
	      ;; Is it a new statement?  Is it an else?
	      ;; Find last non-comment character before this line
	      (save-excursion
		(setq this-point (point))
		(setq at-else (and (looking-at "else\\b")
				   (not (looking-at "else\\s_"))))
		(setq at-brace (= (following-char) ?{))
		(setq at-while (and (looking-at "while\\b")
				    (not (looking-at "while\\s_"))))
		(if (= (following-char) ?})
		    (setq this-indent (car indent-stack))
		  (shift-backward-to-noncomment opoint)
		  (if (not (memq (preceding-char) '(0 ?\, ?\; ?} ?: ?{)))
		      ;; Preceding line did not end in comma or semi;
		      ;; indent this line  shift-continued-statement-offset
		      ;; more than previous.
		      (progn
			(shift-backward-to-start-of-continued-exp (car contain-stack))
			(setq this-indent
			      (+ shift-continued-statement-offset (current-column)
				 (if at-brace shift-continued-brace-offset 0))))
		    ;; Preceding line ended in comma or semi;
		    ;; use the standard indent for this level.
		    (cond (at-else (progn (shift-backward-to-start-of-if opoint)
					  (setq this-indent
						(current-indentation))))
			  ((and at-while (shift-backward-to-start-of-do opoint))
			   (setq this-indent (current-indentation)))
			  ((eq (preceding-char) ?\,)
			   (goto-char this-point)
			   (setq this-indent (calculate-shift-indent)))
			  (t (setq this-indent (car indent-stack))))))))
	    ;; Adjust line indentation according to its contents
	    (if (or (looking-at shift-type-section-regexp)
		    (and (looking-at "[A-Za-z]")
			 (save-excursion
			   (forward-sexp 1)
			   (looking-at ":"))))
		(setq this-indent (max 1 (+ this-indent shift-label-offset))))
	    (if (= (following-char) ?})
		(setq this-indent (- this-indent shift-indent-level)))
	    (if (= (following-char) ?{)
		;; Don't move an open-brace in column 0.
		;; This is good when constructs such as
		;; `extern "C" {' surround a function definition
		;; that should be indented as usual.
		;; It is also good for nested functions.
		;; It is bad when an open-brace is indented at column 0
		;; and you want to fix that, but we can't win 'em all.
		(if (zerop (current-column))
		    (setq this-indent 0)
		  (setq this-indent (+ this-indent shift-brace-offset))))
	    ;; Don't leave indentation in empty lines.
	    (if (eolp) (setq this-indent 0))
	    ;; Put chosen indentation into effect.
	    (or (= (current-column) this-indent)
		(= (following-char) ?\#)
		(progn
		  (delete-region (point) (progn (beginning-of-line) (point)))
		  (indent-to this-indent)))
	    ;; Indent any comment following the text.
	    (or (looking-at comment-start-skip)
		(save-excursion
		  (let ((beg (point)))
		    (and (re-search-forward
			  comment-start-skip
			  (save-excursion (end-of-line) (point)) t)
			 ;; Make sure the comment starter we found
			 ;; is not actually in a string or quoted.
			 (let ((new-state
				(parse-partial-sexp beg (point)
						    nil nil state)))
			   (and (not (nth 3 new-state)) (not (nth 5 new-state))))
			 (indent-for-comment)))))))))))


;;; Look at all comment-start strings in the current line after point.
;;; Return t if one of them starts a real comment.
;;; This is not used yet, because indent-for-comment
;;; isn't smart enough to handle the cases this can find.
(defun indent-shift-find-real-comment ()
  (let (win)
    (while (and (not win)
		(re-search-forward comment-start-skip
				   (save-excursion (end-of-line) (point))
				   t))
      ;; Make sure the comment start is not quoted.
      (let ((state-1
	     (parse-partial-sexp
	      (save-excursion (beginning-of-line) (point))
	      (point) nil nil state)))
	(setq win (and (null (nth 3 state-1)) (null (nth 5 state-1))))))
    win))


;;; Indent every line whose first char is between START and END inclusive.
(defun shift-indent-region (start end)
  (save-excursion
    (goto-char start)
    ;; Advance to first nonblank line.
    (skip-chars-forward " \t\n")
    (beginning-of-line)
    (let ((endmark (copy-marker end))
	  (shift-tab-always-indent t))
      (while (and (bolp) (not (eobp)) (< (point) endmark))
	;; Indent one line as with TAB.
	(let ((shift-amt (shift-indent-line))
	      nextline sexpbeg sexpend)
	  (if (save-excursion (beginning-of-line) (looking-at "[ \t]*#"))
	      (forward-line 1)
	    (save-excursion
	      ;; Find beginning of following line.
	      (save-excursion
		(forward-line 1) (setq nextline (point)))
	      ;; Find first beginning-of-sexp for sexp extending past this line.
	      (beginning-of-line)
	      (while (< (point) nextline)
		(condition-case nil
		    (progn
		      (forward-sexp 1)
		      (setq sexpend (point-marker)))
		  (error (setq sexpend nil)
			 (goto-char nextline)))
		(skip-chars-forward " \t\n"))
	      (if sexpend
		  (progn
		    ;; Make sure the sexp we found really starts on the
		    ;; current line and extends past it.
		    (goto-char sexpend)
		    (backward-sexp 1)
		    (setq sexpbeg (point)))))
	    ;; If that sexp ends within the region,
	    ;; indent it all at once, fast.
	    (if (and sexpend (> sexpend nextline) (<= sexpend endmark)
		     (< sexpbeg nextline))
		(progn
		  (indent-shift-exp)
		  (goto-char sexpend)))
	    ;; Move to following line and try again.
	    (and sexpend (set-marker sexpend nil))
	    (forward-line 1))))
      (set-marker endmark nil))))


(defun set-shift-style (style &optional global)
  "Set shift-mode variables to use one of several different indentation styles.
The arguments are a string representing the desired style
and a flag which, if non-nil, means to set the style globally.
\(Interactively, the flag comes from the prefix argument.)
Available styles are GNU, K&R, BSD and Whitesmith."
  (interactive (list (let ((completion-ignore-case t))
		       (completing-read "Use which SHIFT indentation style? "
					c-style-alist nil t))
		     current-prefix-arg))
  (let ((vars (cdr (assoc style shift-style-alist))))
    (or vars
	(error "Invalid SHIFT indentation style `%s'" style))
    (while vars
      (or global
	  (make-local-variable (car (car vars))))
      (set (car (car vars)) (cdr (car vars)))
      (setq vars (cdr vars)))))


;;; This page handles insertion and removal of backslashes for SHIFT macros.

(defvar shift-backslash-column 48
  "*Minimum column for end-of-line backslashes of macro definitions.")


(defun shift-backslash-region (from to delete-flag)
  "Insert, align, or delete end-of-line backslashes on the lines in the region.
With no argument, inserts backslashes and aligns existing backslashes.
With an argument, deletes the backslashes.

This function does not modify the last line of the region if the region ends
right at the start of the following line; it does not modify blank lines
at the start of the region.  So you can put the region around an entire macro
definition and conveniently use this command."
  (interactive "r\nP")
  (save-excursion
    (goto-char from)
    (let ((column shift-backslash-column)
	  (endmark (make-marker)))
      (move-marker endmark to)
      ;; Compute the smallest column number past the ends of all the lines.
      (if (not delete-flag)
	  (while (< (point) to)
	    (end-of-line)
	    (if (= (preceding-char) ?\\)
		(progn (forward-char -1)
		       (skip-chars-backward " \t")))
	    (setq column (max column (1+ (current-column))))
	    (forward-line 1)))
      ;; Adjust upward to a tab column, if that doesn't push past the margin.
      (if (> (% column tab-width) 0)
	  (let ((adjusted (* (/ (+ column tab-width -1) tab-width) tab-width)))
	    (if (< adjusted (window-width))
		(setq column adjusted))))
      ;; Don't modify blank lines at start of region.
      (goto-char from)
      (while (and (< (point) endmark) (eolp))
	(forward-line 1))
      ;; Add or remove backslashes on all the lines.
      (while (and (< (point) endmark)
		  ;; Don't backslashify the last line
		  ;; if the region ends right at the start of the next line.
		  (save-excursion
		    (forward-line 1)
		    (< (point) endmark)))
	(if (not delete-flag)
	    (shift-append-backslash column)
	  (shift-delete-backslash))
	(forward-line 1))
      (move-marker endmark nil))))


(defun shift-append-backslash (column)
  (end-of-line)
  ;; Note that "\\\\" is needed to get one backslash.
  (if (= (preceding-char) ?\\)
      (progn (forward-char -1)
	     (delete-horizontal-space)
	     (indent-to column))
    (indent-to column)
    (insert "\\")))


(defun shift-delete-backslash ()
  (end-of-line)
  (or (bolp)
      (progn
	(forward-char -1)
	(if (looking-at "\\\\")
	    (delete-region (1+ (point))
			   (progn (skip-chars-backward " \t")
  (point)))))))


(defun shift-up-conditional (count)
  "Move back to the containing preprocessor conditional, leaving mark behind.
A prefix argument acts as a repeat count.  With a negative argument,
move forward to the end of the containing preprocessor conditional.
When going backwards, `#elif' is treated like `#else' followed by `#if'.
When going forwards, `#elif' is ignored."
  (interactive "p")
  (shift-forward-conditional (- count) t))


(defun shift-backward-conditional (count &optional up-flag)
  "Move back across a preprocessor conditional, leaving mark behind.
A prefix argument acts as a repeat count.  With a negative argument,
move forward across a preprocessor conditional."
  (interactive "p")
  (shift-forward-conditional (- count) up-flag))


(defun shift-forward-conditional (count &optional up-flag)
  "Move forward across a preprocessor conditional, leaving mark behind.
A prefix argument acts as a repeat count.  With a negative argument,
move backward across a preprocessor conditional."
  (interactive "p")
  (let* ((forward (> count 0))
	 (increment (if forward -1 1))
	 (search-function (if forward 're-search-forward 're-search-backward))
	 (opoint (point))
	 (new))
    (save-excursion
      (while (/= count 0)
	(let ((depth (if up-flag 0 -1)) found)
	  (save-excursion
	    ;; Find the "next" significant line in the proper direction.
	    (while (and (not found)
			;; Rather than searching for a # sign that comes
			;; at the beginning of a line aside from whitespace,
			;; search first for a string starting with # sign.
			;; Then verify what precedes it.
			;; This is faster on account of the fastmap feature of
			;; the regexp matcher.
			(funcall search-function
				 "#[ \t]*\\(if\\|elif\\|endif\\)"
				 nil t))
	      (beginning-of-line)
	      ;; Now verify it is really a preproc line.
	      (if (looking-at "^[ \t]*#[ \t]*\\(if\\|elif\\|endif\\)")
		  (let ((prev depth))
		    ;; Update depth according to what we found.
		    (beginning-of-line)
		    (cond ((looking-at "[ \t]*#[ \t]*endif")
			   (setq depth (+ depth increment)))
			  ((looking-at "[ \t]*#[ \t]*elif")
			   (if (and forward (= depth 0))
			       (setq found (point))))
			  (t (setq depth (- depth increment))))
		    ;; If we are trying to move across, and we find
		    ;; an end before we find a beginning, get an error.
		    (if (and (< prev 0) (< depth prev))
			(error (if forward
				   "No following conditional at this level"
				 "No previous conditional at this level")))
		    ;; When searching forward, start from next line
		    ;; so that we don't find the same line again.
		    (if forward (forward-line 1))
		    ;; If this line exits a level of conditional, exit inner loop.
		    (if (< depth 0)
			(setq found (point))))
		;; If the line is not really a conditional, skip past it.
		(if forward (end-of-line)))))
	  (or found
	      (error "No containing preprocessor conditional"))
	  (goto-char (setq new found)))
	(setq count (+ count increment))))
    (push-mark)
    (goto-char new)))


(provide 'shift-mode)

;;; shift-mode.el ends here
