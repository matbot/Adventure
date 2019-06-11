/**
 * Mathew McDade
 * CS344 Spring 2019
 * Be the Adventurer!
 * Loads rooms and connections from file and plays the game.
 */
#include <dirent.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*Define constants for program extensibility.*/
#define ROOM_COUNT 7

/*The mighty bool.*/
typedef enum { false, true } bool;

/*This Room struct is a little different from the one in buildrooms.c. Most notably, the outbound
 * connections are stored by name as opposed to as a pointer to another room struct, which is
 * sufficient and simplifies loading the structs from the file format.
 * */
struct Room
{
	char name[25];
	char type[25];
	int numOutboundConnections;
	char OutboundConnections[6][25];
};

/*Initialize a mutex.*/
pthread_mutex_t mainMutex = PTHREAD_MUTEX_INITIALIZER;

/*Based on Block 2.4 reading and example. Finds the directory most recently created by buildrooms
 * and returns in the reference argument.*/
void getDir(char* dir) {
	int newestDirTime = -1;
	char targetDirPrefix[25] = "mcdadem.rooms.";
	char newestDirName[50];
	memset(newestDirName,'\0',sizeof(newestDirName));

	struct dirent *fileInDir;
	struct stat dirAttributes;

	DIR* dirToCheck;
	dirToCheck = opendir(".");		/*Open the current directory.*/

	if(dirToCheck!=NULL) {
		/*Check every file in the current directory*/
		while((fileInDir = readdir(dirToCheck))!=NULL) {
			/*If the folder has the correct prefix, use stat to check the date.*/
			if(strstr(fileInDir->d_name,targetDirPrefix)!=NULL) {
				stat(fileInDir->d_name,&dirAttributes);
				/*Check each folder with the correct prefix for the most recent modification.*/
				if((int)dirAttributes.st_mtime > newestDirTime) {
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName,'\0',sizeof(newestDirName));
					strcpy(newestDirName,fileInDir->d_name);
				}
			}
		}
	}
	/*Pass the correct directory name back by reference.*/
	strcpy(dir,newestDirName);
	closedir(dirToCheck);

	return;
}

/*Would like to break this out into several more indepedent functions on future iterations.	!!*/
struct Room* loadRoomsFromFiles() {
	struct Room* Rooms;
	/*Allocate memory to load seven rooms.*/
	Rooms = calloc(7, sizeof(struct Room));

	int roomsIndex = 0;
	char fileName[50];
	char dirName[50];
	getDir(dirName);
	
	struct dirent *fileInDir;
	DIR* targetDir;
	targetDir = opendir(dirName);

	if(targetDir!=NULL) {
		/*Look at every file in the target folder.*/
		while((fileInDir = readdir(targetDir))!=NULL) {
			/*Comparing to length 2 as an easy way to weed out [.] and [..], which should be the
			 * only non-room files in the folder.*/
			if(strlen(fileInDir->d_name)>2) {
				/*Build the file's pathname from the directory where the program is running.*/
				strcpy(fileName,dirName);
				strcat(fileName,"/");
				strcat(fileName,fileInDir->d_name);

				char buffer[25];
				FILE* ifile;
				ifile = fopen(fileName,"r");
				if(!ifile) { 
					perror("Couldn't open one of the room files");
					return NULL;
				}
				/*Nice little fscanf functionality used here to discard the unwanted string
				 * sections: "%*s" will discard the first string. This processing is very
				 * dependent upon all the files having a correct and predictable structure.*/
				if(fscanf(ifile,"%*s%*s%s\n",buffer)!=EOF) {
					strcpy(Rooms[roomsIndex].name,buffer);
				}
				int connectionsIndex = 0;
				while(fscanf(ifile,"%*s%*s%s\n",buffer)!=EOF) {
					/*Looking for START_ROOM, MID_ROOM, END_ROOM using strstr.*/
					if(strstr(buffer,"ROOM")!=NULL){
						strcpy(Rooms[roomsIndex].type,buffer);
					}
					/*If it's not first, and it's not a type(ROOM), then it's a connection.*/
					else {
						strcpy(Rooms[roomsIndex].OutboundConnections[connectionsIndex],buffer);
						connectionsIndex++;
					}
				}
				fclose(ifile);
				Rooms[roomsIndex].numOutboundConnections = connectionsIndex;
				roomsIndex++;
			}
		}
	}
	closedir(targetDir);

	return Rooms;
}

/*Just loops through all the rooms looking for the start room and returns its address.*/
struct Room* findStartingRoom(struct Room* Rooms) {
	int i;
	for(i=0;i<ROOM_COUNT;i++) {
		if(strcmp(Rooms[i].type,"START_ROOM")==0) {
			return &Rooms[i];
		}
	}
	return Rooms;
}

