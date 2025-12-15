
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct 
{
    int id;
    char name[32];
    int retries;
} Job;

Job make_job(int id, const char* name) 
{
    Job j = { .id = id, .retries = 0 };
    snprintf(j.name, sizeof(j.name), "%s", name);
    return j;
}

#define REGISTER_ZLIST_TYPES(X) \
    X(Job, Job)

#define ZLIST_SHORT_NAMES
#include "zlist.h"

int main(void) 
{
    list(Job) queue      = list_init(Job);
    list(Job) quarantine = list_init(Job);

    printf("=> Enqueuing tasks.\n");
    
    list_push_back(&queue, make_job(101, "Resize Images"));
    list_push_back(&queue, make_job(102, "Send Emails"));
    list_push_back(&queue, make_job(103, "Generate PDF"));

    printf("[!] Urgent task received: Database Backup\n");
    list_push_front(&queue, make_job(999, "DB Backup"));

    printf("\n=> Processing queue.\n");
    
    while (!list_is_empty(&queue)) 
    {
        zlist_node_Job* current_node = list_head(&queue);
        Job* j = &current_node->value;

        printf("Processing Job #%d (%s)... ", j->id, j->name);

        if (strcmp(j->name, "Generate PDF") == 0) 
        {
            printf("FAILED!\n");
            
            zlist_node_Job* failed_node = list_detach_node(&queue, current_node);
            
            j->retries++;
            list_push_back(&quarantine, *j); 
            ZLIST_FREE(failed_node); 
        }
        else 
        {
            printf("Done.\n");
            list_pop_front(&queue); 
        }
    }

    printf("\n=> Quarantine review.\n");
    
    if (list_is_empty(&quarantine)) 
    {
        printf("No failed jobs.\n");
    } 
    else 
    {
        list_foreach(&quarantine, it) 
        {
            printf("Quarantined: %s (Retries: %d)\n", it->value.name, it->value.retries);
        }

        printf("\nMoving failed jobs back to main queue for retry...\n");
        list_splice(&queue, &quarantine);
    }
    
    list_clear(&queue);
    list_clear(&quarantine);

    return 0;
}
