#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

/*
    Creates the parent directory if it does not exist
    and generates the sample transaction CSV file if missing.
    Returns 1 on success, 0 on failure.
*/
int generateTransactionCSV(const char* fileName);

#endif
