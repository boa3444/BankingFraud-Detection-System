#include "../include/data_generator.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void ensureParentDirectories(const char* fileName)
{
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s", fileName);

    for (char* p = buffer + 1; *p != '\0'; p++)
    {
        if (*p == '/' || *p == '\\')
        {
            char saved = *p;
            *p = '\0';
            if (mkdir(buffer, 0755) != 0 && errno != EEXIST)
            {
                /* Ignore if parent already exists or cannot be created here */
            }
            *p = saved;
        }
    }
}

int generateTransactionCSV(const char* fileName)
{
    if (fileName == NULL || fileName[0] == '\0')
    {
        return 0;
    }

    ensureParentDirectories(fileName);

    FILE* existing = fopen(fileName, "r");
    if (existing != NULL)
    {
        fclose(existing);
        printf("Dataset already exists at: %s\n", fileName);
        return 1;
    }

    FILE* outputFile = fopen(fileName, "w");
    if (outputFile == NULL)
    {
        printf("Error: Could not create dataset file: %s\n", fileName);
        return 0;
    }

    fprintf(outputFile,
            "transactionId,senderAccount,receiverAccount,deviceId,merchantId,"
            "amount,location,description,isFraud,timestamp\n");

    /* Three transactions forming a money-laundering cycle: A1001 -> A1002 -> A1003 -> A1001 */
    fprintf(outputFile, "1,A1001,A1002,D100,M100,25000,Delhi,merchant settlement,1,1718000001000\n");
    fprintf(outputFile, "2,A1002,A1003,D101,M101,24000,Mumbai,online transfer,1,1718000001000\n");
    fprintf(outputFile, "3,A1003,A1001,D102,M102,23000,Dubai,urgent international transfer,1,1718000001000\n");

    /* Normal transactions */
    fprintf(outputFile, "4,A2001,A2002,D200,M200,1200,Delhi,grocery payment,0,1718000001000\n");
    fprintf(outputFile, "5,A3001,A3002,D300,M300,800,Noida,food payment,0,1718000001000\n");
    fprintf(outputFile, "6,A4001,A4002,D400,M400,1500,Pune,electricity payment,0,1718000001000\n");
    fprintf(outputFile, "7,A5001,A5002,D500,M500,2200,Chandigarh,shopping payment,0,1718000001000\n");
    fprintf(outputFile, "8,A6001,A6002,D600,M600,900,Jaipur,regular payment,0,1718000001000\n");

    fclose(outputFile);
    printf("New transaction dataset created at: %s\n", fileName);
    return 1;
}
