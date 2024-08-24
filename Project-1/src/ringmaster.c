
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> 

#define MAX_INPUT_LENGTH 1024
#define MAX_NAME_LENGTH 32
#define MAX_TOKENS 256
#define MAX_ITEMS 20
#define MAX_SUBJECTS 20
#define MAX_LOCATIONS 20



bool is_numeric_string(char *str);
bool is_valid_word(char *str);

int get_subject_item_quantity(char* subject_name, char* item_name);
int print_all_items(char* name);
int print_location(char* name);
int print_people_at(char* location_name);

bool contains_keyword(char **tokens, int *token_count);
int get_actionword_index(char **tokens, int token_count, int start_index);
int get_conditionword_index(char **tokens, int token_count, int start_index);
int get_questionword_index(char **tokens, int token_count, int start_index);
int get_if_index(char **tokens, int token_count, int start_index);
bool is_keyword(char* token);
bool initial_valid_check(char **tokens, int token_count);

char** parse_sentence(char *input, int *token_count);
char** split_actions(char** tokens, int token_count, int *start_index, int *word_count);
char** split_conditions(char** tokens, int token_count, int *start_index, int *word_count);

int execute_sentences(char **tokens, int token_count);
int execute_action(char** action_sentence, int word_count);;
int condition_check(char** condition_sentence, int word_count);
int answer_question(char **tokens, int token_count);

void free_tokens(char **tokens, int token_count);


//
// 1. Useful functions that are non-related to project
//

// Function to check whether a string is a number
bool is_numeric_string(char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return false; // Not a digit
        }
        str++;
    }
    return true; // All characters are digits
}


// Function to check whether a name is a valid word
bool is_valid_word(char *str) {
    // if the word is a keyword, than it is invalid
    if (is_keyword(str)) {
        return false;
    }

    while (*str) {
        // Check if the character is a letter or underscore
        if (!(isalpha(*str) || *str == '_')) {
            return false; // Character is not valid
        }
        str++;
    }
    return true; // All characters are valid
}

//
// 2. Structures for Subject, Item and Location
//

// Struct for Item 
typedef struct {
    char name[MAX_NAME_LENGTH];
    int quantity;
} Item;

// Struct for Subject 
typedef struct {
    char name[MAX_NAME_LENGTH];
    int item_count;  // Number of items in subjects inventory
    Item items[MAX_ITEMS];
    char location_name[MAX_NAME_LENGTH];
} Subject;

// Struct for Location
typedef struct {
    char name[MAX_NAME_LENGTH];
    Subject *subjects[MAX_SUBJECTS];
    int subject_count; // Number of subjects in location
} Location;


// all subjects list to access a subject
Subject subjects[MAX_SUBJECTS];
int num_subjects = 0;

// all locations list to access a location
Location locations[MAX_LOCATIONS];
int num_locations = 0;

//
// 3. Structure controlling functions (getters and creaters)
//

// Function to get the Subject by using name
Subject* get_subject(char *name) {
    // Search for the subject with the specified name in subjects list
    for (int i = 0; i < num_subjects; i++) {
        if (strcmp(subjects[i].name, name) == 0) {
            // Subject found, return the pointer to subject
            return &subjects[i];
        }
    }
    // Subject not found
    return NULL;
}

// Function to create a new Subject or return an existing Subject if it is already created
Subject* create_subject(char *name) {
    // Check if the subject already exists
    Subject *subject = get_subject(name);
    if (subject != NULL) {
        // Subject already exists, return it
        return subject;
    }
     // check if the name is a keyword, subject names cannot be keywords
    if(is_keyword(name)) {
        return NULL;
    }

    // Create a new subject
    Subject *new_subject = &subjects[num_subjects++];
    strcpy(new_subject->name, name);
    new_subject->item_count = 0; // initialize the item count as 0
    strcpy(new_subject->location_name, "NOWHERE"); // initialize the location as "NOWHERE" (this may be unnecessary to initialize here)

    // return the pointer to subject
    return new_subject;
}

// Function to get the Item of a Subject by using item name
Item* get_item_of_subject(char *item_name, Subject *subject) {
    // Search for the item with the specified name in Subject's inventory
    // if there is no subject, return NULL
    if (subject == NULL) {
        return NULL;
    }
    for (int i = 0; i < subject->item_count; i++) {
        if (strcmp(subject->items[i].name, item_name) == 0) {
            // Item found, return the pointer to item
            return &subject->items[i];
        }
    }
    // Item not found
    return NULL;
}

// Function to create a new Item of a Subject or return an existing Item if it is already created
Item* create_item_of_subject(char *item_name, Subject *subject) {

    // Check if the item already exists in Subject's inventory
    Item *item = get_item_of_subject(item_name, subject);
    if (item != NULL) {
        // Item already exists, return it
        return item;
    }

    // item name cannot be a keyword, return NULL if so
    if(is_keyword(item_name)) {
        return NULL;
    }

    // Create a new item if it does not exist
    Item *new_item = &subject->items[subject->item_count++];
    strcpy(new_item->name, item_name);

    // return the pointer to item
    return new_item;
}

// Function to get the location by using name
Location* get_location(char *name) {
    // Search for the location with the specified name in locations list
    for (int i = 0; i < num_locations; i++) {
        if (strcmp(locations[i].name, name) == 0) {
            // Location found, return the pointer to location
            return &locations[i];
        }
    }

    // Location not found
    return NULL;
}

// Function to create a new Location, if it already exists, return it 
Location* create_location(char *name) {

    // Check if the location already exists
    Location *location = get_location(name);
    if (location != NULL) {
        // Location already exists, return it
        return location;
    }

    // location name cannot be keyword, return NULL if so
    if(is_keyword(name)) {
        return NULL;
    }
    // Create a new location
    Location *new_location = &locations[num_locations++];
    strcpy(new_location->name, name);

    // return the pointer to location
    return new_location;
}

//
// 4. Action functions
//

// Function to add an item to a subject with a quantity
int add_item_to_subject(Subject *subject, char *item_name, int quantity) {
    // Check if the item already exists, if not create one
    Item *item = create_item_of_subject(item_name, subject);
    // return -1 if there is a problem creating the item (may be unnecesary)
    if (item == NULL) {
        return -1;
    }
    // update the quantity
    item->quantity += quantity;
    return 0;
}

