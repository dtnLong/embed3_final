#include "../include/uart.h"

int str_len(char *str) {
    /*
        Find length of a given string. Return length of string
    */
    int str_lenght = 0;
    for (int i = 0; *(str + i) != '\0'; i++) {
        str_lenght++;
    }
    return str_lenght;
}

void str_concat(char *str1, char *str2) {
    /*
        Concat str2 into str1, the result is stored in str1
    */
	int str1_length = str_len(str1);
    for (int str2_i = 0; *(str2 + str2_i) != '\0'; str2_i++, str1_length++) {
        *(str1 + str1_length) = *(str2 + str2_i);
    }
    *(str1 + str1_length) = '\0';
}

void str_concat_char(char *str, char c) {
    /*
        Concat char c into str1, the result is stored in str1
    */
    int str_length = str_len(str);
    *(str + str_length) = c;
    *(str + str_length + 1) = '\0';
}

int str_compare(char *str1, char *str2) {
    /*
        Compare 2 string. Return 1 if equal, 0 if not equal
    */
    int str1_length = str_len(str1);
    int str2_length = str_len(str2);
    if (str1_length != str2_length) {
        return 0;
    }
    for (int i = 0; i < str1_length; i++) {
        if (*(str1 + i) != *(str2 + i)) {
            return 0;
        }
    }
    return 1;
}

void str_copy(char *str1, char *str2) {
    /*
        Copy str2 into str1
    */
    int str2_lenght = str_len(str2);
    for (int i = 0; i <= str2_lenght; i++) {
        *(str1 + i) = *(str2 + i);
    }
}

void str_split(char (*token_arr)[50], char *str, char delimiter) {
    /*
        Split a string based on char delemiter. Result is an array of string store in array passed in
    */
    int str_lenght = str_len(str);
    char current_str[50] = "";
    char token_amount = 0;
    
    for (int i = 0; i < str_lenght; i++) {
        if (*(str + i) == delimiter) {
            str_copy(*(token_arr + token_amount), current_str);
            token_amount++;
            current_str[0] = '\0';
        } else if (i == str_lenght - 1 ) {
            str_concat_char(current_str, *(str + i));
            str_copy(*(token_arr + token_amount), current_str);
        } else {
            str_concat_char(current_str, *(str + i));
        }
    }
}

void str_trim_last_char(char *str) {
    int str_lenght = str_len(str);
    *(str + str_lenght - 1) = '\0';
}

int str_to_int(char *str) {
	int num = 0;
	for (int i = 0; *(str + i) != '\0'; i++) {
		num *= 10;
		num += *(str + i) - '0';
	}
	return num;
}

int str_is_int(char *str) {
    if (str_len(str) == 0) {
        return 0;
    }
    
    if ((*str < '1' || *str > '9') && str_len(str) > 0) {
        return 0;
    }
    for (int i = 1; i < str_len(str); i++) {
        if (*(str + i) < '0' || *(str + i) > '9') {
            return 0;
        }
    }
    return 1;
}