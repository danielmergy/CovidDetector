#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SpreaderDetectorParams.h"


#define OUTPUT_FILE_ERR_MSG "Error in output file.\n"
#define INPUT_FILE_ERR_MSG "Error in input files.\n"
#define INPUT_FORMAT_ERR_MSG "Usage: ./SpreaderDetectorBackend<Path to People.in><Path to Meetings.in>\n"
#define REALLOC_CAPACITANCE 10
#define LINE_LENGHT 1024

/**
 * A person with name id age and risk of infection that have been seen in the videos
 */
typedef struct Person
{
    float age, risk;
    unsigned long id;
    char name[LINE_LENGHT];
}Person;

/**
 * Help function to print all the people and verify their attributes
 * @param all sll the people in the system
 */
void printAll(Person* all)
{
    int i = 0;
    while (all[i].id != 0)
    {
        printf("%i : %s %lu %f %f\n", i, all[i].name, all[i].id, all[i].age, all[i].risk);
        i ++;
    }
}

/**
 * Close the files associated to the provided pointers
 * @param f1 first file pointer
 * @param f2 second file pointer
 */
void closeFiles(FILE *f1, FILE *f2)
{
    fclose(f1);
    fclose(f2);
}

/**
 * Close the input files and exit from the program
 * @param PeopleFile fist input file
 * @param MeetingsFile second input file
 */
void safeQuit(FILE* PeopleFile , FILE* MeetingsFile)
{
    closeFiles(PeopleFile, MeetingsFile);
    exit(EXIT_FAILURE);
}

/**
 * Comparator two persons by their id
 * @param member first person
 * @param pivot second person
 * @return 1 if first ID is lower than the second else 0
 */
int compareById(Person member , Person pivot)
{
    return  (member.id < pivot.id);
}

/**
 * Comparator two persons by their risk of infection
 * @param member first person
 * @param pivot second person
 * @return 1 if first risk is lower than the second else 0
 */
int compareByRisk(Person member , Person pivot)
{
    return  (member.risk < pivot.risk);
}

/**
 * Permute two persons positions in the data structure
 * @param AllPeople the data struture
 * @param i the index of the first person in the data struture
 * @param j the index of the second person in the data struture
 */
void permute(struct Person AllPeople[] , int i , int j)
{
    Person temp = AllPeople[i];
    memcpy(&AllPeople[i], &AllPeople[j], sizeof(Person));
    memcpy(&AllPeople[j], &temp, sizeof(Person));
}
/**
 * Partition Help Function of Quicksort
 * @param arr the list that have to be sorted
 * @param first_index begining of the new array
 * @param last_index end of the new array
 * @param mode 0 if we compare by ID and 1 if we compare by risk
 * @return the index needed by quicksort algorithm
 */
int partition (struct Person arr[] , int first_index , int last_index, int mode)
{
    int store_index = first_index;
    struct Person pivot = arr[first_index];

    int i;
    for (i = first_index + 1; i <= last_index; i ++)
    {
        int flag;
        if (!mode)
        {
            flag = compareById(arr[i], pivot);
        }
        else
        {
            flag = compareByRisk(arr[i], pivot);
        }

        if (flag)
        {
            store_index ++;
            permute(arr, store_index, i);
        }
    }
    permute(arr, store_index, first_index);
    return store_index;
}

/**
 * Quicksort Recursive sorting algorithm in nLogn
 * @param AllPeople the list that have to be sorted by ID or by risk
 * @param first_index the begiinging of the array to be sorted
 * @param last_index the end of the array to be sorted
 * @param mode 0 if we compare by ID and 1 if we compare by risk
 */
void quickSort( Person* AllPeople, int first_index , int last_index , int mode)
{
    if (first_index < last_index)
    {
        int pivot;
        pivot = partition(AllPeople, first_index, last_index , mode);
        quickSort(AllPeople, first_index, pivot, mode );
        quickSort(AllPeople, pivot + 1, last_index, mode );
    }
}

/**
 * Binary search by ID in the data structure in nLogn
 * @param arr the data sturture
 * @param l for lest index
 * @param r for right index
 * @param ID the value we look for
 * @return the index of the person corresponding to the provided ID in the data structure
 */
