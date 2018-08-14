# Hotel Management System v2.1 (2018-07-17)

## Hotel Terms
Please read carefully the following terms for customers before using the system.
1.	Customers may check-in, check-out or ask for increase at any time of a day.
2.	Customers may choose one of the following room type: single (one person), double (two people), family (three people) or dorm (multi people).
3.	When checking in, customers can book nights and order breakfast tickets, which can be increased then at any time before checking out.
4.	Customers must pay for all breakfast ordered as soon as the tickets are given.
5.	Customers must pay for rooms booked on a daily basis.
6.	How much a customer should pay for a dorm depends on how many roommates he has. The more people in a dorm, the cheaper for everyone.
7.	Dorms have a total price more expensive than a single room, but the system will assign rooms automatically so that your benefit will be maximized.
8.	Customers must checkout before the due, or he will be driven out at the end of the day.


## Introduction
The software is a hotel management system which contains a core library,
a command line interface with demo mode and an (uncompleted) graphic user interface.

The software takes charge of people check-in and check-out, room management and look-up services.

The software is written in C programming language.


## Compiling and Quick Start
### Compile on Linux
The software uses CMake to compile itself.
To compile using CMake, you can enter the following shell command:
```commandline
mkdir build
cd build
cmake ..
make
```

Alternatively, you can use the following building command:
```commandline
gcc -Wall -pedantic -Werror -Wno-unused-result -DNDEBUG -std=c11 -o hotel hotel-main.c hotel-mt.c hotel-db.c hotel-ll.c -lm
```

### Compile on Windows
On MS Windows we recommend you to use Clion with mingw64 toolchain to open the project and compile it.

## Project Binaries
There are four executables built in the project.
- hotel-main: The command line interface of the system with demo mode.
- hotel-app: The graphic user interface based on GTK+. Uncompleted.
- list-test: Runs several tests for the linked list data structure.
- set-test: Runs tests for the set abstract data structure.

The core functions of the system is compiled into a static library.

## Features
In the command line interface you can:
1.	Start a new hotel of four floors with four different types of rooms and specify their number.
2.	Handle check-in, check-out or increase events whenever a customer comes to the front desk.
3.	Information cards and receipts can be printed.
4.	Assign rooms for those who choose a dorm.
5.	Print all people and their information in the hotel.
6.	Update the database dynamically.

## Program Structure
The program is constructed in a hierarchy structure, namely the basic data structures, median interface and higher interface.
#### Basic data structures: hotel-ll
It defines operations on three useful containers: the list, the ordered set and the ordered map.

The list is implemented by linked list, which is used to store the rooms and people in a room and sometimes serves as stack.
The ordered set and map are implemented by AVL trees. For detailed operations on trees, I refer to [this document](./docs/AVL-Tree-Rotations.pdf) written by John Hargrove.
The set is used to store all people in the hotel and sort them in alphabetical order.
The map is used to sort dorms according to their rank, which will be covered.

All data structures are type-independent because they only store pointers of objects.

#### Median interface: hotel-db
It defines the structures of person, room and hotel. It provides functions that allow hotel to be loaded from a database file and complement all information implied.

#### Higher interface: hotel-mt
It provided functions that actually manage a hotel, such as 'find_available_rooms'. Users can build an application base on the higher interface efficiently.


## Implement details
1.	File parsing. When parsing the database, the program reads the whole file into memory and close it. Then the parser takes out fields line by line. When meeting a room, the program inserts a new room into the 'rooms' list. Then all the fields afterwards are added to attributes of this room. The order of the fields does not matter; neither do spaces beside the equal sign.
2.	Event evaluating. The 'hotel' structure maintains an event list. When something happens, an event with corresponding type and arguments will be added into the list. The function 'event_list_handle' will handle all events and clear the event list.
3.	Iterators. Sometimes the program needs to iterate a container, that is, do something for each elements in the container. I implement it by passing function pointers and their arguments to a container iterate function. Functions whose names contain 'iter' are iterator functions.
4.	Recursive algorithms. There are no recursive algorithms in the program other than freeing a tree. In the exercise I am asked to implement a recursive program to sort people. Before that requirement comes out I have made it by inserting people into a binary search tree. For the tree insertion, it is actually more easy to do recursively (iterative version needs stacks). Considering that many operations need set inserting and for the efficient' s sake, I decide to keep the iterative version. See 'node_insert' in 'hotel-ll.c'.
5.	Memory allocation. Memory management in this project is done under the following guide line: As soon as you are sure that you will NEVER use this chunk of memory, free it AT ONCE. This prevents memory leak, but also adds to risks of segmentation fault. However, after debugging, this could hardly happen now.


## Database
Database of the project is hotel.db under the data folder with the executable.
It consists of various of fields that describes attributes of the hotel and rooms.
```
today = 29/08/2017	# Current date
dorm = 180.00		# prices
single = 100.00
double = 180.00
family = 240.00
breakfast = 40.00

[room 101]
type = dorm
capacity = 4		# Capacity of a room rather than vacancy
name1 = Liberty Grabarczyk
id1 = 00000004
arrival1 = 27/08/2017
departure1 = 17/09/2017	# Departure rather than nights
keys1 = 1
breakfast1 = 15
payment1 = 690.00	# How much one has payed
# File truncated here
```

There are several differences between the database in this project and the given sample.
1.	On the header of file information of the hotel is stored.
2.	Store the capacity of the room rather can vacant. Vacant can be get by subtracting capacity with how much people are there in the room.
3.	Store the departure time instead of nights. Just for my convenience.
4.	Because the price of a dorm for each people varies from time, total price cannot be decided. Store how much the person has payed instead.
All these adjustments are done to suit the setting of the hotel.


## Demonstration Mode
The Demonstration Mode is in default unavailable.
To enable the demonstration mode, uncomment the following definition in [hotel-main.c](./hotel-main.c):
```objectivec
// #define HOTEL_DEMO      1
```
and recompile the program.

In the demo, the program generates people with random names trigging random events in a day.
You can press enter at the end of a day to enter a new day.

Note that people's ID are not randomly generated but are just their number of generation,
so before running the demo again please delete the database file,
otherwise it will cause a crash because there could be different people owning one ID and the program does not know who to checkout.


## Dorm Assignment
An important part of the program is to assign a dorm.

From the hotel's view, there is no difference between a dorm with one person or four. But the former one will pay even more than a single room.
When a customer comes, the program calculates the average difference of money paid with or without that person accumulated by time,
a.k.a. the long-term 'gain' and find the room which has the maximum gain.

See [hotel-mt.c](./hotel-mt.c) for more details.

## Contact
If you find bugs, have questions, or want to give me feedback, please send an email to
lepetitsquarth@sjtu.edu.cn

Zhang Zhenyuan