// Function to subtract an item from a subject with quantity 
int subtract_item_from_subject(Subject *subject, char *item_name, int quantity) {
    // check if item exists
    Item *item = get_item_of_subject(item_name, subject);

    if (item == NULL) {
        return 0; // if item is not found, do nothing, return
    }
    // update the quantity
    item->quantity -= quantity;
    // if all items removed, make the quantity 0, it cannot be negative
    if (item->quantity < 0) {
        item->quantity = 0;
    }

    return 0;

}

// Function to execute the buy function between a seller subject and a buyer subject
int buy_from(Subject* buyer_subject, Subject *seller_subject, char *item_name, int quantity) {
    // check if the buyer and seller are same subjects, if so return -1 since this is invalid
    if (buyer_subject == seller_subject) {
        return -1;
    }
    // check if there is enough item in seller
    Item *sellers_item = get_item_of_subject(item_name, seller_subject);
    if (sellers_item == NULL) {
        return 0; // not invalid, just return
    }
    if (sellers_item->quantity < quantity) {
        return 0; // not invalid, just return
    }
    // if there are enought items, execute the buy action (subtract the item from seller and add to the buyer), check if the operations are successful, return -1 if there are problems
    if (subtract_item_from_subject(seller_subject, item_name, quantity) == -1 || add_item_to_subject(buyer_subject, item_name, quantity) == -1) {
        return -1;
    }
    
    // if everything is fine, return 0
    return 0;
}

// Function to change the location of a subject
int change_location(Subject *subject, Location *location) {
    // if the subject belongs to another location, remove it from subject list of that location
    Location* old_location = get_location(subject->location_name);
    if (old_location != NULL && old_location != location) {
        for (int i = 0; i < old_location->subject_count; i++) {
            if (old_location->subjects[i] == subject) {
                // change the element in found index with the last element and then make the last element null and decrement the subject count
                old_location->subjects[i] = old_location->subjects[old_location->subject_count];
                old_location->subjects[old_location->subject_count] = NULL;
                old_location->subject_count--;
                break;
            }
        }
    }

    // change the subject's location
    strcpy(subject->location_name, location->name);
    // add the subject to location's subject list if it is not there already
    for (int i = 0; i < location->subject_count; i++) {
        if (subject == location->subjects[i]) {
            // do nothing
            return 0;
        }
    }
    // if the subject is not in location, add it
    location->subjects[location->subject_count++] = subject;

    return 0;

}

//
// 5. Question functions 
//

// Function to get the quantity of an item of a subject (we have a function to get the Item pointer above, this returns the quantity directly)
int get_subject_item_quantity(char* subject_name, char* item_name) {

    Subject *subject = get_subject(subject_name);
    // if there is no such subject, return 0
    if (subject == NULL) {
        return 0;
    }
    Item *item = get_item_of_subject(item_name, subject);

    // if the item is found, return the quantity
    if (item != NULL) {
        return item->quantity;
    }
    // if there is no item, return 0
    return 0;
}

// Function to print all items of a subject with their quantities, seperated with "and", using the subject name
int print_all_items(char* name) {

    Subject *subject = get_subject(name);
    if (subject != NULL) {
        int notemptyflag = 0; // flag to indicate if the inventory is empty (nothing besides items with 0 quantities) or not
        for (int i = 0; i < subject->item_count; i++) { // search the inventory to see if there are any items with a quantity more than 0
            if (subject->items[i].quantity != 0) {
                notemptyflag++;
                break;
            }
        }
        // if the inventory is empty, print "NOTHING"
        if (notemptyflag == 0) {
            printf("NOTHING\n");
            return 0;
        }
        // else print all items with quantities and names
        for (int i = 0; i < subject->item_count; i++) {
            if (subject->items[i].quantity == 0) { // ignore the items with 0 quantities
                continue;
            } else {
                printf("%d %s", subject->items[i].quantity, subject->items[i].name);
                if (i != subject->item_count -1) {
                    printf(" and "); // add "and" between items
                }
            }
        }
        printf("\n"); //print new line after all items are printed
    } else {
        // print "NOTHING" if subject is not found
        printf("NOTHING\n");
        return 0;
    }
    return 0; //this may be unnecesary
}

// Function to print the location of a subject
int print_location(char* name) {
    Subject *subject = get_subject(name);
    // print the location if subjects location is available, else print "NOWHERE"
    if (subject != NULL) {
        printf("%s\n", subject->location_name);
    } else {
        printf("NOWHERE\n");
        return 0;
    }
    return 0;
}

// Function to print people at given location
int print_people_at(char* location_name) {
    Location *location = get_location(location_name);
    // print "NOBODY" if location is not found
    if (location == NULL) {
        printf("NOBODY\n");
        return 0;
    }
    // print "NOBODY" if there are no subjects in location
    if (location->subject_count == 0) {
        printf("NOBODY\n");
        return 0;
    }
    // else, print subject names seperated with " and "
    for (int i = 0; i < location->subject_count; i++) {
        if (i != 0) {
            printf(" and ");
        }
        printf("%s", location->subjects[i]->name);
    }
    printf("\n");
    return 0;
}

//
// 6. Input controlling functions and data types
//

// Array of keywords
char *keywords[] = {"sell", "buy", "go", "to", "from", "and", "at", "has", "if", "less", "more", "than", "exit", "where", "total", "who", "NOBODY", "NOTHING", "NOWHERE"};
int num_keywords = 19;

// Array of action words
char *actionwords[] = {"buy", "sell", "go"};
int num_actionwords = 3;

// Array of condition words
char *conditionwords[] = {"at", "has"};
int num_conditionwords = 2;

// Array of question words
char *questionwords[] = {"total", "where", "who"};
int num_questionwords = 3;

// Function to check if tokens contains any of the keywords
bool contains_keyword(char **tokens, int *token_count) {
    for (int i = 0; i < *token_count; i++) {
        for (int j = 0; j < num_keywords; j++) {
            if (strcmp(tokens[i], keywords[j]) == 0) {
                return true;
            }
        }
    }
    return false;
}

