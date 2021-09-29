# smallsh
## (Small Shell)

This is my portfolio assignment for **CS 344: Operating Systems I**

This mini shell program for Linux is intended to demonstrate competency with certain operating system concepts including:

- Signals, signal handling, and interrupts
- Process creation, management, and control
- I/O and redirection

This shell only handles certain commands natively by design (see source code for more details). Any command provided to the shell that is not handled explicitly by the shell is executed by forking the process and using `exec [command]`. This is not designed to be a full-featured shell; it is a demonstration of a narrow set of concepts.

To compile this program, simply extract all files to a directory and run:

	make all

This will create an executable called "smallsh".
