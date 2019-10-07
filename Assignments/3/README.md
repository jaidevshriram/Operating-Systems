# jeez shell

A shell that took some time and effort but was worth it in the end :)

## Setup and Running

1. Clone this repository
2. `cd` into this repo and type `make`
3. A new file called 'shell' shall be created. Use `./shell` to run the new shell from your terminal.

## What does what

##### main.c

The driver code initializes the shell, updates history, tokenizes input, sets up command chain of input when there are semicolin seperated inputs and prompts the user for commands. This links all the other files together.

##### changedir.c

This has a function that changes directories if it possible. This calls a helper function called translate home to translate path that include the ~ character

##### echo.c

This displays the argument on screen.

##### helper.c

One of the most important files in this project, it contains a set of helper functions which makes it easier for commands to work.
THis includes for example, counting the number of parameters, removing additional whitespace, updating prompt values and more.

##### history.c

BONUS: Added history command to view at max previous 20 commands. These work cross terminal&session.

##### list.c

Executes the ls command and the -a -l flags. Uses stat and opendir to view files.

##### pinfo.c

Displays certain properties as done in a regular pinfo command

##### pwd.c

Just prints the current working directory

##### system.c

This is responsible for all command execution that is not built in. Deals with foreground and background handling as well.

##### colours.c

Just to make the terminal pretty. Has functions to change color of stdout

##### cron.c

Executes cron job by calling the function start_command_execution in the background

##### env.c

Sets and unsets the environment variables.

##### jobs.c

Responsible for maintaining the list of background processes. Also contains the fg, bg commands.

