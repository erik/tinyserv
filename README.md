# tinyserv
tinyserv is a tiny no-configuration HTTP server that serves up the
contents of whatever directory is specified (the current directory, by
default).

tinyserv makes use of my C HTML generator,
[htmlize](https://github.com/boredomist/htmlize).

# building
There are no dependencies for building besides a C compiler, so run
`make clang` if you have clang, or `make gcc` if you have gcc.

# running
Running tinyserv without arguments will have it run on port 8080, and
serve content from the current working directory. To change the
directory, use the `--directory [dir]` (`-d [dir]` for short) command
line switch. Likewise, to change the port tinyserv uses, set the
`--port [port]` or `-p [port]` command line option.

# license
    Copyright (c) 2011 Erik Price
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