// Function to check if tokens contains any of the action words and return the index of the action word
int get_actionword_index(char **tokens, int token_count, int start_index) {
    for (int i = start_index; i < token_count; i++) {
        for (int j = 0; j < num_actionwords; j++) {
            if (strcmp(tokens[i], actionwords[j]) == 0) {
                return i;
            }
        }
    }
    // return -1 if not found
    return -1;
}

// Function to check if tokens contains any of the condition words and return the index of the condition word
int get_conditionword_index(char **tokens, int token_count, int start_index) {
    for (int i = start_index; i < token_count; i++) {
        for (int j = 0; j < num_conditionwords; j++) {
            if (strcmp(tokens[i], conditionwords[j]) == 0) {
                return i;
            }
        }
    }
    // return -1 if not found
    return -1;
}

// Function to check if tokens contains any of the question words and return the index of the question word
int get_questionword_index(char **tokens, int token_count, int start_index) {
    for (int i = start_index; i < token_count; i++) {
        for (int j = 0; j < num_questionwords; j++) {
            if (strcmp(tokens[i], questionwords[j]) == 0) {
                return i;
            }
        }
    }
    // return -1 if not found
    return -1;
}

// Function to check if tokens contains the word "if" and return the index of it
int get_if_index(char **tokens, int token_count, int start_index) {
    for (int i = start_index; i < token_count; i++) {
        if (strcmp(tokens[i], "if") == 0) {
            return i;
        }
    }
    // return -1 if not found
    return -1;
}

// Function to check if a word is a keyword
bool is_keyword(char* token) {
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(token, keywords[i]) == 0) {
                return true;
        }
    }
    return false;
}


// Function to make an initial check to see whether input is invalid or not
bool initial_valid_check(char **tokens, int token_count) {
    // return true if input is a valid sentence, otherwise false
    if(strcmp(tokens[0], "and") == 0 || strcmp(tokens[token_count -1], "and") == 0) {
        // the sentence cannot start or end with "and"
        return false;
    }

    // search the tokens to see if two keywords or two numbers follow each other, if so return false because it is invalid
    // consecutive normal names without "and" or other keywords are also invalid
    int keyword_flag = 0; // flag to indicate if the previous token is keyword
    int number_flag = 0; // flag to indicate if the previous token is number
    for (int i = 0; i < token_count; i++) {
        if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
            if(is_keyword(tokens[i])) {
                // if two keywords follow each other, the sentence is invalid  // except "go to", "has less than", "has more than" and "who at".
                if(strcmp(tokens[i], "to") == 0 && strcmp(tokens[i - 1], "go") == 0) {
                    continue;
                }
                if((strcmp(tokens[i], "less") == 0 || strcmp(tokens[i], "more") == 0) && i < token_count - 3) {
                    if(strcmp(tokens[i - 1], "has") == 0 && strcmp(tokens[i + 1], "than") == 0) {
                        i++;
                        continue;
                    } 
                }
                if(strcmp(tokens[i], "at") == 0 && strcmp(tokens[i - 1], "who") == 0) {
                    continue;
                }
                // other than these, no keywords can follow other
                if (keyword_flag != 0) {
                    return false;
                } else { // if the previous token is not keyword and this token is, increment the flag and continue
                    keyword_flag++;
                    continue;
                }
            }
            if(is_numeric_string(tokens[i])) {
                // if two numbers follow each other, the sentence is invalid
                if (number_flag != 0) {
                    return false;
                } else { // if the previous token is not number and this token is, increment the flag and continue
                    number_flag++;
                    continue;
                }
            }
        } else {
            // If two names that are not keyword nor number follow each other, the sentence is invalid
            if (i != 0 && keyword_flag == 0 && number_flag == 0 && strcmp(tokens[i], "?") != 0) {
                return false;
            } else {
                // if everything is fine and the token is a normal word, reset the flags and continue
                keyword_flag = 0;
                number_flag = 0;
            }
            // also, if the name contains anything other than letter or an underscore, it is invalid
            if (!is_valid_word(tokens[i]) && strcmp(tokens[i], "?") != 0) {
                return false;
            }
        }
    }

    // return ture if there are no problems, check other invalidations in different steps
    return true;
}


//
// 7. Parsing functions
//

// Function to parse the sentence into tokens and keep them in an array
char** parse_sentence(char *input, int *token_count) {

    char **tokens = malloc(MAX_TOKENS * sizeof(char*)); // Allocate memory for pointers to tokens
    *token_count = 0; //initialize token count
    char *token = strtok(input, " "); //tokenize the input with " "

    while (token != NULL && *token_count < MAX_TOKENS) {
        tokens[*token_count] = strdup(token); // Duplicate token and store it
        (*token_count)++; //increment the token count
        token = strtok(NULL, " "); // and continue
    }
    // return the pointer
    return tokens;
}

