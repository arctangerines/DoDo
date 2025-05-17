# DoDo

ToDo manager

## Usage

`dodo` `[-n{d}]` `[-l]` `file[s]`

* `-n{d}` is the line padding, this being the amount of lines to show before
  and after the ToDo comment, `d` is the actual number,
    * You'd use it like this -n4
* `-l` flag is to pipe the output to less
* Flags have to go before filename[s] :)

## Features

* List TODOs in a file
* List TODOs of multiple files
* Formatted output
* Specifying the amount of lines to show before/after the todo line
* Pipe to less

# Short term goals

* Dump TODOs (same as list?)

# Long term goals

* Implement config file parser
