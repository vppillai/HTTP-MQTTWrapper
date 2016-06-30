#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 20
static unsigned char debug=0;


typedef struct queryNode queryNode;

struct queryNode{
  char* key;
  char* value;
  queryNode* next;
};

static unsigned int get_thing_id(char* thingID);
static unsigned int get_query_count(void);
static unsigned int allocate_query_nodes(queryNode* queryNodeHead);
static void free_query_node(queryNode* queryNodeHead);

int main()
{
  char thingID[MAX_ID_LEN]; /*eg: PIC32WK-B16B00*/

  /*debug and test*/
  if (NULL!=getenv("DEBUG")){debug=1;}
  if(0!=debug){if(!get_thing_id(thingID)) printf("thingID: %s\n",thingID);}
  if(0!=debug){printf("queryCount: %d\n",get_query_count());}

  queryNode queryNodeHead;
  queryNodeHead.next=NULL;
  allocate_query_nodes(&queryNodeHead);
  free_query_node(&queryNodeHead);

  return 0;
}


/*extract thing ID from requestURI*/
static unsigned int get_thing_id(char* thingID)
{
  char* requestURI=getenv("REQUEST_URI");
  if (NULL!=requestURI){
    strncpy(thingID,strtok((strrchr(requestURI,'/')+1),"?"),MAX_ID_LEN);
    return 0;
  }
  else{
    return -1;
  }
}


/*find number of queries*/
static unsigned int get_query_count(void)
{
  char* query=getenv("QUERY_STRING");
  int count=0;
  if (NULL!=query){
    unsigned int queryLength=strlen(query);
    for(int i=0;i<=queryLength;i++){
      if ('='==query[i]) count++;
    }
  }
  return count;
}


/*create a linked list to store the queries*/
static unsigned int allocate_query_nodes(queryNode* queryNodeHead)
{
  int count = get_query_count();
  if(NULL!=queryNodeHead->next){
    return -1; /*already initialized*/
  }
  else{
    queryNode* newNode;
    while(count>0){
      newNode=(queryNode*)calloc(1,sizeof(queryNode));
      if(NULL!=newNode){
        queryNodeHead->next=newNode;
        queryNodeHead=newNode;
        count--;
      }
      else{
        return -2; /*calloc failed*/
      }
    }
    return 0;
  }
}


/*free the query nodes LL*/
static void free_query_node(queryNode* queryNodeHead)
{
  queryNode* queryNodeHandle=queryNodeHead->next;
  queryNode* queryNodeTemp;
  while(NULL!=queryNodeHandle){
    queryNodeTemp=queryNodeHandle->next;
    free(queryNodeHandle);
    queryNodeHandle=queryNodeTemp;
  }
  free(queryNodeHandle);
  queryNodeHead->next=NULL;
}

