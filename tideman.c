#include <cs50.h>
#include <stdio.h>
#include <string.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
    int margin;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
void merge(pair pairs_arr[], int left_ind, int mid, int right_ind); 
void mergesort(pair pairs_arr[], int left_ind, int right_ind);
bool look_for_cycle(int curr_winner, int curr_loser);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("ERROR: Usage: ./tideman [candidates]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }
        
        // Count the number of voters that prefer one candidate over another
        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    
    sort_pairs();
    
    lock_pairs();
    
    print_winner();
    
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcmp(name, candidates[i]) == 0)
        {
            ranks[rank] = i;
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count - 1; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    pair_count = 0;
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            // Update pairs array for each possible pair of candidates, where one candidate is preferred over another
            if (preferences[i][j] > preferences[j][i])
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pairs[pair_count].margin = preferences[i][j] - preferences[j][i];
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    mergesort(pairs, 0, pair_count - 1);
    return;
}

void mergesort(pair pairs_arr[], int left_ind, int right_ind)
{ 
    int mid;
    
    if (left_ind < right_ind) 
    { 
        mid = (left_ind + right_ind) / 2; 
  
        // Recursively sort left and right halves 
        mergesort(pairs_arr, left_ind, mid); 
        mergesort(pairs_arr, mid + 1, right_ind); 
        
        // Merge sorted subarrays
        merge(pairs_arr, left_ind, mid, right_ind); 
    } 
}

void merge(pair pairs_arr[], int left_ind, int mid, int right_ind)
{ 
    int i, j, k; 
    int left_subarr_size = mid - left_ind + 1; //n1
    int right_subarr_size = right_ind - mid; //n2
  
    // Create temp arrays
    pair left[left_subarr_size], right[right_subarr_size]; 
  
    // Copy data to temp arrays
    for (i = 0; i < left_subarr_size; i++) 
    {
        left[i] = pairs_arr[left_ind + i];
    }
    for (j = 0; j < right_subarr_size; j++)
    {
        right[j] = pairs_arr[mid + 1 + j];
    }
  
    // Merge the temp arrays back together
    i = 0; 
    j = 0; 
    k = left_ind; 
    while (i < left_subarr_size && j < right_subarr_size) 
    { 
        if (left[i].margin >= right[j].margin) 
        { 
            pairs_arr[k] = left[i];
            i++; 
        } 
        else
        { 
            pairs_arr[k] = right[j]; 
            j++; 
        } 
        k++; 
    } 
  
    // Copy any leftover elements
    while (i < left_subarr_size)
    { 
        pairs_arr[k] = left[i]; 
        i++; 
        k++; 
    } 
    while (j < right_subarr_size)
    { 
        pairs_arr[k] = right[j];
        j++; 
        k++; 
    } 
} 

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    // First pair in sorted pairs array has largest margin -> lock pair
    locked[pairs[0].winner][pairs[0].loser] = true;
    
    // iterate over remaining elements
    for (int i = 1; i < pair_count; i++)
    {
        bool cycle = look_for_cycle(pairs[i].winner, pairs[i].loser);
        
        if (!cycle)
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    
    return;
}

bool look_for_cycle(int curr_winner, int curr_loser)
{
    bool cycle = false;
    
    for (int i = 0; i < candidate_count; i++)
    {
        // Recursively search for cycle by checking following winner/loser graph edges
        if (locked[i][curr_winner] == true)
        {
            if (i == curr_loser)
            {
                return true;
            }
            else 
            {
                cycle = look_for_cycle(i, curr_loser);
            }
        }
    }
    
    if (cycle)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Print the winner of the election
void print_winner(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        bool winner = true;
        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[j][i])
            {
                winner = false;
            }
        }
        if (winner)
        {
            printf("%s\n", candidates[i]);
        }
    }
    return;
}
