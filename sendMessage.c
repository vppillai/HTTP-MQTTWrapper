#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN 20
static unsigned char debug=0;

static unsigned int get_thing_id(char* thingID);
static unsigned int get_query_count(void);

int main()
{
  char thingID[MAX_ID_LEN]; /*eg: PIC32WK-B16B00*/

  /*debug and test*/
  if (NULL!=getenv("DEBUG")){debug=1;}
  if(0!=debug){if(!get_thing_id(thingID)) printf("thingID: %s\n",thingID);}
  if(0!=debug){printf("queryCount: %d\n",get_query_count());}


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
