securecat
=========

secure chat client written in C++ using ncurses.
This program was created to facilitate the secure communication between parties that are aware of a shared secret key.

Uses dyad.c for networking: https://github.com/rxi/dyad

rc4.cpp was ported from a RC4 implementation in a python script found online.

The client operates simply by encrypting and decrypting all outgoing and incoming data from the server, using the key the user inputs at the program start.

The server operates by relaying this encrypted data between the connected clients.