int binarySearch(Person *arr, int l, int r, unsigned long ID)
{
    while (l <= r)
    {
        int m = l + ( (r - l) / 2 );
        if (arr[m].id == ID )
        {
            return m;
        }
        if (arr[m].id < ID)
        {
            l = m + 1;
        }
        else
        {
            r = m - 1;
        }
    }
    return -1;
}

/**
 * Compute the probability of a person to be infected by its contact with a potential sick
 * @param distance the distance they have been close
 * @param time the time of the contact
 * @return this probability
 */
float crna(float distance, float time)
{
    return (time * MIN_DISTANCE) / ( distance * MAX_TIME);
}

/**
 * Open the output file and properly exit if it fails
 * @param PeopleFile first file to close if we exit
 * @param MeetingsFile second file to close if we exit
 * @param Allpeople Data structure to free if we exit
 * @return pointer associated to the opened file if sucess
 */
FILE* openOutputFile(FILE* PeopleFile , FILE* MeetingsFile, Person **Allpeople )
{
    FILE *fp_output;
    fp_output = fopen(OUTPUT_FILE, "w");
    if (fp_output == NULL)
    {
        fprintf(stderr, OUTPUT_FILE_ERR_MSG);
        free(*Allpeople);
        fclose(fp_output);
        safeQuit(PeopleFile, MeetingsFile);
    }
    return fp_output;

}

/**
 * Print all the people in the system by the end of the data structure with the messsage correlated to their risk
 * @param AllPeople the data structure
 * @param i number of persons in the system
 * @param fp_output pointer to the file we will write
 */
void printOutputFile(Person* AllPeople , int i, FILE* fp_output)
{
    for (int k = i ; k > 0 ; k --)
    {
        char Buffer[50];
        if ((AllPeople[k].risk) >= (MEDICAL_SUPERVISION_THRESHOLD))
        {
            strcpy(Buffer, MEDICAL_SUPERVISION_THRESHOLD_MSG);
        }
        else if ((AllPeople[k].risk) >= (REGULAR_QUARANTINE_THRESHOLD))
        {
            strcpy(Buffer, REGULAR_QUARANTINE_MSG);
        }
        else
        {
            strcpy(Buffer, CLEAN_MSG);
        }
        fprintf(fp_output, Buffer, AllPeople[k].name, AllPeople[k].id);
    }
}

/**
 * Extract data from Meetig and directly update the risk of the concerned persons in the data structure
 * @param AllPeople the data structure
 * @param i number of persons in the system
 * @param MeetingsFile the file we wan to extract the data
 */
void readerMeetingAndAlgo(Person** AllPeople, int i, FILE* MeetingsFile )
{
    char line2[LINE_LENGHT];
    int l = 0;
    while (fgets(line2, LINE_LENGHT, MeetingsFile) != NULL)
    {
        float risk = 0;
        unsigned long infector = 0;
        if (l == 0)
        {
            char * pEnd1;
            infector = strtol(line2, &pEnd1, 10);
            int result = binarySearch(*AllPeople, 0, i - 1, infector);
            (*AllPeople)[result].risk = 1;
            risk = 1;
        }
        else
        {
            unsigned long ID1, ID2;
            float distance = 0, time = 0;
            sscanf(line2, "%lu %lu %f %f", &ID1 , &ID2 , &distance , &time );
            if (ID1 != infector)
            {
                infector = ID1;
                int result = binarySearch(*AllPeople, 0, i - 1, ID1);
                risk = (*AllPeople)[result].risk;
            }
            int result = binarySearch(*AllPeople, 0, i - 1, ID2);
            (*AllPeople)[result].risk = crna(distance, time)*risk;
        }
        l ++;
    } // while meetings
}

/**
 * Extract the data in the file People to update the data structure with new persons
 * @param PeopleFile The file we want ot extract the data
 * @param MeetingsFile  The second input file that have to be closed if we need to exit the system
 * @param AllPeople the initialized main data structure
 * @return the total number of persons in the system
 */
