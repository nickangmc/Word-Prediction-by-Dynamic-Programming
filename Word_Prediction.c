/* * * * * * *
 * Codes written for Assignment 2
 * COMP20007 Design of Algorithms 2018
 *
 * by Ang Mink Chen (Nick)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spell.h"
#include "hashtbl.h"

#define HASH_TABLE_SIZE_CONSTANT 1.5 /* Multiply the hash table size by 1.5 */

/****************************************************************/

/* Function prototypes */

char *append(char *str, char c);
int substitution_cost(char char1, char char2);
int levenshtein_dist(char *word1, char *word2);
int min(int substitution, int deletion, int insertion);
void print_edits_list(List* lists);
void free_temp_node(Node *temp_node);
void deletion(char *word, List *edits);
void insertion(char *word, List *edits);
void substitution(char *word, List *edits);
void fill_hash_table(HashTable *table, List *dictionary);
void extract_prediction(List *dictionary, List *extractions, List *predictions);
bool find_prediction(int edit_dist, char *word, List *dictionary,
					 									List *predictions);
bool find_prediction_dist1(HashTable *table, List *extractions,
		 									List *edits_dist1, char *word);

/****************************************************************/

/* Assignment Task 1: Computing edit distance */
void print_edit_distance(char *word1, char *word2) {

	// Print out the levenshtein distance between the 1st and 2nd words
	// using levenshtein_dist function
	printf("%d\n", levenshtein_dist(word1, word2));

}

/* Assignment Task 2: Enumerating all possible edits */
void print_all_edits(char *word) {

	// Create a new linked-list to store all the edited words
	List *edits = new_list();

	// Finding all edited words with levenshtein distance of 1
	// by applying methods of deletion, subsitution, and insertion
	// to the user input word
	deletion(word, edits);
	substitution(word, edits);
	insertion(word, edits);

	// Print out the edited words
	print_edits_list(edits);

	// Free the linked-list
	free_list(edits);

}

/* Assignment Task 3: Spell checking */
void print_checked(List *dictionary, List *document) {

	// Create a new hash table to store all the words in the dictionary
	HashTable *table = new_hash_table(dictionary->size *
									  HASH_TABLE_SIZE_CONSTANT);

	// Fill the hash table by hashing all the words in the dictionary into it
	fill_hash_table(table, dictionary);

	// Traverse through the document linked-list word by word to see if
	// the word is in dictionary hash-table
	// Create a temporarily node to store the first data/information
	Node *temp_node = document->head;
	// Traverse through the document linked-list
	while (temp_node != NULL) {
		char *word = temp_node->data;
		// Print out the word if it is in the dictionary
		if (hash_table_has(table, word)) {
			printf("%s\n", word);
		// Print out the word with a "?" at the end otherwise
		} else {
			printf("%s?\n", word);
		}
		// Move onto next word
		temp_node = temp_node->next;
	}
	// Free the temp_node
	free_temp_node(temp_node);

	// Free up the hash table
	free_hash_table(table);

}