// Function to split an action sentence from tokens according to action words
char** split_actions(char** tokens, int token_count, int *start_index, int *word_count) {

    int index;
    index = get_actionword_index(tokens, token_count, *start_index); // get the index of the first action word

    if (index != -1) {
        // split the sentence according to action word

        // Subject(s) buy Item(s) (from Subject)
        if(strcmp(tokens[index], "buy") == 0) { 
            
            // check if there are other keywords in left side of buy, return NULL if so because that means an invalidation
            for (int i = *start_index; i < index; i++) {
                if (strcmp(tokens[i], "and") == 0) { // skip "and"
                    continue;
                }
                if (is_keyword(tokens[i])) {
                    return NULL;
                }
            } 
            // Calculate the word count for a "buy" sentence
            // first, add the words between the start and buy
            *word_count += index - *start_index + 1;

            // then search for the right end of the sentence
            for (int i = index + 1; i < token_count; i++) {
                // If there is a condition sentence starting, finish the action sentence
                if (strcmp(tokens[i], "if") == 0) {
                    break;
                }
                // If subject buys the item "from" another subject, add the words "from" and seller subjects name and finish the action sentence (there cant be more than one seller)
                if (strcmp(tokens[i], "from") == 0) {
                    (*word_count) += 2;
                    break;
                }

                // If there is a number, add number and item words and continue
                if (is_numeric_string(tokens[i])) {
                    (*word_count) += 2;
                    i++;
                    continue;
                }

                // If there is an "and", there are two possibilities, either another item coming or a new action sentence starting, check the condition by checking next word
                if (strcmp(tokens[i], "and") == 0 && i < token_count - 1 ) {
                    if (is_numeric_string(tokens[i + 1])) {
                        (*word_count)++;
                        continue;
                    } else {
                        break;
                    }
                }     
            }
            // check for more invalidations
            if (strcmp(tokens[*word_count - 2], "from") == 0) {
                // check if there are other keywords in between buy and from
                for (int i = index + 1; i < *word_count - 2; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            } else {
                // check if there are other keywords in right side of buy
                for (int i = index + 1; i < *word_count; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            }
            // do a check to see whether there are same names (not keywords) in sentence, return NULL if so
            for (int i = *start_index; i < *word_count - 1; i++) {
                if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
                    continue;
                }
                for (int j = i + 1; j < *word_count; j++) {
                    if (is_keyword(tokens[j]) || is_numeric_string(tokens[i])) {
                        continue;
                    }
                    if (strcmp(tokens[i], tokens[j]) == 0) {
                        // sentence is invalid, return NULL
                        return NULL;
                    }
                }
            }
            // if all checks are done and word count is calculated, split the sentence
            char **action_sentence = malloc(*word_count * sizeof(char*)); // allocate memory for pointers to tokens
            for (int i = *start_index; i < *start_index + *word_count; i++) {
                action_sentence[i - *start_index] = tokens[i]; // assign the sentence
            }
            
            (*start_index) += (*word_count); // push the start index to the end of the sentence (start of a new sentence)
            if ((*start_index < token_count) && strcmp(tokens[*start_index], "and") == 0) {
                (*start_index)++; // if there is an and after the sentence, increment start index one more
            }

            // Return the sentence with basic "buy" action and nothing more, execute the sentence in different step
            return action_sentence;
        }

        // Subject(s) sell Item(s) (to Subject)
        if(strcmp(tokens[index], "sell") == 0) {

            // check if there are other keywords in left side of sell
            for (int i = *start_index; i < index; i++) {
                if (strcmp(tokens[i], "and") == 0) {
                    continue;
                }
                if (is_keyword(tokens[i])) {
                    return NULL;
                }
            }
            // Calculate the word count for a "sell" sentence
            // add the left side words
            *word_count += index - *start_index + 1;

            // then search for the right end of the sentence
            for (int i = index + 1; i < token_count; i++) {
                // If there is a condition sentence starting, finish the action sentence
                if (strcmp(tokens[i], "if") == 0) {
                    break;
                }
                // If subject sells the item "to" another subject, add the words "to" and buyer subjects name and finish the action sentence (there can't be more than one buyer)
                if (strcmp(tokens[i], "to") == 0) {
                    (*word_count) += 2;
                    break;
                }
                // If there is a number, add number and item words and continue
                if (is_numeric_string(tokens[i])) {
                    (*word_count) += 2;
                    i++;
                    continue;
                }

                // If there is an "and", there are two possibilities, either another item coming or a new action sentence starting, check the condition by checking next word
                if (strcmp(tokens[i], "and") == 0 && i < token_count - 1 ) {
                    if (is_numeric_string(tokens[i + 1])) {
                        (*word_count)++;
                        continue;
                    } else {
                        break;
                    }
                }     
            }
            // check for more invalidations
            if (strcmp(tokens[*word_count - 2], "to") == 0) {
                // check if there are other keywords in between sell and to
                for (int i = index + 1; i < *word_count - 2; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            } else {
                // check if there are other keywords in right side of sell
                for (int i = index + 1; i < *word_count; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            }
           
            // do a check to see whether there are same names (not keywords) in sentence, return NULL if so
            for (int i = *start_index; i < *word_count - 1; i++) {
                if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
                    continue;
                }
                for (int j = i + 1; j < *word_count; j++) {
                    if (is_keyword(tokens[j]) || is_numeric_string(tokens[i])) {
                        continue;
                    }
                    if (strcmp(tokens[i], tokens[j]) == 0) {
                        // sentence is invalid, return NULL
                        return NULL;
                    }
                }
            }
            // if all checks are done and word count is calculated, split the sentence
            char **action_sentence = malloc(*word_count * sizeof(char*)); // Allocate memory for pointers to tokens
            for (int i = *start_index; i < *start_index + *word_count; i++) {
                action_sentence[i - *start_index] = tokens[i]; // assign the sentence
            }

            (*start_index) += (*word_count); // push the start index to the end of the sentence (start of a new sentence)
            if (*start_index < token_count && strcmp(tokens[*start_index], "and") == 0) {
                (*start_index)++; // if there is an and after the sentence, increment start index one more
            }
            
            // Return the sentence with basic "sell" action and nothing more, execute the sentence in different step
            return action_sentence;
        }

        // Subject(s) go to Location
        if(strcmp(tokens[index], "go") == 0) {
            // 'to' expected after 'go', return NULL otherwise
            if(index < token_count - 1 && strcmp(tokens[index + 1], "to") != 0) {
                return NULL;
            }
            // Calculate the word count for a "go to" sentence

            // check if there are other keywords in left side of go
            for (int i = *start_index; i < index; i++) {
                if (strcmp(tokens[i], "and") == 0) {
                    continue;
                }
                if (is_keyword(tokens[i])) {
                    return NULL;
                }
            }
            // add the left side words (include the word "to", that comes after "go", to the count.)            
            // Since there can be just one location name in the right side of a "go to" sentence, add the location name also to the word count and finish calculating
            *word_count += index - *start_index + 3;

            // do a check whether there are same names (not keywords) in sentence, return NULL if so
            for (int i = *start_index; i < *word_count - 1; i++) { 
                if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
                    continue;
                }
                for (int j = i + 1; j < *word_count; j++) {
                    if (is_keyword(tokens[j]) || is_numeric_string(tokens[i])) {
                        continue;
                    }
                    if (strcmp(tokens[i], tokens[j]) == 0) {
                        // sentence is invalid, return NULL
                        return NULL;
                    }
                }
            }

            // if all checks are done and word count is calculated, split the sentence
            char **action_sentence = malloc(*word_count * sizeof(char*)); // Allocate memory for pointers to tokens
            for (int i = *start_index; i < *start_index + *word_count; i++) {
                action_sentence[i - *start_index] = tokens[i]; // assign the sentence
            }

            (*start_index) += (*word_count); // push the start index to the end of the sentence (start of a new sentence)
            if ( *start_index < token_count && strcmp(tokens[*start_index], "and") == 0) {
                (*start_index)++; // if there is an and after the sentence, increment start index one more
            }

            // Return the sentence with basic "go to" action and nothing more, execute the sentence in different step
            return action_sentence;
        }
    }

    // if index is -1, than there is no action sentence, return NULL
    return NULL;
}

// Function to split a condition sentence from tokens according to condition words
char** split_conditions(char** tokens, int token_count, int *start_index, int *word_count) {

    int index;
    index = get_conditionword_index(tokens, token_count, *start_index); // get the index of the first action word

    if (index != -1) {
        // split the sentence according to condition word

        // Subject(s) at Location
        if (strcmp(tokens[index], "at") == 0) {
            // Calculate the word count for an "at" condition
            
            // check if there are other keywords in left side of at
            for (int i = *start_index; i < index; i++) {
                if (strcmp(tokens[i], "and") == 0) {
                    continue;
                }
                if (is_keyword(tokens[i])) {
                    return NULL;
                }
            }

            // add the left side words
            // Since there can be just one location name in the right side of an "at" condition, add the location name also to the word count and finish the sentence
            *word_count += index - *start_index + 2;

            // do a check whether there are same names (not keywords) in sentence, return NULL if so
            for (int i = *start_index; i < *word_count - 1; i++) {
                if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
                    continue;
                }
                for (int j = i + 1; j < *word_count; j++) {
                    if (is_keyword(tokens[j]) || is_numeric_string(tokens[i])) {
                        continue;
                    }
                    if (strcmp(tokens[i], tokens[j]) == 0) {
                        // sentence is invalid, return NULL
                        return NULL;
                    }
                }
            }
           
            // if all checks are done and word count is calculated, split the sentence
            char **condition_sentence = malloc(*word_count * sizeof(char*)); // allocate memory for pointers to tokens
            for (int i = *start_index; i < *start_index + *word_count; i++) {
                condition_sentence[i - *start_index] = tokens[i]; // assign the sentence
            }

            (*start_index) += (*word_count); // push the start index to the end of the sentence (start of a new sentence)
            if (*start_index < token_count && strcmp(tokens[*start_index], "and") == 0) {
                (*start_index)++; // if there is an and after the sentence, increment start index one more
            }

            // Return the sentence with basic "at" condition and nothing more, execute the sentence in different step
            return condition_sentence;
        }

        // Subject(s) has (less/more than) Item(s)
        if(strcmp(tokens[index], "has") == 0) {
            // Calculate the word count for a "has" sentence

            // check if there are other keywords in left side of has
            for (int i = *start_index; i < index; i++) {
                if (strcmp(tokens[i], "and") == 0) {
                    continue;
                }
                if (is_keyword(tokens[i])) {
                    return NULL;
                }
            }
            // add the left side words
            *word_count += index - *start_index + 1;

            // now search for the right end of the sentence
            for (int i = index + 1; i < token_count; i++) {

                // First, check if the next word is "less" or "more"

                // Subject(s) has less than Item(s)
                // add "less than" to the word count and continue
                if(strcmp(tokens[i], "less") == 0 && i < token_count - 1 ) {
                    if (strcmp(tokens[i + 1], "than") != 0) {
                        return NULL; //'than' is expected after 'less', return NULL otherwise
                    }
                    (*word_count) += 2;
                    i++;
                    continue;
                }

                // Subject(s) has more than Item(s)
                // add "more than" to the word count and continue
                if(strcmp(tokens[i], "more") == 0 && i < token_count - 1 ) {
                    if (strcmp(tokens[i + 1], "than") != 0) {
                        return NULL; //'than' is expected after 'more', return NULL otherwise
                    }
                    (*word_count) += 2;
                    i++;
                    continue;
                }

                // If there is a number, add number and item words and continue
                if (is_numeric_string(tokens[i])) {
                    (*word_count) += 2;
                    i++;
                    continue;
                }

                // If there is an "and", there are two possibilities, either another item coming or a new action sentence starting, check the condition by checking next word
                if (strcmp(tokens[i], "and") == 0  && i < token_count - 1 ) {
                    if (is_numeric_string(tokens[i + 1])) {
                        (*word_count)++;
                        continue;
                    } else {
                        break;
                    }
                }     
            }

            // check for more invalidations
            if (strcmp(tokens[index + 1], "less") == 0 || strcmp(tokens[index + 1], "more") == 0) {
                // check if there are other keywords after 'has less than' / 'has more than'
                for (int i = index + 3; i < *word_count; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            } else {
                // check if there are other keywords in right side of 'has'
                for (int i = index + 1; i < *word_count; i++) {
                    if (strcmp(tokens[i], "and") == 0) {
                        continue;
                    }
                    if (is_keyword(tokens[i])) {
                        return NULL;
                    }
                }
            }
            
            // do a check whether there are same names (not keywords) in sentence, return NULL if so
            for (int i = *start_index; i < *word_count - 1; i++) {
                if (is_keyword(tokens[i]) || is_numeric_string(tokens[i])) {
                    continue;
                }
                for (int j = i + 1; j < *word_count; j++) {
                    if (is_keyword(tokens[j]) || is_numeric_string(tokens[i])) {
                        continue;
                    }
                    if (strcmp(tokens[i], tokens[j]) == 0) {
                        // sentence is invalid, return NULL
                        return NULL;
                    }
                }
            }

            // if all checks are done and word count is calculated, split the sentence
            char **condition_sentence = malloc(*word_count * sizeof(char*)); // Allocate memory for pointers to tokens
            for (int i = *start_index; i < *start_index + *word_count; i++) {
                condition_sentence[i - *start_index] = tokens[i]; // assign the sentence
            }

            *start_index += *word_count; // push the start index to the end of the sentence (start of a new sentence)
            if (*start_index < token_count && strcmp(tokens[*start_index], "and") == 0) {
                (*start_index)++; // if there is an and after the sentence, increment start index one more
            }

            // Return the sentence with basic "has" conditon and nothing more, execute the sentence in different step
            return condition_sentence;
        }
    }

    // if index is -1, than there is no condition sentence, return NULL
    return NULL;
}

//
// 8. Main 
//

int main() {
    char input[MAX_INPUT_LENGTH];

    while (true) {
        printf(">> "); 
        fflush(stdout);
        fgets(input, MAX_INPUT_LENGTH, stdin); // read the input

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Check for exit command, break if input is exit
        if (strcmp(input, "exit") == 0) {
            break;
        }
        
        int token_count = 0; // initialize token count to 0
        // parse the sentence into tokens
        char **tokens = parse_sentence(input, &token_count);

        // do an initial valid check
        if (!initial_valid_check(tokens, token_count)) {
            printf("INVALID\n");
            continue;
        }
        // Determine if input is a sentence or question, if it is a question, answer it, otherwise execute the sentence. Print "INVALID" if input is invalid.
        if (strcmp(tokens[token_count - 1], "?") == 0) {
            if(answer_question(tokens, token_count) == -1) {
                printf("INVALID\n");
                continue;
            }
        } else {
            if (execute_sentences(tokens, token_count) == -1) {
                printf("INVALID\n");
                continue;
            } else {
                printf("OK\n"); // if the sentence is not invalid, print "OK" and continue
                continue;
            }
        }               
    }
    return 0;
}

//
// 9. Executer and logic implementer functions
//

// execute the sentences according to action and condition words
int execute_sentences(char **tokens, int token_count) {

    int start_index = 0; // assign start index to first word of the input

    // there may be multiple sentences in one input, run a while loop to execute all the sentences
    while (start_index < token_count) {    
        
        // first split the conditions by searching for the word "if"
        int if_index = get_if_index(tokens, token_count, start_index);
        int if_start_index = if_index + 1; // assign start index of condition sentences to the next word of if

        if (if_index != -1) { // if there is an 'if', than it is a condition sentence, first check the conditions than execute the actions
            int conditionflag = 1; // flag to indicate whether the conditions are met
            while(if_start_index < token_count) {
                // if there is an action word before condition word, than we should consider new sentence
                int cond_index = get_conditionword_index(tokens, token_count, if_start_index);
                int act_index = get_actionword_index(tokens, token_count, if_start_index);
                if (act_index != -1 && cond_index != -1 && cond_index >= act_index) {
                    break;
                }

                // condition check
                int condition_wc = 0; // word count
                char **condition_sentence = split_conditions(tokens, token_count, &if_start_index, &condition_wc); // split the first condition sentence

                if (condition_sentence == NULL) { // if condition sentence is NULL, than there is an invalidation, return -1
                    return -1;
                }
                
                if (condition_check(condition_sentence, condition_wc) == -1) {
                    // if condition is not satisfied, adjust the flag, free allocated memory and continue to traverse other sentences
                    conditionflag = 0;
                    free_tokens(condition_sentence, condition_wc);
                    continue;
                }
                // if condition is satisfied, just free allocated memory, let flag be the same
                free_tokens(condition_sentence, condition_wc);
            }

            if (conditionflag == 0) { // if conditions are not satisfied, adjust the start index to point next sentence and continue the loop
                start_index = if_start_index;
                continue;
            }

            // if all conditions are satisfied, than execute the actions
            while (start_index < if_index) {
                int word_count = 0;
                char **action_sentence = split_actions(tokens, token_count, &start_index, &word_count); // split the first action sentence

                if (action_sentence == NULL) { // if action sentence is NULL, than there is an invalidation, return -1
                    return -1;
                }
                // execute the action
                if (execute_action(action_sentence, word_count) == -1) { // if there is a problem, free allocated memory and return -1
                    free_tokens(action_sentence, word_count);
                    return -1;
                }
                // if action is executed without problem, free allocated memory
                free_tokens(action_sentence, word_count);
            }
            start_index = if_start_index; // after executing the if sentence, adjust the start index to point the next sentence

        } else {
            // if there is no "if", then the sentence just contains actions
            while (start_index < token_count) {
            
                int word_count = 0;
                char **action_sentence = split_actions(tokens, token_count, &start_index, &word_count); // split the first action sentence

                if (action_sentence == NULL) { // if action sentence is NULL, than there is an invalidation, return -1
                    return -1;
                }
                // execute the action
                if (execute_action(action_sentence, word_count) == -1) { // if there is a problem, free allocated memory and return -1
                    free_tokens(action_sentence, word_count);
                    return -1;
                }
                // if action is executed without problem, free allocated memory
                free_tokens(action_sentence, word_count);
            }
        }
    }
    // return 0 if all the sentences are executed
    return 0;
}


// Function to execute an action
int execute_action(char** action_sentence, int word_count) {
    int index;
    int start_index = 0;
    index = get_actionword_index(action_sentence, word_count, start_index);

    if (index != -1) {

        // Subject(s) buy Item(s) (from Subject)
        if(strcmp(action_sentence[index], "buy") == 0) {

            // check if buy action is between subjects or from an infinite source
            if (strcmp(action_sentence[word_count - 2], "from") == 0) {
                // if there is a seller, get the seller subject
                Subject* seller_subject = create_subject(action_sentence[word_count - 1]);
                if (seller_subject == NULL) {
                    return -1;
                }

                // first, we check if there are enough items on sellers, than we execute
                for (int i = index + 1; i < word_count - 2; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                    }
                    if (get_subject_item_quantity(action_sentence[word_count - 1], action_sentence[i + 1]) >= atoi(action_sentence[i])) {
                        i++;
                    } else {
                        return 0; // it is not invalid, just no action is executed
                    }
                }

                // then exchange the items between subjects with two loops, one for iterating all buyer subjects, one for iterating all items
                for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                    }
                    // get the buyer subject
                    Subject *buyer_subject = create_subject(action_sentence[i]);
                    if (buyer_subject == NULL) {
                        return -1;
                    }
                    for (int j = index + 1; j < word_count - 3; j++) {
                        if (strcmp(action_sentence[j], "and") == 0) {
                            continue;
                        }
                        // buyer buy the item from seller, return -1 if there is a problem
                        if (buy_from(buyer_subject, seller_subject, action_sentence[j + 1], atoi(action_sentence[j])) == -1) {
                            return -1;
                        }
                        // increment j to skip the second word (name) of item
                        j++;
                    }
                }
            } else {
                // if there is no seller subject, buy from an infinite source with two loops
                 for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                    }
                    // get the buyer subject
                    Subject *buyer_subject = create_subject(action_sentence[i]);
                    if (buyer_subject == NULL) {
                        return -1;
                    }
                    for (int j = index + 1; j < word_count - 1; j++) { 
                        if (strcmp(action_sentence[j], "and") == 0) {
                            continue;
                        }
                        // buyer buy the item from an infinite source, return -1 if there is a problem
                        if (add_item_to_subject(buyer_subject, action_sentence[j + 1], atoi(action_sentence[j])) == -1) {
                            return -1;
                        }
                        // increment j to skip the second word (name) of item
                        j++;
                    }
                }
            }
            return 0;
        }

        // Subject(s) sell Item(s) (to Subject)
        if(strcmp(action_sentence[index], "sell") == 0) {

           // check if sell action is between subjects or to an infinite source
            if (strcmp(action_sentence[word_count - 2], "to") == 0) {
                // first, check if there are enough items on sellers, than execute. use two loops one for iterating all seller subjects, one for iterating all items
                for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                                continue;
                    }
                    for (int j = index + 1; j < word_count - 2; j++) {
                        if (strcmp(action_sentence[j], "and") == 0) {
                                continue;
                        }
                        if (get_subject_item_quantity(action_sentence[i], action_sentence[j + 1]) >= atoi(action_sentence[j])) {
                            // if there are enough items, continue
                            j++;
                        } else {
                            return 0; // it is not invalid, just no action is executed
                        }

                    }
                }

                // if there is a buyer, get the buyer subject
                Subject* buyer_subject = create_subject(action_sentence[word_count - 1]);
                if (buyer_subject == NULL) {
                    return -1;
                }
                // then exchange the items between subjects with two loops 
                for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                    }
                    // get the buyer subject
                    Subject *seller_subject = create_subject(action_sentence[i]);
                    if (seller_subject == NULL) {
                        return -1;
                    }
                    for (int j = index + 1; j < word_count - 3; j++) {
                        if (strcmp(action_sentence[j], "and") == 0) {
                            continue;
                        }
                        // buyer buys the item from seller, return -1 if there is a problem
                        if (buy_from(buyer_subject, seller_subject, action_sentence[j + 1], atoi(action_sentence[j])) == -1) {
                            return -1;
                        }
                        // increment j to skip the second word (name) of item
                        j++;
                    }
                }
            } else {
                // first, we check if there are enough items on sellers, than we execute
                for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                                continue;
                    }
                    for (int j = index + 1; j < word_count; j++) {
                        if (strcmp(action_sentence[j], "and") == 0) {
                                continue;
                        }
                        if (get_subject_item_quantity(action_sentence[i], action_sentence[j + 1]) >= atoi(action_sentence[j])) {
                            j++;
                        } else {
                            return 0; // it is not invalid, just no action is executed
                        }
                    }
                }

                // if there is no buyer subject, sell to an infinite source 
                 for (int i = 0; i < index; i++) {
                    if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                    }
                    // get the seller subject
                    Subject *seller_subject = create_subject(action_sentence[i]);
                    if (seller_subject == NULL) {
                        return -1;
                    }
                    for (int j = index + 1; j < word_count - 1; j++) {
                        if (strcmp(action_sentence[j], "and") == 0) {
                            continue;
                        }
                        // seller sell the item to an infite source, return -1 if there is a problem
                        if (subtract_item_from_subject(seller_subject, action_sentence[j + 1], atoi(action_sentence[j])) == -1) {
                            return -1;
                        }
                        // increment j to skip the second word (name) of item
                        j++;
                    }
                }
            }
            return 0;
        }

        // Subject(s) go to Location
        if(strcmp(action_sentence[index], "go") == 0) {
            // get the location, create if it does not exist
            Location *location = create_location(action_sentence[word_count - 1]); 
            if (location == NULL) {
                    return -1;
            }
            // split the subjects and move them to new location
            for (int i = 0; i < index; i++) {
                if (strcmp(action_sentence[i], "and") == 0) {
                            continue;
                }
                // get the subject and change its location
                Subject *subject = create_subject(action_sentence[i]);
                if (subject == NULL) {
                    return -1;
                }
                // return -1 if there is a problem
                if (change_location(subject, location) == -1) {
                    return -1;
                }      
            }
            return 0;
        }
    }
    // if there is no action word, than it is not an action sentence, return -1
    return -1;
}