int readerPeople(FILE* PeopleFile , FILE* MeetingsFile, Person ** AllPeople)
{
    int i = 0; // counter of people
    int k = 0; // counter of dozens to realloc
    int j = 1; // multiply by ten to ge the realloc size
    char line1[LINE_LENGHT];

    while (fgets(line1, LINE_LENGHT, PeopleFile) != NULL)
    {
        sscanf(line1, "%s %lu %f", (*AllPeople)[i].name , &(*AllPeople)[i].id, &(*AllPeople)[i].age );
        i ++, k ++;
        if (k == (REALLOC_CAPACITANCE - 1) )
        {
            k = 0;
            j ++;
            *AllPeople = (Person *)realloc(*AllPeople, j * REALLOC_CAPACITANCE * sizeof(Person));
            if (*AllPeople == NULL)
            {
                fprintf(stderr, STANDARD_LIB_ERR_MSG);
                closeFiles(PeopleFile, MeetingsFile);
                free(*AllPeople);
                exit(EXIT_FAILURE);
            }
        }
    }
    return i;
}

/**
 * Extract the data from the input , compute and write into the output file
 * @param PeopleFile first input file
 * @param MeetingsFile second input file
 * @param AllPeople  the initialized main data structure
 */
void spreaderDetectorBackend(FILE* PeopleFile , FILE* MeetingsFile, Person ** AllPeople)
{
    int i = readerPeople(PeopleFile , MeetingsFile, AllPeople);  //gets input data in people
    quickSort(*AllPeople, 0, i - 1, 0);  // sort by ID
    readerMeetingAndAlgo(AllPeople, i, MeetingsFile);  // gets input data in Meeting and complete risk in data structure
    quickSort(*AllPeople, 0, i, 1);  //sort by risk
    FILE* fp_output = openOutputFile(PeopleFile, MeetingsFile, AllPeople);
    printOutputFile(*AllPeople, i, fp_output);
    fclose(fp_output);
}

/**
 * Detect if the input files have been correctly opened
 * @param fp1 first input file
 * @param fp2 second input file
 */
void errorInFiles(FILE* fp1, FILE* fp2)
{
    if (fp1 == NULL)
    {
        fclose(fp1);
        fprintf(stderr, INPUT_FILE_ERR_MSG);
        exit(EXIT_FAILURE);
    }
    if (fp2 == NULL)
    {
        closeFiles(fp1, fp2);
        fprintf(stderr, INPUT_FILE_ERR_MSG);
        exit(EXIT_FAILURE);
    }

}

/**
 * Detect if he program recieve the good number of arguments
 * @param argc number of arguments provided to the program
 */
void checkArgs(int argc)
{
    if (argc != 3)
    {
        fprintf( stderr, INPUT_FORMAT_ERR_MSG);
        exit(EXIT_FAILURE);
    }
}

/**
 * Check if the memory allocution on the heap is succesfull
 * @param persons the pointer to the memory block we want to allocate
 * @param PeopleFile file we want to close if the allocution fail
 * @param MeetingsFile file we want to close if the allocution fail
 */
void checkAlloc(Person* persons, FILE* PeopleFile , FILE* MeetingsFile)
{
    if (persons == NULL)
    {
        fprintf(stderr, STANDARD_LIB_ERR_MSG);
        free(persons);
        safeQuit(PeopleFile, MeetingsFile);
    }
}

/**
 * Msin driver of the program recives program arguments check them create the data struture and launch the main process
 * @param argc number of argument recieved by the program
 * @param argv array of provided arguments
 * @return 0 if  program sucess else 1
 */
int main(int argc , char *argv[])
{
    checkArgs(argc);
    FILE *PeopleFile =  fopen(argv[1], "r");
    FILE *MeetingsFile = fopen(argv[2], "r");
    errorInFiles(PeopleFile, MeetingsFile);
    Person* persons = (Person *)malloc(REALLOC_CAPACITANCE * sizeof(Person));
    checkAlloc(persons, PeopleFile, MeetingsFile);
    spreaderDetectorBackend(PeopleFile, MeetingsFile, &persons);
    free(persons);
    closeFiles(PeopleFile, MeetingsFile);
    return 0;
}