/* Assignment Task 4: Spelling correction */
void print_corrected(List *dictionary, List *document) {

	// Create a new hash table to store all the words in the dictionary
	HashTable *table = new_hash_table(dictionary->size *
									  HASH_TABLE_SIZE_CONSTANT);

	// Fill the hash table by hashing all the words in the dictionary into it
	fill_hash_table(table, dictionary);

	// Create a linked-list to store all the corrections/predictions of the
	// words in document linked-list
	List *predictions = new_list();

	// Create a linked-list to store all the possible edits of a word
	// with levenshtein distance of 1
	List *edits_dist1 = new_list();
	// Create a linked-list to store the possible edits that are in the
	// dictionary
	List *extractions = new_list();

	// Traverse through the document linked-list word by word to find each
	// correction/prediction accordingly
	// All corrections are to be stored in the "predictions" linked-list
	Node *temp_node = document->head;

	while (temp_node != NULL) {
		char *word = temp_node->data;

		// Conditional variables to control the logic of the loop to reduce
		// unwanted/duplicate calculations. For instance:
		// If correction with levenshtein distance(LD) of 1 is found, then
		// calculations to find correction with LD of 2 and 3 will be skipped.
		// If correction with levenshtein distance(LD) of 1 is not found, then
		// calculations to find correction with LD of 2 will not be skipped.
		bool has_found_dist1 = false;
		bool has_found_dist2 = false;
		bool has_found_dist3 = false;

		// If the word is in dictionary, just print it out directly
		if (hash_table_has(table, word)) {
			list_add_end(predictions, word);
		// Else find its correction/prediction
		} else {

			// * Finding correction of a word with levenshtein distance of 1 *
			// It first finds all possible edits of a word with levenshtein
			// distance of 1, and only extracts those that are in the
			// dictionary, then retains only the correction that occurs first
			// among them in dictionary.
			has_found_dist1 =
				find_prediction_dist1(table, extractions, edits_dist1, word);

			if (has_found_dist1) {
				extract_prediction(dictionary, extractions, predictions);
			}

			//* Finding correction of a word with levenshtein distance of 2/3 *
			// For each word in document linked-list, its levenshtein distances
			// with selected words in the dictionary will be calculated and
			// the first correction that occurs in the dictionary with
			// levenshtein distance of 2/3 will be extracted accordingly
			if (!has_found_dist1) {
				has_found_dist2 =
					find_prediction(2, word, dictionary, predictions);
			}
			if ((!has_found_dist1) && (!has_found_dist2)) {
				has_found_dist3 =
					find_prediction(3, word, dictionary, predictions);
			}

			// If no correction can be found, append a "?" to the word
			// at the end
			if ((!has_found_dist1) & (!has_found_dist2) & (!has_found_dist3)) {
				char *no_prediction = append(word, '?');
				list_add_end(predictions, no_prediction);
			}
		}
		// Move onto next word
		temp_node = temp_node->next;
	}
	// Free the temp_node
	free_temp_node(temp_node);

	// Print out all the corrections/predictions
	print_edits_list(predictions);

	// Free all the linked-lists created and used
	free_list(edits_dist1);
	free_list(extractions);
	free_list(predictions);

	// Free up the hash table
	free_hash_table(table);

}

/****************************************************************/

/* Assignment Task 1 Helper functions */

// Finds and returns the levenshtein distance between two words
// Codes copied and modified from the lecture of COMP20007 Unimelb 2018
int levenshtein_dist(char *word1, char *word2){

	// Varibles that store lengths of words
	int n = strlen(word1);
	int m = strlen(word2);

	// Create an int type 2d-array with n+1 rows and m+1 columns
	int e[n+1][m+1];

	// Fill up the known values
	for (int i=0; i<=n; i++) {
		e[i][0] = i;
	}
	for (int j=1; j<=m; j++) {
		e[0][j] = j;
	}

	// Calculate cost for each edit of letter of the word accordingly
	for (int i=1; i<=n; i++){
		for (int j=1; j<=m; j++){
			e[i][j] = min(
				e[i-1][j-1] + substitution_cost(word1[i-1], word2[j-1]),
				e[i-1][j] + 1,
				e[i][j-1] + 1
			);
		}
	}

	// Return the most bottom right value of the 2d-array which is the
	// lowest costs it takes to edit word1 to be word2
	return e[n][m];

}

// Finds and returns the minimum int value among the three int varibles
int min(int substitution, int deletion, int insertion) {

	int min = substitution;

	if (min > deletion) {
		min = deletion;
	}
	if (min > insertion) {
		min = insertion;
	}
	return min;

}

// Finds and returns the cost to subsitute char1 with char 2
// Return 0 if char1 equals char2, 1 otherwise
int substitution_cost(char char1, char char2) {

	int cost = 1;
	if (char1 == char2) {
		cost = 0;
	}
	return cost;

}

/****************************************************************/

/* Assignment Task 2 Helper Functions */

// Find all the possible edits with levenshtein distance of 1 of the inputted
// word by deletion method
// Add all the edits found to the inputted linked-list
void deletion(char *word, List *edits) {

	// For each letter in the word
	for (int i=0; i<strlen(word); i++) {
		// Variable to store the appropriate letters
		char *str = "";
		// Loop through each letter of the word again
		for (int j=0; j<strlen(word); j++) {
			// If the index of the letter in the first loop and in the second
			// loop are not the same, append the letter to the char* variable
			// skip otherwise
			if (j != i) {
				str = append(str, word[j]);
			}
		}
		// Add the edit found to the inputted linked-list
		list_add_end(edits, str);
	}

}

