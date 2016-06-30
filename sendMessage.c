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

static int get_thing_id(char* thingID);
static int get_query_count(void);
static int allocate_query_nodes(queryNode* queryNodeHead);
static void free_query_node(queryNode* queryNodeHead);
static int parse_query_string(queryNode* queryNodeHead); 
static void traverse_query_string(queryNode* queryNodeHead);
static int mqtt_pub(char* thingID,queryNode* queryNodeHead);

int main()
{
  char thingID[MAX_ID_LEN]; /*eg: CIP23KW-B16B00*/

  printf("Content-type: text/html\n\n");

  /*debug and test*/
  if (NULL!=getenv("DEBUG")){debug=1;}
  if(0!=debug){if(!get_thing_id(thingID)) printf("thingID: %s\n",thingID);}
  if(0!=debug){printf("queryCount: %d\n",get_query_count());}

  queryNode queryNodeHead;
  queryNodeHead.next=NULL;
  allocate_query_nodes(&queryNodeHead);

  parse_query_string(&queryNodeHead);
  traverse_query_string(&queryNodeHead);

  get_thing_id(thingID);
  mqtt_pub(thingID,&queryNodeHead);
  free_query_node(&queryNodeHead);
  return 0;
}


/*extract thing ID from requestURI*/
static int get_thing_id(char* thingID)
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
static int get_query_count(void)
{
  char* query=getenv("QUERY_STRING");
  int queryCount=0;

  if (NULL!=query){
    unsigned int queryLength=strlen(query);
    for(int i=0;i<=queryLength;i++){
      if ('='==query[i]) queryCount++;
    }
  }
  return queryCount;
}


/*create a linked list to store the queries*/
static int allocate_query_nodes(queryNode* queryNodeHead)
{
  int queryCount = get_query_count()-1;

  if(NULL!=queryNodeHead->next){
    return -1; /*already initialized*/
  }
  else{
    queryNode* newNode;
    while(queryCount>0){
      newNode=(queryNode*)calloc(1,sizeof(queryNode));
      if(NULL!=newNode){
        queryNodeHead->next=newNode;
        queryNodeHead=newNode;
        queryCount--;
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

  free(queryNodeHead->key);
  free(queryNodeHead->value);
  while(NULL!=queryNodeHandle){
    queryNodeTemp=queryNodeHandle->next;
    free(queryNodeHandle->key);
    free(queryNodeHandle->value);
    free(queryNodeHandle);
    queryNodeHandle=queryNodeTemp;
  }
  free(queryNodeHandle);
  queryNodeHead->next=NULL;
}


static int parse_query_string(queryNode* queryNodeHead)
{
  char *query=getenv("QUERY_STRING");
  int queryCount = get_query_count();
  char *key, *value;

  for (int i=0;i<queryCount;i++){
    key=strtok(query,"=");
    queryNodeHead->key=(char*)malloc(sizeof(char) * (strlen(key)+1));
    strcpy(queryNodeHead->key,key);

    query+=strlen(key)+sizeof(char);

    value=strtok(query,"&");
    queryNodeHead->value=(char*)malloc(sizeof(char) * (strlen(value)+1));
    strcpy(queryNodeHead->value,value);
    query+=strlen(value)+sizeof(char);

    queryNodeHead=queryNodeHead->next;
  }
  return 0;
}

static void traverse_query_string(queryNode* queryNodeHead)
{
  while(NULL!=queryNodeHead){
//    printf("%s:%s\n",queryNodeHead->key,queryNodeHead->value);
    queryNodeHead=queryNodeHead->next;
  } 
}


static int mqtt_pub(char* thingID,queryNode* queryNodeHead)
{
  char query[1000];
  query[0]='\0';
  char command[1000];
  
  while(NULL!=queryNodeHead->next){
    strcat(query,queryNodeHead->key);
    strcat(query,"=");
    strcat(query,queryNodeHead->value);
    strcat(query,"&");
    queryNodeHead=queryNodeHead->next;
  }
    strcat(query,queryNodeHead->key);
    strcat(query,"=");
    strcat(query,queryNodeHead->value);
 

  sprintf(command, "mosquitto_pub -t %s -m \"%s\" -q 1",thingID,query);
  printf("%s\n",command);
  system(command);
  return(0);
}
