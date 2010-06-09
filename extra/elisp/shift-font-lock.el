;; shift-font-lock.el -- SHIFT reserved words and font lock keywords.

;; Fred White (fwhite@bbn.com)

;; In order to accurately pick out method and constructor definitions,
;; some of these regexp's assume Sun's coding conventions, ie, that
;; class names begin with a upper-case letter and method names begin
;; with a lower-case letter.  This seemed like the best compromise
;; between accuracy and messiness.

;; Since some of the regexps are generally useful, I've placed them
;; into separate defvar's.

;; Change History:
;;  30May96: First version
;;  12Jun96: Handle constructors, default visibilility, $ in identifiers, etc.

;; Example usage:
;;  (add-hook 'shift-mode-hook '(lambda () (shift-font-lock-mode 'gaudy)))


(defvar shift-primitive-types
  '("number"))

(defvar shift-storage-classes
  '("continuous"))

(defvar shift-visibility-modifiers
  '())

;; SHIFT reserved words, other than those above.
(defvar shift-reserved-words
  '("type" "export" "input" "output" "discrete" "flow" "state" "setup"
    "transition" "default" "define" "when" "do" "if" "then" "else"
    "array" "set" "exists" "in" "create" "not" "and" "or" "by"
    "ascending" "descending"))

(defvar shift-identifier-regexp "[a-zA-Z$_][a-zA-Z0-9$_.]*")

;; Any SHIFT identifier may contain one of these, too.
(defvar shift-unicode-regexp "\\\\u[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]")


(defun shift-regexpify-list (list &optional noparens)
  (concat (if noparens "" "\\(")
	  (mapconcat 'identity list "\\|")
	  (if noparens "" "\\)")))


(defvar shift-modifiers-regexp
  (shift-regexpify-list
   (append shift-visibility-modifiers shift-storage-classes)))


;;; Class and interface names.
(defvar shift-class-regexp 
  (concat "^[ \t]*\\(type\\|Type\\|TYPE\\)[ \t]+\\("
	  shift-identifier-regexp
	  "\\)"))

;; The hand-wave here is to assume that the names of user-defined
;; types (classes) begin with a capital letter, in order to
;; distinguish them from reserved words.  Loosening this constraint
;; causes, eg, "return sum(...)", to appear to be a method declaration.

(defvar shift-class-name-regexp "[A-Z$_][a-zA-Z0-9$_]*")


(defvar shift-font-lock-keywords nil "Expressions to highlight in SHIFT mode.")

(defvar shift-font-lock-keywords-1
  (list
   ;; C++ style comments.
   '("//.*$" . font-lock-comment-face)

   ;; Definition names.
   (list shift-class-regexp 2 'font-lock-function-name-face t)
   
   ;; Case clauses.
   (cons (concat "^[ \t]*case[ \t]+\\([^:]*\\):")
	 1)
   (cons "^[ \t]*\\(default\\):"
	 1)))


(defvar shift-font-lock-keywords-2
  (append
   shift-font-lock-keywords-1
   (list

    ;; Fontify storage classes and primitive types.
    (cons (concat "\\<" (shift-regexpify-list shift-storage-classes) "\\>")
	  'font-lock-type-face)
    (cons (concat "\\<" (shift-regexpify-list shift-primitive-types) "\\>")
	  'font-lock-type-face)
    
    ;; Fontify reserved words in font-lock-keyword-face
    (cons (concat "\\<" (shift-regexpify-list
			 (append shift-reserved-words shift-visibility-modifiers))
		  "\\>")
	  1))))


;; For code which does not use shift-font-lock-mode, below.
(setq shift-font-lock-keywords shift-font-lock-keywords-2)


(defun shift-font-lock-mode (&optional gaudy)
  "Turn on font lock mode in current buffer for SHIFT.
Argument should be one of these symbols, defaulting to 'gaudy:
    none: turn off font lock mode.
 minimal: highlight comments.
 subdued: highlight method names, too.
   gaudy: highlight reserved words, too."
  (interactive
   (list (let ((tab '(("none" . none)
		      ("minimal" . minimal)
		      ("subdued" . subdued)
		      ("gaudy" . gaudy))))
	   (cdr (assoc (completing-read "How much highlighting: " tab)
		       tab)))))
  (setq gaudy (or gaudy 'gaudy))

  (setq shift-font-lock-keywords
	(cond ((or (eq gaudy 'none)
		   (eq gaudy 'minimal))
	       nil)
	      ((or (eq gaudy 'subdued)
		   (eq gaudy t))
	       shift-font-lock-keywords-1)
	      ((eq gaudy 'gaudy)
	       shift-font-lock-keywords-2)))

  (cond ((eq gaudy 'none)
	 (font-lock-mode -1))

	((string-match "19.6 Lucid" (emacs-version))
	 ;; In WinEmacs this sets font-lock-keywords back to nil!
	 (font-lock-mode 1)
	 (setq font-lock-keywords shift-font-lock-keywords)
	 (font-lock-fontify-buffer))

	(t
	 (setq font-lock-keywords shift-font-lock-keywords)
	 (cond ((and (boundp 'font-lock-mode) font-lock-mode)
		(font-lock-fontify-buffer))
	       (t
		(font-lock-mode 1))))))

(provide 'shift-flock)