// Find all the possible edits with levenshtein distance of 1 of the inputted
// word by substitution method
// Add all the edits found to the inputted linked-list
void substitution(char *word, List *edits) {

	// For each letter in the word
	for (int i=0; i<strlen(word); i++) {
		// Substitute each letter in the word 26 times with alphabets from
		// 'a'-'z'
		for (char letter='a'; letter<='z'; letter++) {
			char *str = "";
			for (int j=0; j<strlen(word); j++) {
				if (j != i) {
					str = append(str, word[j]);
				} else {
					str = append(str, letter);
				}
			}
			// Add the edit found to the inputted linked-list
			list_add_end(edits, str);
		}
	}

}

// Find all the possible edits with levenshtein distance of 1 of the inputted
// word by insertion method
// Add all the edits found to the inputted linked-list
void insertion(char *word, List *edits) {

	// For each letter in the word
	for (int i=0; i<strlen(word)+1; i++) {
		// Insert all alphabets from 'a'-'z' into the word at the front,
		// in-between each two-letters, and at the end of the word.
		for (char letter='a'; letter<='z'; letter++) {
			char *str = "";
			int index = 0;
			for (int j=0; j<strlen(word)+1; j++) {
				if (j != i) {
					str = append(str, word[index]);
					index++;
				} else {
					str = append(str, letter);
				}
			}
			// Add the edit found to the inputted linked-list
			list_add_end(edits, str);
		}
	}

}

// Print out all data/information stored in the inputted linked-list
void print_edits_list(List* lists) {

	// Traverse through each node in the linked-list
	Node *temp_node = lists->head;

	while (temp_node != NULL) {
		char *word = temp_node->data;
		// Print out the data/information
		printf("%s\n", word);
		temp_node = temp_node->next;
	}
	// Free the temp node
	free_temp_node(temp_node);

}

/****************************************************************/

/* Assignment Task 3 Helper Functions */

// Simple memory freeing codes
void free_temp_node(Node *temp_node) {

	temp_node = NULL;
	free(temp_node);

}

// Hash each data/information of the inputted linked-list and store the
// hashed value in the inputted hash table accordingly
void fill_hash_table(HashTable *table, List *dictionary) {

	// Create a temporarily node to store the first data/information
	Node* temp_node = dictionary->head;
	// Traverse through the data/information one by one
	while (temp_node != NULL) {
		// Variable with more human-readable name to store the data
		char *word = temp_node->data;

		// If the hashed value is not in the hash table, store the data
		// into the hash table with occurence value of 1 (First occurence)
		if ( ! hash_table_has(table, word)) {
			hash_table_put(table, word, 1);

		// Else store the data into the hash table with occurence value of
		// its current occurence value + 1
		} else {
			int count = hash_table_get(table, word);
			hash_table_put(table, word, count+1);
		}
		// Move onto next node
		temp_node = temp_node->next;
	}

	// Free up temp node
	free_temp_node(temp_node);

}

/****************************************************************/

/* Assignment Task 4 Helper Functions */

// Append the inputted character c to the string inputted at the end
// Returns the concatenated string
char *append(char *str, char c) {

	// Get the length of the string inputted
	size_t len = strlen(str);

	// Malloc the new string variable with size of length + 1 + 1
	// Add one for extra char, one for trailing zero
	char *str2 = malloc(len + 1 + 1 );

	// Copy the inputted string to the new string
    strcpy(str2, str);

	// Assign character c to the last letter
    str2[len] = c;

	// Add in the null terminator
    str2[len + 1] = '\0';

	// Return the new string
	return str2;

}

