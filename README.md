# Client-Server-using-IPC

This is a C++ assignment that mimics client server interaction.

This is a client program that connects to a given server. The server defines a communication protocol, which the client has to implement by sending properly formulated messages over a communication pipe. The server hosts several electrocardiogram (ECG) data points of 15 patients suffering from various cardiac diseases. The client’s goal is to obtain these data points by sending properly-formatted messages that the server understands. In addition, the server can send raw files potentially in several segments.
Obtained the above dataset from physionet.org. 

This project was part of my operating systems class, and was recently moved to my public repo.

## Background Information

## Flag Description

* p flag is the patient number
* t flag is for time
* e flag is for the ecg number (1 or 2)
* m flag is the buffer capacity
* f flag is for file transger
* c flag is for channel creation

## Request Data Point Example Command
* $ ./client -p <patient no> -t <time in seconds> -e <ecg no> 
* $ ./client -p 10 -t 59.00 -e 2 

## File Transfer Example Command
* $ ./client -f <file name> 
* $ ./client -f 10.csv

## Change Buffer Capacity Example Command
$ ./client -m 5000 

## Create New FifoRequest Channel Example Command
$ ./client -c 

Type quit to end the program