int condition_check(char** condition_sentence, int word_count) {
    int index;
    int start_index = 0;
    index = get_conditionword_index(condition_sentence, word_count, start_index);

    if (index != -1) {

        // Subject(s) at Location
        if(strcmp(condition_sentence[index], "at") == 0) {

            // get the location, if there is no location, return -1
            Location *location = get_location(condition_sentence[index + 1]);
            if (location == NULL) {
                return -1;
            }
            
            // check if the subjects at location, return -1 if they are not
            for (int i = 0; i < index; i++) {
                if (strcmp(condition_sentence[i], "and") == 0) {
                            continue;
                    }
                Subject *subject = get_subject(condition_sentence[i]);
                if (subject == NULL) {
                    return -1;
                }
                if (strcmp(subject->location_name, location->name) != 0) {
                    return -1;
                }
            }
            // if everything is fine, return 0
            return 0;
        }

        // Subject(s) has Item(s) 
        if(strcmp(condition_sentence[index], "has") == 0) {
            // check whether it is 'has less than' or 'has more than'
            if (strcmp(condition_sentence[index + 1], "less") == 0) {
                // if it is less than, check for subjects and items with two loops, if there are no subjects or items don't return -1, because it counts as less than
                for (int i = 0; i < index; i++) {
                    if (strcmp(condition_sentence[i], "and") == 0) {
                            continue;
                    }
                    Subject *subject = get_subject(condition_sentence[i]);
                    if (subject == NULL) {
                        // since it says less than, dont return -1
                        continue;
                    }
                    for (int j = index + 3; j < word_count; j++) {
                        if (strcmp(condition_sentence[j], "and") == 0) {
                            continue;
                        }
                        int comparison_quantity = atoi(condition_sentence[j]);
                        Item *item = get_item_of_subject(condition_sentence[j+1], subject);
                        if (item == NULL) {
                            // since it says less than, return 0 (it is valid)
                            continue;
                        }
                        if (item->quantity >= comparison_quantity) {
                            // if subject has more or equal, return -1
                            return -1;
                        }

                        j++;
                    }
                }
                //if everything is fine, return 0
                return 0;

            } else if (strcmp(condition_sentence[index + 1], "more") == 0){
                // if it is more than, check for subjects and items with two loops, if there are no subjects or items return -1
                for (int i = 0; i < index; i++) {
                    if (strcmp(condition_sentence[i], "and") == 0) {
                            continue;
                    }
                    Subject *subject = get_subject(condition_sentence[i]);
                    if (subject == NULL) {
                        return -1;
                    }
                    for (int j = index + 3; j < word_count; j++) {
                        if (strcmp(condition_sentence[j], "and") == 0) {
                            continue;
                        }
                        int comparison_quantity = atoi(condition_sentence[j]);
                        Item *item = get_item_of_subject(condition_sentence[j+1], subject);
                        if (item == NULL) {
                            return -1;
                        }
                        if (item->quantity <= comparison_quantity) {
                            // if subject has less or equal, return -1
                            return -1;
                        }
                        
                        j++;
                    }
                }
                //if everything is fine, return 0
                return 0;
                
            } else { // just has
                for (int i = 0; i < index; i++) {
                    if (strcmp(condition_sentence[i], "and") == 0) {
                            continue;
                    }
                    Subject *subject = get_subject(condition_sentence[i]);
                    for (int j = index + 1; j < word_count; j++) {
                        if (strcmp(condition_sentence[j], "and") == 0) {
                            continue;
                        }
                        if (!is_numeric_string(condition_sentence[j])) {
                            continue;
                        }
                        int comparison_quantity = atoi(condition_sentence[j]);
                        // if there is no subject, it counts as 0
                        if (subject == NULL) {
                            if (comparison_quantity == 0) {
                                continue;
                            }
                            return -1;
                        }
                        Item *item = get_item_of_subject(condition_sentence[j+1], subject);
                        // if there is no item, it counts as 0
                        if (item == NULL) {
                            if (comparison_quantity == 0) {
                                continue;
                            }
                            return -1; 
                        }
                        if (item->quantity != comparison_quantity) {
                            // if subject has different amount of, return -1
                            return -1;
                        }
                        if (j < word_count - 2) {
                            j++;
                        }
                    }
                } 
                //if everything is fine, return 0
                return 0;
            }
            return 0; // this may be unneccesary
        }
    }
    // if it is not a condition sentence, return -1
    return -1;
}

