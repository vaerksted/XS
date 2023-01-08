xs Tutorial
===========

`xs` is an extensible shell. Like other shells with which you're no
doubt familiar, `xs` can be used to run programs and scripts. But `xs`
is extensible. Parts of `xs` are written in `xs`. You can take advantage
of this by writing replacements for those parts to augment or modify
the behavior of `xs`. That's the "extensible" part.

Before we delve into `xs`'s extensibility, though, we need to take a
look at basic operations. Much of `xs` will seem familiar, but there are
enough differences from the shell you usually use to warrant a quick tour.

Perhaps the biggest difference between `xs` and traditional shells is that
`xs` has different rules for how data gets passed around in scripts that
you write. Traditional shells provide multiple ways to quote and evaluate
data; `xs` is much simpler. While you might think that it's better to
have more choices, you've probably questioned that wisdom when trying
to figure out how to deal with multiple levels of quoting and evaluation.

In `xs`, as in other shells, a word is formed from a sequence of
constituent characters. Unlike other shells, `xs` leaves many punctuation
characters available as word constituents. Therefore, these are all
valid words in `xs`:

```
  simple
  camelCase
  [bracketed]
  dotted.word
  1time
  *+,-./:=?@[]_~"
```

The final example is a word composed of all the non-special punctuation. I
don't know why you'd want to, but you *could* use that as a variable name:

```
; *+,-./:=?@[]_~" = 'I don''t even know...'
; echo $(*+,-./:=?@[]_~")
```

There are a few things to note in the preceding example:

    * The `xs` prompt is `;`. The man page explains this. Don't worry:
      you can change the prompt.
    * Words are quoted using `'`, which is escaped by doubling it.
      That's everything you need to know about `xs` quoting. Take a
      moment, if you will, to compare that to what you know about
      quoting in other shells.
    * The assignment operator `=` is always surrounded by spaces.
      Without the spaces, `=` becomes a word constituent. Indeed, this
      is the case in the `*+,-./:=?@[]_~"` word.
    * `"` is also a word constituent. This might seem odd at first, so
      I'll remind you: words in `xs` are quoted *only* using `'`.
    * As in other shells, you use `$` to dereference a variable. This
      example also needs to enclose the variable name in `(` and `)`.
      We could also have written `echo $'*+,-./:=?@[]_~?'`. See `xs(1)`
      for more information on constructing variable names.

In addition to quoting with `'`, you can use `\` to escape certain
characters. Here again, `xs`'s treatment differs somewhat from the
behavior you've come to expect from other shells. In `xs`, `\` is used
to escape the "special" characters:

```
# $ & ´ ( ) ; < > \ ^ ` { | } <space> <tab>
```

`xs` also recognizes escapes for a subset of characters that don't
have a printable glyph, i.e.:

    * \a <alert>
    * \b <backspace>
    * \e <escape>
    * \f <form-feed>
    * \n <newline>
    * \r <carriage return>
    * \t <tab>

as well as allowing you to spell out single-byte characters by their
code points in hexadecimal and octal notation:

    * \xNN where N is a hex digit
    * \MNN where M is {0..3} and N is an octal digit

You can also write Unicode characters (most of which are represented
by multiple bytes) using their hexadecimal code points:

    * \u'N...' where N... is one to six hex digits

`xs` does not permit creation of a NUL character using escapes. The
characters `\x00`, `\000` and `\u'0'` are all invalid.

It's an error in `xs` to use `\` except as noted above. In the shell
you usually use, applying `\` to escape some other character simply
yields the escaped character. In `xs`, this happens:

```
; echo \j
columns 0-0 bad backslash escape
```

Do you remember the rule for quoting with `'`? Everything inside
`'...'` is quoted; escapes don't get expanded. Thus:

```
; echo 'label:\tThis is unusual.'
label:\tThis is unusual.
```

In `xs` you'd write:

```
; echo label:\t'This is unusual.'
label:  This is unusual.
```
or:

```
; echo label:\tThis\ is\ unusual.
label:  This is unusual.
```

In most shells, the underlying data type is a string and all of the
shell's machinery is designed around doing useful manipulations of this
data. You probably understand well the headaches this can cause when a
word contains whitespace.

In `xs` the underlying data type is a list. Every word in a list
retains its identity as a word, regardless of whether the word contains
whitespace.

