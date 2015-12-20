#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <string.h>
#include "list.h"
#include "lambda.h"
#include "person.h"

#ifdef DLL
#include <dlfcn.h>
#endif

void printTimeInfo(struct tms startTime, struct tms lastChechpointTime, struct tms currentTime){
	printf("\n");
	printf("CHECKPOINT           |   real   |   user   |  system  |\n" );
	printf("---------------------|----------|----------|----------|\n" );
	printf("current              |%10d|%10d|%10d|\n", 0, (int) currentTime.tms_utime                                     , (int) currentTime.tms_stime                                      );
	printf("from start           |%10d|%10d|%10d|\n", 0, (int) currentTime.tms_utime - (int) startTime         .tms_utime, (int) currentTime.tms_stime - (int) startTime         .tms_stime );
	printf("from last checkpoint |%10d|%10d|%10d|\n", 0, (int) currentTime.tms_utime - (int) lastChechpointTime.tms_utime, (int) currentTime.tms_stime - (int) lastChechpointTime.tms_stime );
	printf("\n");
}

void printInfo(char *info){
	printf("\n");
	printf("-------------------------------------------------------\n");
	printf("INFO: %s", info);
	printf("-------------------------------------------------------\n\n");
}

int main(){
	#ifdef DLL
	List *(*createList)();
	void (*destroyList)(List*);	
	void (*addElement)(List*, void*);
	void (*removeElement)(List*, void*);
	void (*setOrdering)(List*, ComparingFunction);
	//void (*removeOrdering)(List*);
	void *(*find)(List*, Predicate);
	//void (*map)(List*, MappingFunction);
	//void (*filter)(List*, Predicate);
	void (*printList)(List*, PrintingFunction);

	void (*printPerson)(Person*);
	

	void *liblist   = dlopen("liblist.so", RTLD_LAZY);
	createList      = dlsym(liblist , "createList");
	destroyList     = dlsym(liblist , "destroyList");
	addElement      = dlsym(liblist , "addElement");
	removeElement   = dlsym(liblist , "removeElement");
	setOrdering     = dlsym(liblist , "setOrdering");
	//removeOrdering  = dlsym(liblist , "removeOrdering");
	find            = dlsym(liblist , "find");
	//map             = dlsym(liblist , "map");
	//filter          = dlsym(liblist , "filter");
	printList       = dlsym(liblist , "printList");

	void *libperson = dlopen("libperson.so", RTLD_LAZY);
	printPerson     = dlsym(libperson , "printPerson");
	#endif

	struct tms startCheckpoint;
	times(&startCheckpoint);
	printInfo("Starting test execution\n");
	printTimeInfo(startCheckpoint, startCheckpoint, startCheckpoint);

	printInfo("Instantiating 4 people\n");
	Person person1 = {.firstName =     "Janek", 
	                  .lastName =      "Kowalski",
	                  .birthDate =     "2000-01-23",
	                  .email =         "janek@kowalski.ru",
	                  .phoneNumber =   "666-257-123",
	                  .address =       "russland 13"};
	
	Person person2 = {.firstName =     "Waldus", 
	                  .lastName =      "Kiepski",
	                  .birthDate =     "2001-02-23",
	                  .email =         "waldus@kiepski.pl",
	                  .phoneNumber =   "654-111-123",
	                  .address =       "cwiartki trzy przez cztery"};
	
	Person person3 = {.firstName =     "Mietek", 
	                  .lastName =      "Monopolowy",
	                  .birthDate =     "2000-01-23",
	                  .email =         "mietek.monopolowy324454124@poczta.onet.pl",
	                  .phoneNumber =   "000-257-123",
	                  .address =       "Ulica 8"};
	
	Person person4 = {.firstName =     "Kazik", 
	                  .lastName =      "Nowak",
	                  .birthDate =     "1954-01-23",
	                  .email =         "kazik@nowak.com",
	                  .phoneNumber =   "222-257-123",
	                  .address =       "bezdomny"};

	printPerson(&person1);
	printPerson(&person2);
	printPerson(&person3);
	printPerson(&person4);

	printInfo("Creating a list\n");
	List *list = createList();
    printList(list, (PrintingFunction) printPerson);

	printInfo("Adding 3 people to the list\n");
	addElement(list, &person1);
	addElement(list, &person2);
	addElement(list, &person3);
    printList(list, (PrintingFunction) printPerson);
	
	struct tms secondCheckpoint;
	times(&secondCheckpoint);
	printTimeInfo(startCheckpoint, startCheckpoint, secondCheckpoint);

	printInfo("Sorting list by Person.firstName\n");
	setOrdering(list, (ComparingFunction) lambda (int, (const Person *a, const Person *b), { 
		return strcmp(a -> firstName, b -> firstName);
	}));
    printList(list, (PrintingFunction) printPerson);

	printInfo("Adding 4th person\n");
	addElement(list, &person4);
    printList(list, (PrintingFunction) printPerson);

	struct tms thirdCheckpoint;
	times(&thirdCheckpoint);
	printTimeInfo(startCheckpoint, secondCheckpoint, thirdCheckpoint);

	printInfo("Sorting list by Person.email DESCENDING\n");
	setOrdering(list, (ComparingFunction) lambda (int, (const Person *a, const Person *b), { 
		return - strcmp(a -> email, b -> email);
	}));
    printList(list, (PrintingFunction) printPerson);

	printInfo("Finding a person in a list with a given address: Ulica 8\n");
	Person *foundPerson = find(list, (Predicate) lambda (bool, (const Person *a), {
		return strcmp(a -> address, "Ulica 8") == 0;
	}));
	printPerson(foundPerson);

	printInfo("Removing that person from the list\n");
	removeElement(list, foundPerson);
    printList(list, (PrintingFunction) printPerson);

	struct tms fourthCheckpoint;
	times(&fourthCheckpoint);
	printTimeInfo(startCheckpoint, thirdCheckpoint, fourthCheckpoint);

	printInfo("Deleting list\n");
	destroyList(list);

	#ifdef DLL
	dlclose(liblist);
	dlclose(libperson);
	#endif

}






