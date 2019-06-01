/*************************************************************
* Author:		Joshua Hardy
* Filename:		Lab8.c
* Date Created:	1/6/2018
* Date Modifed: 1/6/2018
**************************************************************/
/*************************************************************
*
* Lab/Assignment: Lab 8
*
* Overview:
*	  Finds prime numbers
* Input:
*	  commandline arguments
* Output:
*   prime numbers to stdout
*   displays known prime numbers from prime.txt along side
*   generated numbers. For easy grading.
* Usage:
*   -t # Number of threads to use. One is the default.
*   -u # Upperbound of numbers to test for prime values. 10240 is the default.
*   -h   Display this help.
*   -v   Verbose mode - Debugging.
************************************************************/
#include <stdio.h>
#include <unistd.h> //getopt()
#include <stdlib.h> //strtoull()
#include <string.h> //memset()
#include <time.h>   //time()
#include <pthread.h>

#define BYTE_TYPE unsigned int

#define GET_INDEX(k)        ( k / ( sizeof(BYTE_TYPE) * 8 ) )
#define GET_BIT_INDEX(k)    (1 <<  (k % ( sizeof(BYTE_TYPE) * 8) ) )

#define TRUE    1
#define FALSE   0

#define PRIME     1
#define NOT_PRIME 0

typedef struct BitBlock_s
{
    BYTE_TYPE bits;
    pthread_mutex_t mutex;
} BitBlock_t;

//local globals
static int verbose_mode = FALSE;
static unsigned long largest_number = 10240;
static BitBlock_t *bits = NULL;
static unsigned long largest_index = 0;
static unsigned long current_prime = 3;
static pthread_mutex_t current_prime_mutex;


void *find_prime_numbers(void *not_used);
void display_help(void);
void display_primes(void);
void display_all_bits(void);
void find_prime_numbers_single_process(unsigned long range);


int main(int argc, char **argv)
{
    char c = 0;
    unsigned int thread_count = 1;
    pthread_t *threads = NULL;
    pthread_attr_t thread_attribute;
    int r_val = 0;
    int i = 0;
    time_t start_time;
    time_t elapsed_time;

    //get commandline arguments
    while ((c = getopt(argc, argv, "t:u:vh")) != -1)
    {
        switch (c)
        {
            case 't':
                thread_count = strtol(optarg, NULL, 10);
                break;
            case 'u':
                largest_number = strtol(optarg, NULL, 10);
                break;
            case 'v':
                verbose_mode = TRUE;
                break;
            case 'h':
            case '?':
            case ':':
                display_help();
                exit(EXIT_SUCCESS);
            break;
        }
    }
    //this is how large the array needs to be
    largest_index = largest_number / (sizeof(BYTE_TYPE) * 8);

    //allocate threads
    threads = malloc(sizeof(pthread_t) * thread_count);

    //allocate BitBlock_t
    bits = malloc(sizeof(BitBlock_t) * largest_index);

    //init default mutex
    for(i = 0; i < largest_index; i++)
    {
        //flip all bits to ones
        bits[i].bits = ~0;
    }
    //begin prime finding
    //start start_time
    start_time = time(0);

    //do one iteration of multiples of 2
    //this way the threads are able to find primes.
    find_prime_numbers_single_process(largest_number);

    pthread_mutex_init(&current_prime_mutex, NULL);

    //start threads
    for(i = 0; i < thread_count; i++)
    {
        pthread_attr_init(&thread_attribute);
        pthread_attr_setdetachstate(&thread_attribute, PTHREAD_CREATE_JOINABLE);

        r_val = pthread_create(&threads[i], &thread_attribute, &find_prime_numbers, NULL);
        pthread_attr_destroy(&thread_attribute);
        //failed to create thread - exit process
        if(r_val != 0)
        {
            perror("Error creating threads.\n");
            break;
        }
    }

    //do thread cleanup
    for(i = 0; i < thread_count; i++)
    {
        if(verbose_mode == TRUE)
            fprintf(stderr, "Waiting on thread: %i\n", i);

        //I was tracking a bug down that was reseting my thread_count
        //I though I'll rename thread_count to xyz - after inspecting
        //every use of it. Still changed the value...
        //turns out passing NULL to retval fixes this.
        //I don't understand!
        //pthread_join(threads[i], (void*) &r_val);
        pthread_join(threads[i], NULL);

        perror(NULL);
    }
    //done priming - get time
    elapsed_time = time(0) - start_time;
    //clean up mutexs
    for(i = 0; i < largest_index; i++)
        pthread_mutex_destroy(&bits[i].mutex);

    pthread_mutex_destroy(&current_prime_mutex);

    if(verbose_mode)
        display_primes();

    //display_all_bits();

    fprintf(stdout, "All threads %i completed.\n", thread_count);
    fprintf(stdout, "\nElapsed time %lu seconds.\n", elapsed_time);


    free(threads);
    free(bits);

    return 0;
}

