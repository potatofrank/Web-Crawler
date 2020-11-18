#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "crawler.h"

int main(int argc,char **argv){
  char url[url_size];

  //make sure the input size is correct
  if(argc != 2){
    fprintf(stderr, "ERROR, input format should be %s + url\n", argv[0]);
    exit(1);
  }
  
  strcpy(url, argv[1]);
  char url_head[url_size] = "http://";
  char *check_url = strstr(url, url_head);
  int pos = check_url - url;

  //if no proper header format, make it proper
  if(check_url == NULL){
    strcat(url_head, url);
    strcpy(url, url_head);
  }else if(pos != 0){
    //if http is in wrong position, reject program straight away
    fprintf(stderr, "ERROR, wrong url!\n");
    exit(1);
  }

  //start crawling
  crawling(url);

  return 0;
}