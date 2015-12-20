#ifndef PORSON_H
#define PORSON_H 

typedef struct _Person Person;
struct _Person {
	char *firstName;
	char *lastName;
	char *birthDate;
	char *email;
	char *phoneNumber;
	char *address;
};

void printPerson(Person *person);

#endif