For example, this `xs` code assigns a four-element list to `a`:

```
; a = foo a\ word\ with\ spaces 'another word' bar
```

The words in the list are

    * foo
    * 'a word with spaces'
    * 'another word'
    * bar

You can pass the variable `a` to a function; it will remain a
four-element list despite the blanks that are part of the second and third
words. There's no need for special quoting and unquoting conventions as
in other shells.

You can select elements of a list using subscript notation. For example:

```
; echo $a(3)
another word
```

Subscripts start at one. It's an error to specify a subscript less than 1.

You may specify multiple indices:

```
; echo $a(1 4)
foo bar
```

In this example, the result is a two-element list.

You can use `...` in a subscript list to specify a range. For example:

```
; l = a b c d e f g h i j
; echo $l(4 ... 6)
d e f
```

Either end of the range may be open:

```
; echo $l(... 3)
a b c
; echo $l(7 ...)
g h i j
```

Reversing the indices of a range yields the specified elements in
reverse order:

```
; echo $l(6 ... 4)
f e d
```

Specifying an index greater than the number of list elements yields the
empty list:

```
; echo $l(11)

```

While we're on the subject, let's consider lists and empty lists. Lists
in `xs` are always one-dimensional; a list can't contain another
list. This makes an empty list "disappear" when it's contained within
a list. Returning to our subscript examples:

```
; m = $l(1 11 2 11 11 11)
; echo $m
a b
; echo $#m
2
```

Even though we specified six indices, four of these indexed past the
end of `$l` and yielded empty lists.

If you need an empty placeholder in a list, use an empty word:

```
; q = a '' b '' '' c
; echo $q
a  b   c
; echo $#q
6
```

Again, contrast the above with:

```
; r = a () b () () c
; echo $r
a b c
; echo $#r
3
```

Subscripts may not be used on the left-hand side of an assignment:

```
; l(3) = 99
columns 2-2 syntax error, unexpected SUB, expecting NL or ENDFILE
```

Here are two more things you can do with lists:

```
; echo $#l
10
; echo $^l
a b c d e f g h i j
```

$#<name\> returns the number of items in a list. $^<name\> returns a
one-element list composed of all the items in the original list,
separated with spaces. Let's make that a bit more clear:

```
; m = $^l
; echo $m
a b c d e f g h i j
; echo $#m
1
```

`xs` splits text into words delimited by a field separator; any character
not matching a field separator is part of a word; a character matching
a field separator ends the word and begins a new word with the next
non-separator character. In `xs` the field separators are determined by
the value of `$ifs`, which normally contains <space\>, <tab\> and <newline\>
(or, in `xs`: `\ \t\n`).