/*Prints the menu specified in the requirements to stdout.*/
void printMenu(struct Room* currentRoom) {
	printf("CURRENT LOCATION: %s\n",currentRoom->name);
	printf("POSSIBLE CONNECTIONS: ");
	int i;
	for(i=0;i<currentRoom->numOutboundConnections-1;i++) {
		printf("%s, ",currentRoom->OutboundConnections[i]);
	}
	/*Printed seperately to get that period at the end of the list of connections.*/
	printf("%s.",currentRoom->OutboundConnections[currentRoom->numOutboundConnections-1]);
	printf("\nWHERE TO? >");		/*Prompt left immediately after the >.*/
	return;
}

/*You won! Prints the game result statement, including number of steps taken.*/
/*Need to add room path printing to this function, eventually.							!!*/
void printResults(int steps) {
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",steps);
	/*print path rooms.*/
	return;
}

/*Writes the current time to a file named currentTime.txt*/
void* writeCurrentTimeFile() {
	char buffer[50];
	time_t now = time(&now);
	struct tm* currentTime;
	currentTime = localtime(&now);
	/*Format time using strftime. Note: ISO C doesn't like %P, which is lowercase am/pm.*/
	strftime(buffer,sizeof(buffer),"%I:%M%P, %A, %B %d, %Y",currentTime);
	
	FILE* ofile;
	ofile = fopen("currentTime.txt","w+");
	if(!ofile) {
		perror("Could not open file, currentTime.txt\n");
	}
	else {
		fputs(buffer,ofile);
	}
	fclose(ofile);

	return NULL;
}

/*Reads the time from the file currentTime.txt into a buffer and prints it.*/
void printCurrentTimeFile() {
	char buffer[50];
	
	FILE* ifile;
	ifile = fopen("currentTime.txt","r");
	if(!ifile) {
		perror("Could not open file, currentTime.txt\n");
	}
	else {
		fgets(buffer,sizeof(buffer),ifile);
		printf(" %s\n\n",buffer);
	}
	fclose(ifile);

	return;
}

/*Here's the meat. Coordinates the various game functions.*/
void playGame(struct Room* Rooms) {
	int steps = 0;
	char path[100][25];		/*Gosh, I hope no one uses more than 100 steps, maybe use dynamic?*/
	struct Room* currentRoom;
	currentRoom = findStartingRoom(Rooms);
	/*Run the game until the user gets to the END_ROOM*/
	while(strcmp(currentRoom->type,"END_ROOM")!=0) {
		printMenu(currentRoom);

		char nextRoom[25];
		/*fgets is really easy to use and is buffer size limited.*/
		fgets(nextRoom,sizeof(nextRoom),stdin);
		strtok(nextRoom,"\n");	/*obligatory strtok to strip newline from stdin.*/
		printf("\n");

		bool foundIndicator = false;
		int i;
		for(i=0;i<ROOM_COUNT;i++) {
			/*Run the time functions in a locking thread.*/
			if(strcmp(nextRoom,"time")==0) {
				/*insert threading mutex stuff here.*/
				pthread_t timeThread;
				pthread_mutex_lock(&mainMutex);
				pthread_create(&timeThread,NULL,writeCurrentTimeFile,NULL);
				pthread_mutex_unlock(&mainMutex);
				pthread_join(timeThread,NULL);
				printCurrentTimeFile();
				foundIndicator = true;
				break;
			}
			/*I think these two nested loops could be simplified.						!!*/
			else if(strcmp(Rooms[i].name,nextRoom)==0) {
				int j;
				for(j=0;j<currentRoom->numOutboundConnections;j++) {
					if(strcmp(currentRoom->OutboundConnections[j],nextRoom)==0) {
						currentRoom = &Rooms[i];
						strcpy(path[steps],currentRoom->name);
						steps++;
						foundIndicator = true;
						break;
					}
				}
			}
		}
		/*If the user input doesn't match time or any valid room selection, act confused.*/
		if(foundIndicator==false) {
			printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
		}
	}

	printResults(steps);
	/*This print loop should really be in the printResults function, but I had some trouble	!!
	//passing it correctly--needs work.
	*/
	int i;
	for(i=0;i<steps;i++) {
		printf("%s\n",path[i]);
	}
	return;
}

/* Main: aims to make the process of loading the files and playing the game easy to conceptualize.*/
int main() {
	/*Declare an array of seven Room structs.*/
	struct Room* Rooms;

	/*Initialize each room with an id, room type, and name from room files.*/
	Rooms = loadRoomsFromFiles();

	/*Plays the game until the user arrives at the END_ROOM.*/
	playGame(Rooms);

	/*Clean up.*/
	free(Rooms);
	pthread_mutex_destroy(&mainMutex);

	return 0;
}
