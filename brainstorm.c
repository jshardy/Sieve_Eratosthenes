/*
logic?...
1. get thread count
 number of threads is the number of prime numbers we need to find right away
2. Start each thread with it's own prime numbers
3. Before waiting for each thread
    find thread_count more of prime numbers.
4. Wait for each thread to come back (join)
5. reassign threads and start them again.
6. Goto step 1.
*/


/*
Logic 2
1. find all multiples of 2
    cross them out
2. create threads
    threads have access to mutex/locked global current_prime
3. threads can self-assign/self regulate next prime
4. threads end when prime +p < largest_number
5. end.
*/



for( i = p ; p <= n; i += p)
{
    if( ((i / p) == 0) && ((i % p) == 0) )
    {
                pthread_mutex_lock(&bits[ GET_INDEX(p) ].mutex);
                //flip the bit to 0
                //flip the bitindex so the index is 0 and rest are 1111101111
                bits[ GET_INDEX(i) ].bits &= ~GET_BIT_INDEX(i);
                pthread_mutex_unlock(&bits[ GET_INDEX(p) ].mutex);
    }
    if( (i + p) >= n)
    {
        pthread_mutex_lock(&current_prime_mutex);
        {
            p = current_prime;
            current_prime++;
            while((bits[ GET_INDEX(current_prime) ].bits & GET_BIT_INDEX(current_prime)) == 0)
                current_prime++;
        }
        pthread_mutex_unlock(&current_prime_mutex);
        i = p;
    }
}
