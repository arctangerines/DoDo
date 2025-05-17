# DoDo

ToDo manager

## Usage

`dodo` `[-n{d}]` `[-l]` `file`

* `-n{d}` is the line padding, this being the amount of lines to show before
  and after the ToDo comment, `d` is the actual number,
    * You'd use it like this -n4
* `-l` flag is to pipe the output to less
* Flags have to go before filename :)

## Features

* List TODOs in a file
* Formatted output
* Specifying the amount of lines to show before/after the todo line
* Pipe to less

# Short term goals

* List TODOs in a folder
* Add the option to show a bit more of the code around the comments
* Would be cool to add an option to print some lines before/after
* Dump TODOs (same as list?)

# Long term goals

* Implement config file parser
