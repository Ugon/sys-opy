#include <stdio.h>
#include "person.h"

void printPerson(Person *person) {
	printf("Person{\n");
	printf("firstName:   %s\n", person -> firstName);
	printf("laseName:    %s\n", person -> lastName);
	printf("birthDate:   %s\n", person -> birthDate);
	printf("email:       %s\n", person -> email);
	printf("phoneNumber: %s\n", person -> phoneNumber);
	printf("address:     %s\n", person -> address);
	printf("}\n");
}