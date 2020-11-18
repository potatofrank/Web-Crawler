#include "crawler.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

char *front_url;
char m_host[url_size];
char m_object[url_size];
char html[] = "";
int m_socket;
int req_num = 0;

//start crawling
void crawling(char *url){
  queue *q;
  char url_list[html_size] = "";
  char my_url_str[url_amount][url_size];
  int count = 0;
  int count_url = 0;

  //queue initialization and establish our first push
  q = malloc(sizeof(queue));
  initialize(q);
  push(q, url);

  //this string is for comparison for duplication removal purpose
  char temp_curr[url_size];
  strcpy(temp_curr, url);
  int url_index = strlen(temp_curr);
  char last_char = temp_curr[url_index-1];
  if(last_char != '/'){
    strcat(temp_curr, "/");
  }
  strcat(url_list, temp_curr);
  strcat(url_list, "\n");

  //printf("push succeed\n");
  while(!isempty(q)){
    strcpy(html, "");
    //1. print the url that's been fetching
    char *curr_url;
    curr_url = q->front->url_str;

    //remove the head of queue as soon as we record it and construc fetching
    pop(q);

    //initialize internet
    Init();

    //analyze current url string
    url_analysis(curr_url);

    //initiate network connection
    Connect();

    //fetch html plain text
    char html[html_size];
    fetch_html(html);

    //check out the request type
    char *check_request = strstr(html, "http/1.1");
    int http_pos = check_request - html;
    int req_num_len = strlen(html);
    char req_num_cpy[req_num_len + 1];
    slice_str(html, req_num_cpy, http_pos+req_typelen, http_pos+req_taillen);
    sscanf(req_num_cpy, "%d", &req_num); 

    //deal with special requests 
    if(req_num == UNAVAILABLE || req_num == UNAUTHORIZATION){
      crawling(curr_url);
      continue;
    }else if(req_num == ERROR || req_num == GONE || req_num == LONG){
      continue;
    }else if(req_num == MOVEDURL){
      char *find_http = "http://";
      char *find_vary = ".html";
      char *check_http = strstr(html, find_http);
      char *check_vary = strstr(html, find_vary);
      int pos_head = check_http - html;
      int pos_tail = check_vary - html;
      int len = strlen(html);
      char red_url[len+1];
      slice_str(html, red_url, pos_head, pos_tail+html_length);
      crawling(red_url);
      continue;
    }
    printf("%s\n", curr_url);

    //check content type and reject certain types
    char *content_type = "content-type:";
    char *check_type = strstr(html, content_type);
    int type_pos = check_type - html;
    int type_len = strlen(html);
    char type[type_len+1];
    slice_str(html, type, type_pos, type_pos+TYPE_VOLUME);
    char *type_want = "text/";
    char *check_want = strstr(type, type_want);
    if(check_want==NULL){
        continue;
    }
    
    //start fetching html text
    while(true){
      //first thing of the first, check if there's more to catch
      char *anchor_head = "<a ";
      char *anchor_tail = ">";
      char *find_head = strstr(html, anchor_head);
      if(find_head == NULL){
        break;
      }
      
      //get anchor tag head
      int pos = find_head - html;
      int len = strlen(html);
      char temp_anchor[len + 1];
      slice_str(html, temp_anchor, pos, len);
      
      //get url head
      char *search = "href";
      char *find_href = strstr(temp_anchor, search);
      int new_pos = find_href - temp_anchor;
      len = strlen(temp_anchor);
      char anchor_text[len+1];
      slice_str(temp_anchor, anchor_text, new_pos+4, len);
      
      //get url tail
      char *quot = "\"";
      char *search_str = strstr(anchor_text, quot);
      new_pos = search_str - anchor_text;
      len = strlen(anchor_text);
      char new_url[len+1];
      slice_str(anchor_text, new_url, new_pos+1, len);

      //completely harvest our url
      char *second_quot = strstr(new_url, quot);
      new_pos = second_quot - new_url;
      len = strlen(new_url);
      char my_url[len+1];
      slice_str(new_url, my_url, 0, new_pos-1);
      
      //give the backside of the url to html for continuous fetching
      len = strlen(html);
      char buf[len+1];
      slice_str(new_url, buf, new_pos+1, len);
      strcpy(html, buf);

      //for possible urls lack of an http header, we add one to it
      //urls can lack of a host name, or has a host name but withour http header
      char *verify_http = "http://";
      char *check_http = strstr(my_url, verify_http);
      char *check_host = strstr(my_url, m_host);
      char *check_double_dash = strstr(my_url, "//");

      //no http header, not current host name, doesnt have double dash at front
      //no http header, current host name, double dash
      //no http header, current host name, has double dash
      //no http header, not current host name, does have double dash
      if(check_http == NULL && check_host == NULL && check_double_dash == NULL){
        char *web_exis = "asjdaskjd";
        char *check_web = strstr(my_url, web_exis);
        if(check_web == NULL){
          char *single_slash = "/";
          char *check_single_slash = strstr(my_url, single_slash);
          if(check_single_slash != NULL){
            int sla_pos = check_single_slash - my_url;
            char use[url_size] = "http://";
            if(sla_pos == 0){
              strcat(use, m_host);
              strcat(use, my_url);
              strcpy(my_url, use);
            }else if(sla_pos > 0){
              strcat(use, m_host);
              strcat(use, "/");
              strcat(use, my_url);
              strcpy(my_url, use);
            }
          }else{
            continue;
          }
        }else{
          char use[url_size] = "http://";
          strcat(use, my_url);
          strcpy(my_url, use);
        }
      }else if(check_http == NULL && check_host != NULL && check_double_dash == NULL){
          char use[url_size] = "http://";
          strcat(use, my_url);
          strcpy(my_url, use);
      }else if(check_http == NULL && check_host != NULL && check_double_dash != NULL){
          char use[url_size] = "http:";
          strcat(use, my_url);
          strcpy(my_url, use);
      }else if(check_http == NULL && check_host == NULL && check_double_dash != NULL){
          char use[url_size] = "http:";
          strcat(use, my_url);
          strcpy(my_url, use);
      }
      
      
      //this is the temp url string to do the duplication check
      char temp_my_url[url_size];
      strcpy(temp_my_url, my_url);
      int my_index = strlen(temp_my_url);
      char my_last = temp_my_url[my_index-1];
      if(my_last != '/'){
        strcat(temp_my_url, "/");
      }

      //search for current with "/" sign added to it, if unique, append
      char *check_duplicate = strstr(url_list, temp_my_url);
      if(check_duplicate == NULL){
        strcpy(my_url_str[count], my_url);
        push(q, my_url_str[count]);
        strcat(url_list, temp_my_url);
        strcat(url_list, "\n");
        count++;
      }

      //In case we find a change line sign, for display purpose we delete it
      int url_len_check = strlen(my_url);
      if(my_url[0] == '\n'){
        char change_url[len+1];
        char buf_url[len+1];
        slice_str(my_url, change_url, 1, len);
        strcpy(my_url, change_url);
      }
    }
  }
  free(q);
}

