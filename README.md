XS: The eXtensible Shell
========================

**NOTICE: As of February 2021 I will no longer maintain the XS project. If you care to assume ownership of this repo, please open an issue.**

This is `xs 1.3`; see `doc/CHANGES`.

XS is the eXtensible Shell, a command line interpreter featuring
Lisp-like semantics (lists, function parameters, closures, exceptions,
lexical bindings, lambdas, etc.) and a conventional syntax.

The package includes extensive documentation. Sample scripts to define
library functions, tools and utilities are found in
[https://github.com/TieDyedDevil/XS-library.git](https://github.com/TieDyedDevil/XS-library.git)  .

Use meson-0.54.x or meson-0.56.x to build XS.

```
$ ~/xs_env/bin/meson --version
0.56.2
$ export PATH=~/xs_env/bin:${PATH}
$ cd /tmp
$ git clone -q https://github.com/vaerksted/XS.git
$ cd XS
$ ./buildscan.sh
scan-build: Using '/usr/bin/clang-13' for static analysis
Using 'PKG_CONFIG_PATH' from environment with value: '/usr/local/lib64/pkgconfig:/usr/local/share/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig'
Using 'PKG_CONFIG_PATH' from environment with value: '/usr/local/lib64/pkgconfig:/usr/local/share/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig'
The Meson build system
Version: 0.56.2
Source dir: /tmp/XS
Build dir: /tmp/XS/buildscan
Build type: native build
Project name: xs
Project version: undefined
Using 'CXX' from environment with value: '/usr/bin/../libexec/c++-analyzer'
C++ compiler for the host machine: /usr/bin/../libexec/c++-analyzer (gcc 11.2.0 "g++ (GCC) 11.2.0")
C++ linker for the host machine: /usr/bin/../libexec/c++-analyzer ld.bfd 2.37-slack15
Using 'CXX' from environment with value: '/usr/bin/../libexec/c++-analyzer'
Host machine cpu family: x86_64
Host machine cpu: x86_64
Found pkg-config: /usr/bin/pkg-config (0.29.2)
Run-time dependency Boost found: YES 1.77.0 (/usr)
Found CMake: /usr/bin/cmake (3.21.4)
Run-time dependency ffi found: NO (tried pkgconfig and cmake)
Library ffi found: YES
Run-time dependency gc found: NO (tried pkgconfig and cmake)
Library gc found: YES
Library gccpp found: YES
Using 'PKG_CONFIG_PATH' from environment with value: '/usr/local/lib64/pkgconfig:/usr/local/share/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig'
Run-time dependency readline found: YES 8.1
Build targets in project: 12

Found ninja-1.10.2 at /usr/bin/ninja
scan-build: Analysis run complete.
scan-build: Removing directory '/tmp/scan-build-2021-11-28-113036-4216-1' because it contains no reports.
scan-build: No bugs found.
scan-build: Using '/usr/bin/clang-13' for static analysis
[14/82] Compiling C++ object xsdump.p/src_eval.cxx.o
../src/eval.cxx:170:9: warning: Access to field 'defn' results in a dereference of a null pointer (loaded from variable 'sequence') [core.NullDereference]
                                if (sequence->defn != NULL) {
                                    ^~~~~~~~~~~~~~
1 warning generated.
[22/82] Compiling C++ object xsdump.p/src_input.cxx.o
../src/input.cxx:314:2: warning: Potential leak of memory pointed to by 's' [unix.Malloc]
        return text;
        ^~~~~~~~~~~
../src/input.cxx:314:2: warning: Potential leak of memory pointed to by 's' [unix.Malloc]
        return text;
        ^~~~~~~~~~~
../src/input.cxx:446:29: warning: Array access (from variable 'results') results in a null pointer dereference [core.NullDereference]
                results[results_size - 1] = NULL;
                ~~~~~~~                   ^
3 warnings generated.
[27/82] Compiling C++ object xsdump.p/src_prim-io.cxx.o
../src/prim-io.cxx:178:7: warning: Although the value stored to 'pid' is used in the enclosing expression, the value is never actually read from 'pid' [deadcode.DeadStores]
        if ((pid = pipefork(p, NULL)) == 0) {   /* child that writes to pipe */
             ^     ~~~~~~~~~~~~~~~~~
../src/prim-io.cxx:251:13: warning: Array access (from variable 'pids') results in a null pointer dereference [core.NullDereference]
                pids[n++] = pid;
                ~~~~      ^
2 warnings generated.
[39/82] Compiling C++ object xsdump.p/src_token.cxx.o
../src/token.cxx:139:10: warning: Although the value stored to 'c' is used in the enclosing expression, the value is never actually read from 'c' [deadcode.DeadStores]
        switch (c += '0') {
                ^    ~~~
1 warning generated.
[45/82] Compiling C++ object xsdump.p/src_var.cxx.o
../src/var.cxx:244:10: warning: Although the value stored to 'varlist' is used in the enclosing expression, the value is never actually read from 'varlist' [deadcode.DeadStores]
        return (varlist = sortlist(varlist));
                ^         ~~~~~~~~~~~~~~~~~
1 warning generated.
[51/82] Compiling C++ object xs.p/src_eval.cxx.o
../src/eval.cxx:170:9: warning: Access to field 'defn' results in a dereference of a null pointer (loaded from variable 'sequence') [core.NullDereference]
                                if (sequence->defn != NULL) {
                                    ^~~~~~~~~~~~~~
1 warning generated.
[60/82] Compiling C++ object xs.p/src_input.cxx.o
../src/input.cxx:314:2: warning: Potential leak of memory pointed to by 's' [unix.Malloc]
        return text;
        ^~~~~~~~~~~
../src/input.cxx:314:2: warning: Potential leak of memory pointed to by 's' [unix.Malloc]
        return text;
        ^~~~~~~~~~~
../src/input.cxx:446:29: warning: Array access (from variable 'results') results in a null pointer dereference [core.NullDereference]
                results[results_size - 1] = NULL;
                ~~~~~~~                   ^
3 warnings generated.
[65/82] Compiling C++ object xs.p/src_prim-io.cxx.o
../src/prim-io.cxx:178:7: warning: Although the value stored to 'pid' is used in the enclosing expression, the value is never actually read from 'pid' [deadcode.DeadStores]
        if ((pid = pipefork(p, NULL)) == 0) {   /* child that writes to pipe */
             ^     ~~~~~~~~~~~~~~~~~
../src/prim-io.cxx:251:13: warning: Array access (from variable 'pids') results in a null pointer dereference [core.NullDereference]
                pids[n++] = pid;
                ~~~~      ^
2 warnings generated.
[77/82] Compiling C++ object xs.p/src_token.cxx.o
../src/token.cxx:139:10: warning: Although the value stored to 'c' is used in the enclosing expression, the value is never actually read from 'c' [deadcode.DeadStores]
        switch (c += '0') {
                ^    ~~~
1 warning generated.
[81/82] Compiling C++ object xs.p/src_var.cxx.o
../src/var.cxx:244:10: warning: Although the value stored to 'varlist' is used in the enclosing expression, the value is never actually read from 'varlist' [deadcode.DeadStores]
        return (varlist = sortlist(varlist));
                ^         ~~~~~~~~~~~~~~~~~
1 warning generated.
[82/82] Linking target xs
scan-build: Analysis run complete.
scan-build: 16 bugs found.
scan-build: Run 'scan-view /tmp/scan-build-2021-11-28-113042-4369-1' to examine bug reports.
scan-build: Viewing analysis results in '/tmp/scan-build-2021-11-28-113042-4369-1' using scan-view.
/usr/bin/scan-view:9: DeprecationWarning: the imp module is deprecated in favour of importlib; see the module's documentation for alternative uses
  import imp
Starting scan-view at: http://127.0.0.1:8181
  Use Ctrl-C to exit.

^C$
```
