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


void update_history()
{
    FILE *history = fopen("./history", "w");

	if(history == NULL)
	{
		perror("Shell:");
		return;
	}
		

    struct node *temp = head;
    while(temp)
	{
		char *withoutnewline = strtok(temp->command, "\n");
        fprintf(history, "%s\n", withoutnewline);
		temp = temp->next;
	}

    fclose(history);
}

void initialize_history()
{

	// printf("1.");

    head = end = NULL;

    FILE *history = fopen("./history", "r");
	
	// printf("2.");

	if(history == NULL)
	{
		perror("Shell:");
		return;
	}

    int c = 0;
    n = 0;

    char command[1000];
    
    while(fgets(command, 1000, history) != NULL)
    {   
        c++;
        pushend(command);
    }

	// printf("3.");

	n = c;
	update_history();

	fclose(history);
}

void add_history(char *command)
{
    if(n == 20)
        popend();
    pushfront(command);
    update_history();
}

void history(char **tokenized_input, int count)
{
	if(count == 1)
	{
		struct node *temp = head;
		int i=0;
		while(temp && i<10)
		{
			printf("\n%s", temp->command);
			temp = temp->next;
			i++;
		}
	}
	else
	{
		count = atoi(tokenized_input[1]);
		if(count == 0)
		{
			printf("\nUsage: history <number of commands to be displayed>");
			return;

		}

		struct node *temp = head;
		int i=0;
		while(temp && i<count)
		{
			printf("\n%s", temp->command);
			temp = temp->next;
			i++;
		}
	}
}