//initialize a queue
void initialize(queue *q)
{
  q->count = 0;
  q->front = NULL;
  q->back = NULL;
}

//check if the queue is empty
bool isempty(queue *q)
{
  if(q->front == NULL){
    return true;
  }
  return false;
}

//push to the queue
void push(queue *q, char *value)
{
  node *tmp;
  tmp = malloc(sizeof(node));
  tmp->url_str = value;
  tmp->next = NULL;
  if(!isempty(q))
  {
    q->back->next = tmp;
    q->back = tmp;
  }
  else
  {
    q->front = q->back = tmp;
  }
  q->count++;
}

//pop from the queue
char* pop(queue *q)
{
  node *tmp;
  front_url = q->front->url_str;
  tmp = q->front;
  q->front = q->front->next;
  q->count--;
  free(tmp);
  return front_url;
}

//print the queue
void display(node *head)
{
  if(head == NULL)
  {
    printf("NULL\n");
  }
  else
  { 
    printf("%s\n", head -> url_str);
    
    if(head->next == NULL){
      return;
    }
    display(head->next);
  }
}

//url to lower case 
char* str_tolower(char *url){
  for(int i = 0; url[i]; i++){
    url[i] = tolower(url[i]);
  }
  return url;
}

//string slicing function
void slice_str(const char * str, char * buffer, size_t start, size_t end)
{
  size_t j = 0;
  for ( size_t i = start; i <= end; ++i ) {
      buffer[j++] = str[i];
  }
  buffer[j] = 0;
}

//initialize internet
void Init(){
  //socket creation
  m_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket == -1){
    fprintf(stderr, "ERROR, no socket found!\n");
    exit(1);
  }
}

void url_analysis(char *url){

  str_tolower(url);
  int len = strlen(url);
  char new_url[len + 1];
  slice_str(url, new_url, http_length, len);

  //record host name
  if(find(new_url, "/") == false){
    strcpy(m_host, new_url);
    strcpy(m_object, "/");
  }else{
    int index = find_index(new_url, "/") - 1;
    char *new_url_str = new_url;
    len = strlen(new_url_str);
    char buffer_host[len + 1];
    char buffer_object[len + 1];
    slice_str(new_url_str, buffer_host, 0, index);
    slice_str(new_url_str, buffer_object, index+1, len);
    strcpy(m_host, buffer_host);
    strcpy(m_object, buffer_object);
  }
}

//get the index of a character in a string
int find_index(char *str, char *ch){
  int index;
  char *c;
  c = strchr(str, *ch);
  index = (int)(c - str);
  return index;
}

//search for a character in a string
bool find(char *sent, char *word){
  if(strstr(sent, word) != NULL) {
    return true;
  }
  return false;
}

//internet programming - socket
void Connect(){

  //get host
  struct hostent *p = gethostbyname(m_host);
  
  //check if host is successfully achieved
  if(p == NULL){
    fprintf(stderr,"ERROR, invalid host\n");
    exit(1);
  }

  //establish connection
  struct sockaddr_in sa;
  memcpy(&sa.sin_addr, p->h_addr, 4);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(PORT);

  //validate connection
  if(connect(m_socket, (struct sockaddr*)&sa, sizeof(sa)) < 0){
    fprintf(stderr, "ERROR, connection false!\n");
    exit(1);
  }
}

void fetch_html(char *html){
  //send GET request string initialization
  char info[url_size];
  strcpy(info, "GET ");
  strcat(info, m_object);
  strcat(info, " HTTP/1.1\r\n");
  strcat(info, "Host: ");
  strcat(info, m_host);
  strcat(info, "\r\n");
  strcat(info, "User-Agent: yichaoy1\r\n");
  if(req_num == UNAUTHORIZATION){
    strcat(info, "Authorization: Basic eWljaGFveTE6cGFzc3dvcmQ=\r\n");
  }
  strcat(info, "Connection: Close\r\n\r\n");
  //send request to server
  if(send(m_socket, info, strlen(info), 0) < 0){
    fprintf(stderr, "ERROR, failed to send");
    exit(1);
  }
  
  //receive data from server
  char ch = 0;
  int nRecv = 0;
  strcpy(html, "");
  while((nRecv = recv(m_socket, &ch, sizeof(ch), 0)) > 0){
    //appened bits to the html string
    strncat(html, &ch, 1);
  }
  str_tolower(html);
}