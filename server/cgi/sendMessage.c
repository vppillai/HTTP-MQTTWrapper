#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

/*
 *TODO: add size limit checks to enhance security
 *TODO: optimize size usage of query and command strings
 *
 */

#define MAX_ID_LEN 20
#define PUB_COMMAND_TEMPLATE  "mosquitto_pub -t %s -m '%s' -q 1"

static unsigned char debug=0;


typedef struct queryNode queryNode;

struct queryNode{
  char* key;
  char* value;
  queryNode* next;
};

char* REQUEST_URI;
char* QUERY_STRING;
int gQueryCount;

static int get_thing_id(char* thingID);
static int get_query_count(void);
static int allocate_query_nodes(queryNode* queryNodeHead);
static void free_query_node(queryNode* queryNodeHead);
static int parse_query_string(queryNode* queryNodeHead); 
static void traverse_query_string(queryNode* queryNodeHead);
static int mqtt_pub(char* thingID,queryNode* queryNodeHead);

int main(int argc, char *argv[])
{
  char thingID[MAX_ID_LEN]; /*eg: CIP23KW-B16B00*/
  int retVal;

  
  /*debug and test*/
  if (NULL!=getenv("DEBUG")){debug=1;}
  
  /*init global*/
  REQUEST_URI=getenv("REQUEST_URI");
  QUERY_STRING=getenv("QUERY_STRING");
  gQueryCount=get_query_count();

  get_thing_id(thingID);

  queryNode queryNodeHead;
  queryNodeHead.next=NULL;
  retVal=allocate_query_nodes(&queryNodeHead);

  switch(retVal){
    case -1 :
      printf("status: 500 Internal Server Error\n\n");
      printf("{\"error\":{\"code\":\"ERR_ALREADY_ALLOCATED\",\"reason\":\"node already allocated\"}}");
      exit(0);  
      break;
    case -2 : 
        printf("status: 500 Internal Server Error\n\n");
        printf("{\"error\":{\"code\":\"ERR_NODE_CALLOC_FAILED\",\"reason\":\"failed to calloc node\"}}");
        exit(0);  
        break;
    case 0 :
          break;
    default:
            break;
  }

  parse_query_string(&queryNodeHead);
  traverse_query_string(&queryNodeHead);

  printf("Content-type: application/json\n\n");
  mqtt_pub(thingID,&queryNodeHead);
  free_query_node(&queryNodeHead);
  return 0;
}


/*extract thing ID from requestURI*/
static int get_thing_id(char* thingID)
{

  if (NULL!=REQUEST_URI){
    char *tempID=strtok((strrchr(REQUEST_URI,'/')+1),"?");
    if(NULL!=tempID){
      strncpy(thingID,tempID,MAX_ID_LEN);
    }
    else{
      printf("status: 400 Bad request\n\n");
      printf("{\"error\":{\"code\":\"ERR_THING_UNKNOWN\",\"reason\":\"thing unknown\"}}"); /*test case: request ending in / */
      exit(0); /*if there is no thing, then there is no thang*/ 
    }
    return 0;
  }
  else{
    return -1;
  }
}


/*find number of queries*/
static int get_query_count(void)
{

  if (NULL!=QUERY_STRING){
    int queryCount_eql=0;
    int queryCount_amb=0;
    unsigned int queryLength=strlen(QUERY_STRING);
    for(int i=0;i<=queryLength;i++){
      if ('='==QUERY_STRING[i]) queryCount_eql++;
    }
    for(int i=0;i<=queryLength;i++){
      if ('&'==QUERY_STRING[i]) queryCount_amb++;
    }
    if(1==queryCount_eql){
      return 1;
    }
    else{
      if(queryCount_eql != (queryCount_amb+1)) return 0; /*we dont support non key=val queries*/
      else {
        return queryCount_eql;
      }
    }
  }
  else{
    return 0;
  }
}


/*create a linked list to store the queries*/
static int allocate_query_nodes(queryNode* queryNodeHead)
{
  if(0==gQueryCount)
    return 0;


  int queryCount = gQueryCount-1; /*we already gave head*/

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

  if(0==gQueryCount)
    return;

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

/*parse the query string and store it in the queryNode LL*/
static int parse_query_string(queryNode* queryNodeHead)
{
  char *query;
  char *key, *value;

  if(0!=gQueryCount){
    query=(char*)calloc(1,sizeof(char)*(strlen(QUERY_STRING)+1));
    strcpy(query,QUERY_STRING);
  }
  else{
    return 0;
  }

  for (int i=0;i<gQueryCount;i++){
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

/*a local test function to traverse the LL and print its contents*/
static void traverse_query_string(queryNode* queryNodeHead)
{
  while(NULL!=queryNodeHead){
    //printf("%s:%s\n",queryNodeHead->key,queryNodeHead->value);
    queryNodeHead=queryNodeHead->next;
  } 
}


/*publish the reconstructed queryString to topic-thingID*/
static int mqtt_pub(char* thingID,queryNode* queryNodeHead)
{
  /*sample: {"hello":"world","foo":"bar"} */
  char *query,*command;

  if(gQueryCount>0){
    unsigned int queryLength= sizeof(char)*(strlen(QUERY_STRING)+(5*gQueryCount)+100);
    query=(char*)malloc(queryLength);
    command=(char*)malloc(queryLength+(sizeof(char)*(strlen(thingID)+strlen(PUB_COMMAND_TEMPLATE)+10)));
    strcpy(query,"{\"");
    while(NULL!=queryNodeHead->next){
      strcat(query,queryNodeHead->key);
      strcat(query,"\":\"");
      strcat(query,queryNodeHead->value);
      strcat(query,"\",\"");
      queryNodeHead=queryNodeHead->next;
    }
    strcat(query,queryNodeHead->key);
    strcat(query,"\":\"");
    strcat(query,queryNodeHead->value);
    strcat(query,"\"}");
  }
  else{
    query="{}";
    command=(char*)calloc(1,(sizeof(char)*(strlen(thingID)+strlen(PUB_COMMAND_TEMPLATE)+10)));
  }


  sprintf(command, PUB_COMMAND_TEMPLATE,thingID,query);
  system(command);
  printf("{\"with\":{\"thing\":\"%s\",\"created\":\"2016-07-01T14:50:31.911Z\",\"content\":%s,\"transaction\":\"b80f15cf-e0e6-43e0-8caa-6575ece86187\"}}",thingID,query);
  //free(query);
  //free(command);
  return(0);
}