Now that we've introduced `$ifs`, we can turn our attention to the
backquote (`) operator. As in other shells, backquote captures the
output of a command or function. The syntax is slightly different than
what you've seen elsewhere:

```
; file_list = `ls
```

captures a list of file names in the current directory. You'll note that
there's no closing backquote as in other shells; you're probably wondering
how to capture the output of a command that has more than one word. In
`xs` we use a "program fragment":

```
; file_list = `{ls a*}
```

A program fragment is simply a group of commands wrapped in braces. A
program fragment may appear in an `xs` program anywhere you can use a
word. Thus the above example captures a list of all files beginning with
the letter "a".

Of course, because the default `$ifs` includes a space, any filename
containing spaces will be split into multiple words. That's normally
not what you'd like. As in other shells, the solution is to temporarily
replace the value of `$ifs` with just a newline. This would work:

```
; save_ifs = $ifs
; ifs = \n
; file_list = `ls
; ifs = $save_ifs
```

However, this can be simplified to:

```
; file_list = `` \n ls
```

Think of (``) as "backquote with temporary $ifs."

`xs` captures the status of a backquote command in the `$bqstatus`
variable. If the command is a pipeline, `$bqstatus` contains a return
code for each command in the pipeline.

True values in `xs` are `0`, `()` and `''`. Everything else is treated
as false.

A list evaluates true in `xs` only if all of the elements are true:

```
; if {result 0 () ''} {echo yes} else {echo no}
yes
; if {result 0 1 0} {echo yes} else {echo no}
no
```

When `xs` evaluates a list, it tries to take the first word as a
command. In the previous example, we used `result` as the command;
this simply returns its arguments.

At times it may be desirable to construct and execute a program
fragment. One such use might be to construct a Unicode character from
a code point computed at run time:

```
; echo `{{ |cp| eval echo '\u'''$cp''''} 01dd}
ǝ
```

Looking at the above example from the inside out, we're constructing
a word like `\u'N...'`. This is how `xs` names a Unicode code point. The
value of `$cp` is lambda-bound; that's the `|cp|` notation in the inner
program fragment. `eval` expects a string, but `\u'N...'` is a word. `echo`
does what it always does: it prints a string. `eval` parses and evaluates
the given text, producing the `ǝ` as a word. Now look at the two program
fragments. The inner fragment, the lambda expression, is in the command
position; `01dd` is its argument, which gets bound to `cp` in the lambda
list. The backquote says "run this command." Finally, the leftmost `echo`
prints the `ǝ` character.

A statement in `xs` is simply a command followed by the command's
arguments. A statement is terminated by any of:

    * a newline
    * a semicolon
    * the closing brace of a program fragment
    * a "special" character (see above)

Note that it's the most restrictive syntactic feature that determines
the end of a statement. Consider this code:


```
{ foo a b c; bar x y
  qux 17 39 }
```

The `foo` statement ends at `;`, the `bar` statement ends at the newline
and the `qux` statement ends at `}`.

The following are not equivalent:

```
{ bagley parsimony fletch
  grackle }
```

and

```
{ bagley parsimony fletch grackle }
```

The former is two separate statements; the latter only one. We can,
however, rewrite the first to be equivalent to the second by using
line continuation:

```
{ bagley parsimony fletch \
  grackle }
```

The backslash-newline sequence reads as a blank space.

Parentheses may be used to bound a list. A list so bounded may span
newlines. The following assignments are all equivalent:

```
l = a b c d e f
l = a b c \
    d e f
l = (a b c
     d e f)
```

Remember, too, that `xs` lists are flat (that is: always a list; never
a tree) and that empty lists "disappear" as a component of a list. The
following are equivalent:

```
m = (a b (c d (e) () f))
m = a b c d e f
m = ((((a b c d e f))))
```

Also, a list is not a program fragment, nor vice versa. Consider:

```
; (echo 1
   echo 2)
1 echo 2
; {echo 1
   echo 2}
1
2
```

The above example also illustrates how `xs`'s input reader handles a
continuation line. In both cases we typed an unfinished statement on
the first line; `xs` responded by printing its continuation prompt,
which is by default empty.

We've seen that a command may be a program (e.g. `ls`) or a lambda. A
lambda is just an unnamed `xs` function. We can also name `xs`
functions. The simplest case is to name a variable prefixed by `fn-`:

```
fn-ll = ls -l
```

You can also use the `fn` keyword to define a named `xs` function. This
is exactly equivalent to the previous example:

```
fn ll {ls -l}
```

A program not in a directory on `$PATH` may be used as a command by
naming an absolute or relative path to the program.

In the case of a relative path to the program, `xs` differs from many
other shells by requiring that the path begins with a `.` (dot). This
is because the word that names an `xs` function may legitimately contain
a path-separator character.

`xs` has all of the usual control-flow constructs, plus a few that you
don't usually see in shells.

The simplest control flow is a sequence. `xs` evaluates one statement
after another. Statements may be separated by a newline or a `;`.

Then there's conditional sequencing, provided by the familiar `&&` and
`||` operators. `&&` evaluates the statement to its right only if the
statement to its left has a true return code. `||` evaluates the statement
to its right only if the statement to its left has a false return code.

The `!` operator negates the return code of the statement to its right. As
in most programming languages, `!` has higher precedence than the other
boolean operators.

Like other shells, `xs` has wilcard matching, more commonly referred to
as "globbing". `*` and `?` match any sequence of characters (including
an empty sequence) or any individual character, respectively, in a
filename. `*` and `?` do not match `.` at the beginning of a filename,
nor do they match the `/` path separator.

Characters bracketed by `[` and `]` form a class that matches any one
character between the brackets. A negative class, i.e. a class that
matches any character *not* listed, is introduced by a `~` (*not* `^`
as in other shells) immediately following the open bracket.

Remember that there are no special characters within `'...'` in xs.

For the case where you'd like to match something other other than
a filename, `xs` provides a match operator. The syntax of the match
operator is:

```
~ <subject> <pattern>...
```

where <subject\> is typically a variable, a backquote expression or other
statement that produces a string value. The <subject\> is followed by one
or more patterns. `xs` does not match patterns against filenames in this
case; there's no need to (nor should you) quote the <pattern\>s. (However,
a literal wilcard in the <subject\> *is* expanded as described in the
previous paragraphs.)

You can specify the empty list as a pattern. But remember how `xs`
collapses empty lists within a list: you can't match *either* a pattern
or an empty list. This expression, for example, matches *only* `f?b`:

```
~ $v f?b ()
```

The pattern match operator returns true if <subject\> matches any of
the <pattern\>s. In the case where <subject\> is a list, we obtain a true
result if any of the <subject\>s match any of the <pattern\>s. All in all,
`~` is a pretty powerful tool. It's also fast. In particular, think of
`~` rather than a relational operator when you need to match a specific
numeric or string value.

There's also `~~`, the pattern extraction operator, used like this:

```
~~ <subject> <pattern>...
```

The <subject\> is matched against <pattern\>s; only the portions of
<subject\> that match wilcards in <pattern\>s are returned as the value of
`~~`. When <subject\> is a list, the result is the concatenation of all
<pattern\> matches for the first list item, then the second, and so on.

Globbing is handy, but it can't handle the cases where you'd like to
match specific substrings in file and directory names. This is where `xs`
lists come in handy when combined with the concatenation operator, `^`.

This command:

```
ls (foo bar baz qux)^*.abc

```

lists all files having names matching `foo*.abc`, `bar*.abc`, `baz*.abc`
and `qux*.abc`.

Of course, you can combine list concatenation with other forms of
globbing, such as:

```
vi *^(mem wan)^.[ch]
```

You can even concatenate multiple lists using `^`; `xs` will generate the
cross product of the lists. This:

```
mv (base config alt)^-^(session client wrapper)^.^(lisp fasl) archive/
```

moves all of the following files from the current directory to the
`archive/` subdirectory:

```
base-session.lisp
base-session.fasl
base-client.lisp
base-client.fasl
base-wrapper.lisp
base-wrapper.fasl
config-session.lisp
config-session.fasl
config-client.lisp
config-client.fasl
config-wrapper.lisp
config-wrapper.fasl
alt-session.lisp
alt-session.fasl
alt-client.lisp
alt-client.fasl
alt-wrapper.lisp
alt-wrapper.fasl
```

Of course, if all you need is to join two lists into a longer list,
without the cross-product operation, simply adjoin the lists:

```
; l = hello world
; m = nice clouds
; n = $l $m
; echo $n
hello world nice clouds
; echo $l^$m
```

Contrast that with:

```
; echo $l^$m
hellonice helloclouds worldnice worldclouds
```

In addition to the boolean control-flow operators we've already
encountered, `xs` provides a number of familiar constructs. Starting
with the conditionals, we have:

```
if <test> <consequent> [else <alternative>]
switch <value> <case-and-action>... [<default-action>]
```

Unlike many other languages, `xs` ends a statement at a newline. This
means that these statements must either be written on one line, or
extended to subsequent lines using a backslash continuation. You may also
satisfy `xs`'s all-one-line requirement by using a program fragment to
span lines, such as:

```
if {~ $TERM linux} {
    echo 'I''m in the console.'
} else {
    echo 'Maybe I''m on a pty...'
}
```

For the `switch` statement, usual practice is to enclose all of the case
and actions and the default action within a list, like this:

```
switch $x (
    1 {echo one}
    2 {echo two}
    {echo many})
```

`xs` provides these iteration constructs:

```
for <var-and-list>... <command>
until <test> <body>
while <test> <body>
```

In the simplest case, the `for` loop looks quite familiar:

```
for i `{seq 5} {printf 'i is %d'\n $i}
```

But you can provide multiple iteration variables:

```
for i `{seq 5}; j (a b c) {printf '%d is %s'\n $i $j}
```

The `for` loop continues to iterate until the *longest* list has been
exhausted, substituting an empty list value for each list that has run
out of elements.

The `while` and `until` loops complement each other: `while` executes
the body until the test is true; `until` runs the body while the test
is false.

The next couple of control constructs have to do with altering the
interpreter's behavior.

```
eval <list>
result <arg>...
```

`eval` turn the given list into a string, separating the words with
blanks, and then feeds that string to the interpreter. The interpreter
processes the string as `xs` statements.

`result` is an identity function; it returns whatever it's given. If
this seems useless, consider the following. Let's say your program sets
a variable that you want to use elsewhere to condition the execution of
a brief statement. Your first thought may be to write this:

```
if $cond do_something
```

The above doesn't work because `if` expects a statement, not a variable,
for its condition. We use `result` to return the value of `$cond` in
this corrected code:

```
if {result $cond} do_something
```

You might also try to write the expression-oriented form like this:

```
$cond && do_something
```

The above won't work. Recall that `xs` tries to evaluate the first word
of a statement as a command or a function, of which a boolean value is
neither. The solution is to use the built-in function `result`:

```
result $cond && do_something
```

While on the subject of booleans, note that `xs` provides functions that
evaluate to boolean values:

```
false
true
```

The next group of `xs` functions deals with process manipulation. These
have the same meaning as in other shells.

```
exec <command>
fork <command>
wait [<pid>]
```

The variable `$apid` is the PID of the process most recently forked,
while `$apids` is a list of processes for which `xs` has not yet
`wait`ed. Please consult `xs(1)` for lower-level functions that manipulate
processes.

The `exit` command causes `xs` to terminate and return a specific status
to its caller. If no status is given, `xs` returns zero. This is unlike
some other shells that use the status of the last-executed command.

```
exit [<status>]
```

Most shells provide a mechanism to execute a command upon receipt of
a signal. `xs` does as well; additionally allowing for different handlers
to be present at different places in the call stack.

```
signals-case <body> <handlers-alist>
```

The `signals-case` command executes its body with signal handlers bound
according to a list of `<signal-name> <fragment>` pairs. The `<signal-name>`
is from `signals.h`, but spelled using all lowercase letters. The `<fragment>`
executes when its matching signal arrives during execution of `signals-case`.

An `xs` script can use the `raise` command to raise one of its own
`signals-case` handlers.

```
raise <signal>
```

The `<signal>` argument is either a Linux signal name as defined above or
another name appearing in the `<handlers-alist>` of an active `signals-case`
command.

In addition to the signal handlers, `xs` provides an exception framework
reminiscent of higher-level languages.

```
catch <catcher> <body>
throw <exception> <arg>...
```

The `catch` function establishes a catcher for the code in its body. If
the body signals an exception, or if a signal is received, the catcher
receives the exception as an argument. You'd use a lambda to bind the
exception for use by the catcher:

```
catch {|e v| printf 'exception was %s %s'\n $e $v} {exit 99}

```

Yes, `exit` is an exception. Here's a list of all the built-in exceptions:

```
eof
error <source> <message>
exit <status>
signal <name>
```

(Note: `catch` does not handle the `signal` exception, but the primitive
`$&catch` does. Take a look at the definitions of `catch` and `signals-case`.)

You can `throw` any of these exceptions. You can also define your own:

```
catch {|e v| printf 'caught %s %s'\n} {throw foo 'my exception'}
```

If you need to, you can throw an exception from within the catcher;
the next handler up the chain (i.e. an active catch established earlier)
will receive the newly-thrown exception.

```
retry
```

Within a catcher, the `retry` exception is treated specially: it causes
the body to be run again. This can be useful for a structured retry
of a failed operation. Beware, though: if the body has not cleared the
original condition that caused the original exception, throwing `retry`
from the catcher will cause an uninterruptible loop.

Since we've already mentioned lambdas a few times, let's quickly review
the finer points.

```
{|<lambda-list>| <body>}
```

The <lambda-list/> is just a list of names that are bound to arguments to
the lambda. For example:

```
{|a b c| echo $a $b $c} 1 2 3
```

prints `1 2 3`. If there are more names than arguments, the additional
names are bound to `()`. If there are more arguments than names, the
final name is bound to all of the remaining arguments.

There are a few more control-flow constructs to cover.

```
escape <lambda>
forever <command>
unwind-protect <body> <cleanup>
```

Non-local exits may be defined by `escape`. This allows your program
to exit from multiple levels of lexically nested statements. Here's
an example:

```
fn count {|n|
  {escape { |fn-return|
    i = 0
    while true {
      i = `($i+1)
      echo $i
      if {~ $i 5} {return}
    }
  }}
}
```

Note that `xs` doesn't have a built-in `return` function like other
shells. The `escape` mechanism gives you more control at the expense of
slightly more verbose code. Also, the name is not important: you could
replace `return` with any other unused name.

`forever` is a loop from which exceptions and signals cannot escape.
This finds use in the read-eval-print loop of an interactive `xs` shell.

`unwind-protect` assures that cleanup code runs regardless of the success
or failure of code in the body.

The `map` and `omap` functions apply a function or command to each
element of a list.

```
map <action> <list>
omap <action> <list>
```

`map` returns a list of the action's results, while `omap` returns a
list of the action's outputs.

Controlling the visibility of names can be useful in a large shell
program. When you use a variable in one part of the program, you'd like
some assurance that you won't clobber its value in another unrelated
piece of code. Other shells may provide a `local` keyword to use within
function declarations. `xs` lets you declare local names anywhere.

```
let (<binding>...) <body>
```

A binding is an assignment, except that any name so bound is visible
only within the body. Bindings are separated by newline or `;`. The
assignment may be elided; this is equivalent to writing `<name> =`,
which binds the name to `()`. All bindings are done at the same time;
you can't have a later binding depend upon an earlier binding in the
same `let`.

Other shells usually collect all manner of tests under one builtin
that handles file accessibility and relational operators under a single
function. `xs` separates these tests. We've already encountered
the match operator, `~`. The built-in function `access` tests given
filesystem paths against various criteria:

```
access [-n <name>] [-1|-e] [-r|-w|-x] [-f|-d|-c|-b|-l|-s|-p] <path>...
```

The `rwx` flags test accessibility. The `fdcblsp` flags test the file's
type. The accessibility flags may be combined. `access` uses only the
rightmost type flag.

Normally, `access` returns a list of boolean results, one for each
<path\>. Rather than return a canonical `false` value where appropriate,
`xs` returns the text of the error code that was obtained by trying
to access <path\> according to the given criteria. That text is still
logically false, but it provides additional information that you may
choose to use.

When we provide the `-1` flag, `access` stops at the first <path\> that
meets the criteria and returns not a boolean, but the path itself. If
no <path\> meets the criteria, `access` returns `()`. When `-e` is used
together with `-1`, `access` signals an error if no <path\> meets the
given criteria.

With the `-n <name>` option, the <path\>s are treated as a list of
directories and <name\> is taken as the name of a file to be found in
those directories. Otherwise, all other behaviors are the same with the
`-n` option. In other words, `access -n <name> <criteria> <path>...`
finds <name\>d file(s) in <path\>s that meet the specified criteria. The
`-1` and `-e` options may also be given, with their usual effects.

Finally, `xs` has the usual relational operators. These are named `:lt`,
`:le`, `:gt`, `:ge`, `:eq` and `:ne` in order to avoid confusion with
other `xs` usage of the usual tokens.

Relational comparisons coerce both values to floats if either
is a float, and both values to strings if either is not numeric. String
comparisons are performed using the current locale's collating order.

The relational operators expect a pair of singleton values. The result of
using list arguments is undefined, except that `()` may be compared using
`:eq` or `:ne` (although the `~` operator is preferred in this case).

`xs` executes commands in one or two startup files, depending upon how
`xs` is invoked.

As a startup shell (i.e. as started by the OS as a result of just having
logged in) or when invoked with the `-l` flag, `xs` is a login shell. When
invoked as a login shell, `xs` reads `~/.xsrc`.

As an interactive shell, `xs` reads `~/.xsin`. `xs` is interactive as
a login shell, when invoked from the command line (i.e. with a terminal
attached to standard input) and when invoked with the `-i` flag.

If `xs` reads both `~/.xsrc` and `~/.xsin`, it always does so in that
order.

This concludes our brief introduction to the `xs` shell. You'll find
much more information in `xs(1)` and in various files installed to
`xs`'s documentation directory.

If you're interested in extending or modifying `xs`'s behavior, take
a look at `xs(1)`'s explanation of hooks, primitives and syntactic
rewrites. The `initial.xs` file (in the source repository) defines a
significant portion of `xs` using `xs` code and contains good examples
of some of the more complex `xs` features.

The source code repository and issue tracker is at:

```
https://github.com/TieDyedDevil/XS
```
