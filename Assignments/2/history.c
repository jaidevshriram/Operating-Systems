#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<fcntl.h>
#define int long long int

struct node{
    char command[1000];
	struct node * next;
	struct node * prev;
};

struct node *head, *end;

int n,k,th;

void pushfront(char *command)
{
	struct node *temp=(struct node *)malloc(sizeof(struct node));
    strcpy(temp->command, command);
	temp->next=head;
	temp->prev=NULL;

	if(head==NULL)
	{
		head=temp;
		end=temp;
		// printf("First Value pushed to queue is: %lld\n", head->a );
	}
	else
	{
		head->prev=temp;
		head=temp;
		// printf("Value pushed to queue is: %lld\n",head->a );
	}
    n++;
}

void pushend(char *command)
{
	struct node *temp=(struct node *)malloc(sizeof(struct node));
    strcpy(temp->command, command);
	temp->next=NULL;

	if(head==NULL)
	{
		temp->prev=NULL;
		head=temp;
		end=temp;
		// printf("First Value pushed to queue is: %lld\n", head->a );
	}
	else
	{
		end->next=temp;
		temp->prev=end;
		end=temp;
		// printf("Value pushed to queue is: %lld\n",head->a );
	}
    n++;
}

void popend()
{
	// printf("DELETED.\n");
	struct node *temp=end;

	//Only one element in queue
	if(end->prev==NULL)
	{
		// printf("ONLY ONE ELEMENT\n");
		end=head=NULL;
	}
	else
	{
		(end->prev)->next=NULL;
		end=end->prev;
		// printf("REMOVE: %lld from end\n", temp->a );
	}

	free(temp);
    n--;
}

void popfront()
{
	struct node *temp=head;
	if(head->next==NULL)
	{
		// printf("NULL ERROR\n");
		head->prev=NULL;
		end->next=end->prev=NULL;
		end=head=NULL;
		return;
	}
	
	head=head->next;
	// printf("head->next->a %lld\n", temp->next->a);
	head->prev=NULL;

	// printf("REMOVE 2.0: %lld from front\n", temp->a );
	free(temp);
	// printf("DELETE.\n");
	if(head==NULL)
		end=NULL;
    n--;
}

void initialize_history()
{

    head = end = NULL;

    FILE *history = fopen("./history", "r");

    int c = 0;
    n = 0;

    char command[1000];
    
    while(fscanf(history, "%[^\n]", command))
    {   
        c++;
        pushfront(command);
    }
	
	fclose(history);
}

void update_history()
{
    FILE *history = fopen("./history", "w");

    struct node *temp = head;
    while(temp)
        fprintf(history, "%s\n", temp->command);

    fclose(history);
}

void add_history(char *command)
{
    if(n == 20)
        popend();
    pushfront(command);
    update_history();
}