// answer the questions according to question word
int answer_question(char **tokens, int token_count) {
    // assign start index to first word of the input
    int start_index = 0;
    int index;
    index = get_questionword_index(tokens, token_count, start_index);

    if (index != -1) {

        // Subject total? -- Subject(s) total Item?
        if (strcmp(tokens[index], "total") == 0) {
            int item_index = index + 1;

           // check if there is an item or question mark after total

            // Subject total?
            if (strcmp(tokens[item_index], "?") == 0) {
                
                // if there is more than one word before "total", question is invalid return -1
                if (index != 1) {
                    return -1;
                }
                // if the Subject word is not valid, the question is invalid return -1
                if (!is_valid_word(tokens[0])) {
                    return -1;
                }

                // if everything is fine, print the items and return
                if (print_all_items(tokens[index - 1]) == -1) {
                    return -1;
                }
                return 0;
            } 

            // Subject(s) total Item?

            // if the item word is not valid, return -1
            if (!is_valid_word(tokens[item_index])) {
                return -1;
            }
            // if the word after item is not a question mark or there are more than one words after "total", then there is an invalidation, return -1
            if (strcmp(tokens[item_index + 1], "?") != 0 || item_index + 2 != token_count ) {
                return -1;
            }

            // if there is no problem with the right side of "total", continue
            int total = 0; // initialize total amount
            for(int i = 0; i < index; i++) {
                // If the word is "and", not a subject, continue, first and last words cannot be "and"
                if (strcmp(tokens[i], "and") == 0 && i != 0 && i != index - 1) {
                    continue;
                } else {
                    // if there is no "and" between subject names, return -1
                    if (i != 0 && strcmp(tokens[i - 1], "and") != 0) {
                        return -1;
                    }
                    // if the Subject name is not valid, question is invalid return -1
                    if (!is_valid_word(tokens[i])) {
                        return -1;
                    } 
                    // if there are no problems, add the item quantity of subject to total
                    total += get_subject_item_quantity(tokens[i], tokens[item_index]);
                }
            }
            // print the total
            printf("%d\n", total);
            return 0;
        }

        // Subject where?
        if (strcmp(tokens[index], "where") == 0) {
            
            // check possible invalidations, the format should be "Subject where ?", things other than that is not accepted, also check if the subject is a valid word
            if(token_count != 3 || !(is_valid_word(tokens[index - 1])) || index != 1 || strcmp(tokens[index +1], "?") != 0) {
                return -1;
            }
            // print the location if all things fine
            if (print_location(tokens[index - 1]) == -1) {
                return -1;
            }
            return 0;
        }

        // who at Location?
        if (strcmp(tokens[index], "who") == 0) {

            // check possible invalidations, the format should be "Who at Location ?", things other than that is not accepted, also check if the location is a valid word
            if(token_count != 4 || strcmp(tokens[index + 1], "at") != 0 || !(is_valid_word(tokens[index + 2])) || index != 0 || strcmp(tokens[index + 3], "?") != 0) {
                return -1;
            }
            // print the people at location if all things fine
            if (print_people_at(tokens[index + 2]) == -1) {
                return -1;
            }

            return 0;
        }

        // if the question does not involve the word "total", "where", or "who", the question is invalid return -1 (might be unneccesary, check later)
        return -1;
    }

    // if the question does not involve question word, it is invalid so return -1 
    return -1;
}

// Function to free allocated memory for tokens
void free_tokens(char **tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

