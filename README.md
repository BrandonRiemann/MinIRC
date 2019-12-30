# MinIRC
 Minimalistic IRC client using ncurses written in C. (One of my first major C programs. Still WIP.)

![](https://i.gyazo.com/781b1b70c41df7652e1261ec841755a3.gif)

## What is it?
MinIRC is a basic prototype (i.e., work in progress) of a minimalistic IRC client using the ncurses library designed for POSIX-compatible operating systems. The goal of the project was to design a simple interface and gain familiarity with the RFC 1459 protocol (https://tools.ietf.org/html/rfc1459) which IRC uses. Many websites that offer chat features often have an IRC server allowing the user to connect to the chat from a program such as this. This offers flexibility in how the chat client is implemented, such as UI design, keyboard controls, and other additional functionality that the developer might wish to add.

## Future goals
If development is continued, some possible features to be added would include:
* Different colors for different users, primarily to distinguish between the messages
* Different color schemes for the UI
* Multiple "tabs" to allow several connected sessions simultaneously
* Nick highlighting

## Compiling
I use the following to compile the source code: gcc [.c files] -lncurses -o [output file]
 
## Usage
For now, the program can be run with the following arguments: minirc [host] [port] [nick] [pass] [channel]