// Find correction of a word with levenshtein distance of 1
bool find_prediction_dist1(HashTable *table, List *extractions,
							List *edits_dist1, char *word) {

	// Variable to control the logic of the function:
	// If any edits of the inputted word can be found in the dictionary
	// return true, false otherwise
	bool has_found_edits = false;

	// Find all possible edits of the inputted word with levenshtein
	// distance of 1 using the three methods
	deletion(word, edits_dist1);
	substitution(word, edits_dist1);
	insertion(word, edits_dist1);

	// A temporarily node to store the first edit found
	Node *temp_node = edits_dist1->head;
	// Traverse through all the edits found
	while (temp_node != NULL) {

		// If the edit can be found in the dictionary hash table, add the edit
		// to the extractions linked-list, and makes has_found_edits true
		if (hash_table_has(table, temp_node->data)){
			list_add_end(extractions, temp_node->data);
			has_found_edits = true;
		}

		// Move onto the next edit
		temp_node = temp_node->next;
	}
	// Remove all edits found that are stored in the edits_dist1 linked-list
	// to prepare to store the edits of the next inputted word
	while (edits_dist1->size > 0) {
		list_remove_end(edits_dist1);
	}

	// Free up the temp node
	free_temp_node(temp_node);

	// Return the has_found_edits with true or false value
	return has_found_edits;

}

// Extracts the edits from the extractions linked-list that are present in the
// dictionary linked-list, then retains only the correction/edit that occurs
// first among them in dictionary.
void extract_prediction(List *dictionary, List *extractions, List *predictions){

	// Conditional variable to control when to stop the loop
	bool has_extracted = false;

	// Temporarily nodes to store the first edits and the first word in the
	// extractions and dictionary respectively
	Node *temp1 = extractions->head;
	Node *temp2 = dictionary->head;
	// Traverse through each word in the dictionary linked-list
	while (temp2 != NULL) {

		// For each word in dictionary, all edits will be compared to it so
		// that the edit that occur first in the dictionary will be retained
		temp1 = extractions->head;
		// Traverse through each edit
		while (temp1 != NULL) {

			// Compare the word and the edit
			if (strcmp(temp2->data, temp1->data) == 0) {
				// Add the edit into the "prediction" linked-list(List that
				// stores final correction for every word in document) if they
				// are equal
				list_add_end(predictions, temp1->data);
				has_extracted = true;
				break;
			}
			// Move onto the next edit
			temp1 = temp1->next;
		}
		// Move onto the next word
		temp2 = temp2->next;
		// If the desired edit has been extracted, stop the loop
		if (has_extracted) {
			temp1 = NULL;
			temp2 = NULL;
		}
	}
	// Free up both temp nodes
	free(temp1);
	free(temp2);

	// Remove all edits found that are stored in the extractions linked-list
	// to prepare to store the new edits
	while (extractions->size > 0) {
		list_remove_end(extractions);
	}

}

// Find correction of a word with levenshtein distance(LD) of 2/3 from the words
// in dictionary
// Return true if correction with desired LD value can be found, false otherwise
// Note: first parameter(int edit_dist) is the desired LD value the user wishes
//       to use in the LD calculation
bool find_prediction(int edit_dist, char *word, List *dictionary,
					 List *predictions) {

	// Traverse through each word in the dictionary linked-list
	Node *temp_node = dictionary->head;
	while (temp_node != NULL) {
		// A more human-readable variable name is created to represents
		// the word that is currently being used in the calculation
		char *word_in_dict = temp_node->data;

	   // Only the words in the dictionary that have the length between the
	   // length of the inputted word +- the levenshtein distance desired
	   // (a parameter of the function) will be used to calculate its
	   // levenshtein distance with the inputted word
	   // Aims to remove unnecessary redundant calculations
		if (strlen(word_in_dict) <= (strlen(word) + edit_dist) &&
			strlen(word_in_dict) >= (strlen(word) - edit_dist)) {
				if (levenshtein_dist(word_in_dict, word) == edit_dist) {

					// Add the word to the "prediction" linked-list if
					// its LD equals to the desired LD value
					list_add_end(predictions, word_in_dict);
					// Free up temp node
					free_temp_node(temp_node);
					// Return true as it means the first correction among the
					// words in dictionary has been found, hence no need to
					// continue
					return true;
				}
		}
		// Move onto the next word
		temp_node = temp_node->next;
	}

	// Free up temp node
	free_temp_node(temp_node);

	// Return false as it means no correction with LD value of 2 can be found
	return false;

}

/****************************************************************/

/* End of the codes */
