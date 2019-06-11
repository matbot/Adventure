/**
 * Mathew McDade
 * CS344 Spring 2019
 * Build the Adventure!
 * This program creates a set of seven random rooms each with random connections to other rooms.
 * A directory is created using the programs pid and a file is written in that directory for
 * each of the created rooms.
 */
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*Define constants for program extensibility.*/
#define ROOM_COUNT 7
#define MIN_CONNECT 3
#define MAX_CONNECT 6

/*A custom bool type to get something like C++ bool functionality. Very useful.*/
typedef enum { false, true } bool;

/*Room struct to hold the required room details. Element types chosen to be easily passed to
 * functions. */
struct Room
{
	char* name;
	char* type;
	int numOutboundConnections;
	struct Room* OutboundConnections[6];
};

/*Accessory swap function for Fisher-Yates shuffle in function setRoomNames--should be extended to work 
 * with void cast types and integrated primary function with indeterminate argument sizes on 
 * program upgrades.															!!
*/
void swap(int arr[], int i, int j) {
	int temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}

/*Shuffles a set of 10 static potential room names, selects 7 of them, and assigns those names to a
 * room. You heard about Pluto? That's messed up.
 */
void setRoomNames(struct Room* Rooms) {
	char* names[10];
	names[0] = "Sun";		/*The hottest planet.*/
	names[1] = "Mercury";
	names[2] = "Venus";
	names[3] = "Earth";
	names[4] = "Mars";
	names[5] = "Jupiter";
	names[6] = "Saturn";
	names[7] = "Uranus";
	names[8] = "Neptune";
	names[9] = "Pluto";
	
	/*Fisher-Yates shuffle a simple int array to randomize name indices.
	//Adapted from psuedocode of Durstenfeld's algorithm:
	//https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
	//Should be moved into its own function..										!!
	*/
	int indices[] = { 0,1,2,3,4,5,6,7,8,9 };
	int i;
	for(i=0;i<=7;i++) {
		int j = i + rand() % (10-i);
		swap(indices,i,j);
	}
	
	/*Take the first seven room names from the pseudorandom permutation, itself therefore a random
	//set, and assign them to Rooms.
	*/
	for(i=0;i<ROOM_COUNT;i++) {
		Rooms[i].name = names[indices[i]];
	}
	return;
}

/*Sets the room types randomly by starting every room as a MID_ROOM, then selecting two random
 * rooms to be the START_ROOM and END_ROOM.
 */
void setRoomTypes(struct Room* Rooms) {
	/*Set all room types to MID_ROOM.*/
	int i;
	for(i=0;i<ROOM_COUNT;i++) {
		Rooms[i].type = "MID_ROOM";
	}

	/*Select random start and end room indices.*/
	int startIndex = rand() % 7;
	int exitIndex = startIndex;
	/*Loop to ensure start and end indices are not the same.*/
	while(exitIndex==startIndex) {
		exitIndex = rand() % 7;
	}
	/*Set the start and end room types.*/
	Rooms[startIndex].type = "START_ROOM";
	Rooms[exitIndex].type = "END_ROOM";
	return;
}

/*A function to package the two functions above, which actually do the work. */
void initializeRooms(struct Room* Rooms) {
	setRoomNames(Rooms);
	setRoomTypes(Rooms);
	return;
}

/*Checks if each Room in the graph, Rooms, has the minimum number of outbound connections as
//defined by MIN_CONNECT, and returns the nifty bool.
*/
bool IsGraphFull(struct Room* Rooms) {
	int i;
	for(i=0;i<ROOM_COUNT;i++) {
		if(Rooms[i].numOutboundConnections < MIN_CONNECT) {
			return false;
		}
	}
	return true;
}

/*Checks if room x is in fact room y, in disguise. bool */
bool IsSameRoom(struct Room* x, struct Room* y) {
	if(strcmp(x->name,y->name)==0) {
		return true;
	}
	else {
		return false;
	}
}

/*Checks if there is already a connection between room x and y. Here, it is sufficient to loop
 * through just one of the rooms existing connections because if x is connected to y, then y is
 * also connected to x by design.
 * */
bool ConnectionAlreadyExists(struct Room* x, struct Room* y) {
	int i;
	for(i=0;i<x->numOutboundConnections;i++) {
		if(strcmp(x->OutboundConnections[i]->name,y->name)==0) {
			return true;
		}
	}
	return false;
}

/*Just checking if the room has capacity for adding another connection.*/
bool CanAddConnectionFrom(struct Room* x) {
	if(x->numOutboundConnections < MAX_CONNECT) {
		return true;
	}
	else {
		return false;
	}
}

/*Uses a random index to pick a random room and returns that room's address.*/
struct Room* GetRandomRoom(struct Room* Rooms) {
	int index = rand() % ROOM_COUNT;
	return &Rooms[index];
}

/*Adds a room connection. Note this only adds the connection in one direction.*/
void Connect(struct Room* x, struct Room* y) {
	x->OutboundConnections[x->numOutboundConnections] = y;
	x->numOutboundConnections++;
}

/*Adapted from program assignment pseudocode. Putting all those bool functions to use.*/
void AddRandomConnection(struct Room* Rooms) {
	/*declare some rooms A and B.*/
	struct Room* A;
	struct Room* B;
	
	do { A = GetRandomRoom(Rooms);
	} while(!CanAddConnectionFrom(A));

	do { B = GetRandomRoom(Rooms);
	} while(!CanAddConnectionFrom(B) || IsSameRoom(A,B) || ConnectionAlreadyExists(A,B));

	Connect(A,B);
	Connect(B,A);
}

/*Adds random connections with respect to requirements until the graph is full.*/
void connectRooms(struct Room* Rooms) {
	while (IsGraphFull(Rooms)==false) {
		AddRandomConnection(Rooms);
	}
	return;
}

/*Creates a new directory to put room files in.*/
void createDir(char* dir) {
	sprintf(dir,"mcdadem.rooms.%d",getpid());
	mkdir(dir,0755);
	return;
}

/*Writes a room to a file in the directory created in createDir(). Files are named the same thing as
 * the room they describe.
 */
void writeRoom(struct Room* Room) {
	FILE* ofile;
	ofile = fopen(Room->name,"w");

	fprintf(ofile,"ROOM NAME: %s\n", Room->name);
	int i;
	for(i=0;i<Room->numOutboundConnections;i++) {
		fprintf(ofile,"CONNECTION %d: %s\n",(i+1),Room->OutboundConnections[i]->name);
	}
	fprintf(ofile,"ROOM TYPE: %s\n",Room->type);

	fclose(ofile);
	return;
}

/*Organizes the directory creation and room writing functions.*/
void writeRooms(struct Room* Rooms) {
	char dir[50];
	createDir(dir);
	chdir(dir);
	int i;
	for(i=0;i<ROOM_COUNT;i++) {
		writeRoom(&Rooms[i]);
	}
	return;
}

/* Main: aims to make the process of struct creation, random connections, and file writing clear 
 * through function encapsulation.
 */
int main() {
	/*Begin by seeding rand globally for all function calls.*/
	srand(time(NULL));

	/*Declare an array of seven Room structs and allocate memory.*/
	struct Room* Rooms = calloc(7, sizeof(struct Room));

	/*Initialize each room with an id, room type, and random name.*/
	initializeRooms(Rooms);

	/*Create all random room connections between rooms.*/
	connectRooms(Rooms);
	
	/*Write the information about each room to a file in the appropriate format.*/
	writeRooms(Rooms);

	/*Free the allocated memory.*/
	free(Rooms);

	return 0;
}