/**********************************************************************
* Function:         void *find_prime_numbers(void *never_used)
* Purpose:          multithreaded find_prime_numbers
* Precondition:     global variable bits must be allocated
* Postcondition:    prime numbers found
************************************************************************/
void *find_prime_numbers(void *never_used)
{
    unsigned long i = 2;
    unsigned long p = 0;

    //get the starting point
    pthread_mutex_lock(&current_prime_mutex);
    {
        p = current_prime;
        current_prime++;
        while((current_prime < largest_number) && (bits[ GET_INDEX(current_prime) ].bits & GET_BIT_INDEX(current_prime)) <= NOT_PRIME)
            current_prime++;
    }
    pthread_mutex_unlock(&current_prime_mutex);

    while(p * p < largest_number)
    {
        {
            //test all numbers for multiples of i
            //if multiple then cross out of list.
            if((bits[ GET_INDEX(p) ].bits & GET_BIT_INDEX(p)) >= PRIME)
            {
                for( i = p + p; i < largest_number - 1; i += p)
                {
                    if( GET_INDEX(i) < largest_index)
                    {
                        pthread_mutex_lock(&bits[ GET_INDEX(i) ].mutex);

                        //flip that bit to a zero.
                        bits[GET_INDEX(i)].bits &= ~GET_BIT_INDEX(i);
                        pthread_mutex_unlock(&bits[ GET_INDEX(i) ].mutex);
                    }
                    else
                        break;
                }

                //get the new starting point
                pthread_mutex_lock(&current_prime_mutex);
                {
                    p = current_prime;
                    current_prime++;
                    while((current_prime < largest_number) && (bits[ GET_INDEX(current_prime) ].bits & GET_BIT_INDEX(current_prime)) <= NOT_PRIME)
                        current_prime++;
                }
                pthread_mutex_unlock(&current_prime_mutex);
            }
        }
    }
    pthread_exit(NULL);
}

/**********************************************************************
* Function:         void find_prime_numbers_single_process(unsigned long range)
* Purpose:          singlethreaded find prime numbers - primes the threads
* Precondition:     pass range or max value
* Postcondition:    *some prime numbers found starting at factor 2
************************************************************************/
void find_prime_numbers_single_process(unsigned long range)
{
    int p = 4;
    //set zero and 1 to not prime
    bits[0].bits &= ~GET_BIT_INDEX(0);
    bits[0].bits &= ~GET_BIT_INDEX(1);

    //test all numbers for multiples of i
    //if multiple then cross out of list.
    for( ; p < largest_number; p += 2)
    {
        //flip that bit to a zero.
        bits[GET_INDEX(p)].bits &= ~GET_BIT_INDEX(p);
    }
}

/**********************************************************************
* Function:         void display_primes(void)
* Purpose:          displays all prime numbers found in bits array
*                   *Also loads a file and display known prime #s
*                   along side generated numbers of optimus prime.
*                   if primes.txt doesn't exist it just ignores file.
* Precondition:     none
* Postcondition:    all prime numbers displayed along side a file of prime #
************************************************************************/
void display_primes(void)
{
    unsigned long p = 0;
    unsigned long line_count = 0;
    unsigned long number = 1;

    char str_input[30] = { 0 };
    FILE *f_in = NULL;
    f_in = fopen("primes.txt", "r");

    for( ; p < largest_number - 1; p++)
    {
        //test if bit is on
        if((bits[ GET_INDEX(p) ].bits & GET_BIT_INDEX(p)) > 0)
        {
            line_count++;

            if(f_in != NULL)
                fgets(str_input, 30, f_in); //this will only null if eof - printf can handle it.

            str_input[strlen(str_input) - 1] = 0;
            //swap the comments to get the find #
            //fprintf(stdout, "%lu = %.4lu\t File: %s\n", number, p, str_input);
            fprintf(stdout, "%.4lu\t File: %s\n", p, str_input);

            //add newline on screen
            if(line_count > 6)
            {
            //    fprintf(stdout, "\n");
                line_count = 0;
            }
            number++;
        }
    }
    fclose(f_in);
}

/**********************************************************************
* Function:         void display_all_bits(void)
* Purpose:          display all bits of bit struct
* Precondition:     none
* Postcondition:    bit's displayed to stdout
************************************************************************/
void display_all_bits(void)
{
    unsigned long p = 0;
    unsigned long line_count = 0;

    for( ; p < largest_number; p++)
    {
        {
            line_count++;
            fprintf(stdout, "%.4lu = %c\t", p, (bits[ GET_INDEX(p) ].bits & GET_BIT_INDEX(p)) == 0 ? '0' : '1');

            //add newline on screen
            if(line_count > 6)
            {
                fprintf(stdout, "\n");
                line_count = 0;
            }
        }
    }
}

/**********************************************************************
* Function:         void display_help(void)
* Purpose:          display program help
* Precondition:     none
* Postcondition:    help displayed to stdout
************************************************************************/
void display_help(void)
{
    char *str_help = "Optimus Prime Help\n"
                    "-t # \tNumber of threads to use. One is the default.\n"
                    "-u # \tUpperbound of numbers to test for prime values. 10240 is the default.\n"
                    "-h   \tDisplay this help.\n"
                    "-v   \tVerbose mode - Debugging.\n";
    printf("%s", str_help);
}
