#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define url_size 1024
#define url_amount 100
#define html_size 100000
#define http_length 7
#define html_length 4
#define req_typelen 9
#define req_taillen 11
#define TYPE_VOLUME 30
#define MOVEDURL 301
#define UNAUTHORIZATION 401
#define UNAVAILABLE 503
#define ERROR 404
#define GONE 410
#define LONG 414
#define BADREQ 400
#define PORT 80

struct node
{
  char *url_str;
  struct node *next;
};
typedef struct node node;

//queue definition
struct queue
{
    int count;
    node *front;
    node *back;
};
typedef struct queue queue;

//function definitions
void crawling(char *url);
void initialize(queue *q);
bool isempty(queue *q);
void push(queue *q, char *value);
char* pop(queue *q);
void display(node *head);
char* str_tolower(char *url);
void slice_str(const char * str, char * buffer, size_t start, size_t end);
int find_index(char *str, char *ch);
bool find(char *sent, char *word);
void Init();
void url_analysis(char *url);
void Connect();
void fetch_html(char *html);
void redirection(char *html);