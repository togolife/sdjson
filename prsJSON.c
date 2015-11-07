#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum JSON_TYPE{
    JSON_OBJECT = 1,
    JSON_ARRAY,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_STRING,
    JSON_STATIC // TRUE/FALSE/NULL
};

typedef struct json_item {
    json_item *next, *prev;
    json_item *child;
    JSON_TYPE type;
    char *title;// item's name
    char *value;
    int ivalue;
    double dvalue;
}json_item;

char *errstr = NULL;

char *getError (void);
json_item *createItem (void);
void deleteItem (json_item *it);
json_item *newItem (char **str);
json_item *parseString (json_item *it, char **str);
json_item *parseNum (json_item *it, char **str);
json_item *parseArray (json_item *it, char **str);
json_item *parseObject (json_item *it, char **str);
json_item *parseJson (json_item *it, char **str);
void printJson (json_item *it, int tab);
char *skip (char *str);


char *getError (){
    return errstr;
}

char *skip (char *str){
    char *p = str;
    while (*p == ' ' || *p == '\t' || *p == '\n'){
      if (*p == '\0') break;
      ++p;
    }
    return p;
}

json_item *createItem (void){
    json_item *it = NULL;
    it = (json_item *) malloc (sizeof (json_item));
    if (it == NULL){
        errstr = "create item malloc error!";
    }else{
        memset (it, 0x00, sizeof (json_item));
    }
    return it;
}

void deleteItem (json_item *it){
    if (it == NULL)
        return;
    json_item *prev, *next;
    prev = it->prev;
    next = it->next;
    if (prev != NULL){
        prev->next = next;
    }
    if (next != NULL){
        next->prev = prev;
    }
    it->prev = it->next = NULL;
    json_item *p = it->child;
    json_item *q = NULL;
    while (p){
        q = p->next;
        deleteItem (p);
        p = q;
    }
    it->child = NULL;
    if (it->title){
        free (it->title);
    }
    if (it->value){
        free (it->value);
    }
    free (it);
}

// new create item
json_item *newItem (char **str){
    json_item *it = createItem ();
    if (it == NULL){
        return NULL;
    }
    char *p = *str;
    // if this item have title then save
    if (strchr (p, ':') != NULL && *p == '\"'){
        ++p;
        int ilen = 0;
        while (p != NULL && *p != '\0' && *p != '\"'){
            ++ilen;
            ++p;
        }
        if (*p == '\0' || p == NULL){
            errstr = "title not correct!";
            deleteItem (it);
            return NULL;
        }
        it->title = (char *) calloc (ilen + 1, sizeof (char));
        if (it->title == NULL){
            errstr = "save title error!";
            deleteItem (it);
            return NULL;
        }
        memcpy (it->title, (*str)+1, ilen);
        *str = *str + ilen + 2;
    }
    return it;
}

json_item *parseString (json_item *it, char **str){
    char *p = *str;
    ++p;
    int len = 0;
    while (p != NULL && *p != '\0' && *p != '\"'){
        ++len;
        if (*p == '\\'){// if 
            ++p;
            ++len;
        }
        ++p;
    }
    if (*p == '\0' || p == NULL){
        errstr = "string value not correct!";
        deleteItem (it);
        return NULL;
    }
    it->value = (char *) calloc (len + 1, sizeof (char));
    if (it->value == NULL){
        errstr = "save value malloc mem error!";
        deleteItem (it);
        return NULL;
    }
    memcpy (it->value, *str + 1, len);
    *str = *str + 2 + len;
    return it;
}

json_item *parseNum (json_item *it, char **str){
    char *p = *str;
    int num = 0, flag = 1, dflag = 1, num1 = 0, num2 = 0;
    char tmp = 1;
    if (*p == '-'){
        flag = -1;
        ++p;
    }else if (*p == '+'){
        ++p;
    }
    while (*p >= '0' && *p <= '9'){
        num = 10 * num + (*p - '0');
        ++p;
    }
    if (*p == '.'){
        tmp = 0;
        ++p;
    }
    while (*p >= '0' && *p <= '9'){
        num = 10 * num + (*p - '0');
        --num1;
        ++p;
    }
    if (*p == 'e' || *p == 'E'){
        ++p;
    }
    if (*p == '-'){
        dflag = -1;
        ++p;
    }else if (*p == '+'){
        ++p;
    }
    while (*p >= '0' && *p <= '9'){
        num2 = 10 * num2 + (*p - '0');
        ++p;
    }
    if (tmp){
        it->type = JSON_INTEGER;
    }else{
        it->type = JSON_DOUBLE;
    }
    it->dvalue = num * flag * pow (10, num1 + dflag * num2);
    it->ivalue = (int)it->dvalue;
    it->value = (char *) calloc (p - *str + 1, sizeof (char));
    if (it->value == NULL){
        errstr = "save num value malloc mem error";
        deleteItem (it);
        return NULL;
    }
    memcpy (it->value, *str, p - *str);
    *str = p;
    return it;
}

int getItem (char **str, char c){
    char end = c == '[' ? ']' : '}';
    char itemb = c == '[' ? '{' : '[';
    char iteme = c == '[' ? '}' : ']';
    char *p = *str;
    int num = 0;
    int len = 0;
    while (*p != '\0'){
        if (*p == '\\'){
            p += 2;
            len += 2;
            continue;
        }
        if (*p == '\"'){
            ++p;
            ++len;
            while (*p != '\"'){
                if (*p == '\\'){
                    p += 2;
                    len += 2;
                    continue;
                }
                ++p;
                ++len;
            }
            ++p;
            ++len;
            continue;
        }
        if (*p == itemb){
            int inum = 0;
            ++p;
            ++len;
            while (*p != iteme){
                ++p;
                ++len;
                if (*p == itemb){
                    ++inum;
                }
                if (*p == iteme){
                    if (inum > 0)
                        --inum;
                    else
                        break;
                }
                ++p;
                ++len;
            }
            ++p;
            ++len;
            continue;
        }
        if (*p == c){
            ++num;
        }
        if ((*p == ',' || *p == end) && num == 0){
            break;
        }else if (*p == end){
            --num;
        }
        ++p;
        ++len;
    }
    *str = p;
    return len;
}

json_item *parseArray (json_item *it, char **str){
    char *p = *str;
    char *q = NULL;
    json_item *pre = it;
    json_item *cur = NULL;
    int i = 0;
    ++p;
    q = p;
    // search ','  and parse each string
    while (p != NULL && *p != '\0'){
        p = skip (p);
        q = p;
        int len = getItem (&p, '[');
        if (*p == '\0'){
            errstr = "search array's value error!";
            deleteItem (it);
            return NULL;
        }
        char *tv = (char *) calloc (len + 1, sizeof (char));
        if (tv == NULL){
            errstr = "save array's item malloc mem error!";
            deleteItem (it);
            return NULL;
        }
        char *tvtmp = tv;
        memcpy (tv, q, len);
        cur = newItem (&tv);
        if (cur == NULL){
            printf ("%s\n", getError ());
            free (tvtmp);
            return NULL;
        }
        cur = parseJson (cur, &tv);
        if (i == 0){
            i = 1;
            cur->prev = NULL;
            cur->next = NULL;
            pre->child = cur;
        }else{
            cur->prev = pre;
            cur->next = NULL;
            pre->next = cur;
        }
        pre = cur;
        free (tvtmp);
        if (*p == ']')
            break;
        ++p;
    }
    *str = p + 1;
    return it;
}

json_item *parseObject (json_item *it, char **str){
    char *p = *str;
    char *q = NULL;
    json_item *pre = it;
    json_item *cur = NULL;
    int i = 0;
    ++p;
    q = p;
    // search ','  and parse each string
    while (p != NULL && *p != '\0'){
        p = skip (p);
        q = p;
        int len = getItem (&p, '{');
        if (*p == '\0'){
            errstr = "search object's value error!";
            deleteItem (it);
            return NULL;
        }
        char *tv = (char *) calloc (len + 1, sizeof (char));
        if (tv == NULL){
            errstr = "save object's item malloc mem error!";
            deleteItem (it);
            return NULL;
        }
        char *tvtmp = tv;
        memcpy (tv, q, len);
        if (strchr (tv, ':') == NULL){
            errstr = "an item should have title!";
            free (tv);
            deleteItem (it);
            return NULL;
        }
        cur = newItem (&tv);
        if (cur == NULL){
            printf ("%s\n", getError ());
            free (tvtmp);
            return NULL;
        }
        tv = skip (tv);
        if (*tv == ':'){
        //    printf ("correct!\n");
            ++tv;
        }
        tv = skip (tv);
        cur = parseJson (cur, &tv);
        if (i == 0){
            i = 1;
            cur->prev = NULL;
            cur->next = NULL;
            pre->child = cur;
        }else{
            cur->prev = pre;
            cur->next = NULL;
            pre->next = cur;
        }
        pre = cur;
        free (tvtmp);
        if (*p == '}')
            break;
        ++p;
    }
    *str = p + 1;
    return it;
}

json_item *parseJson (json_item *it, char **str){
    char *p = *str;
    if (*p == '['){
        it->type = JSON_ARRAY;
        it = parseArray (it, str);
    }
    else if (*p == '{'){
        it->type = JSON_OBJECT;
        it = parseObject (it, str);
    }
    else if (*p == '\"'){
        it->type = JSON_STRING;
        it = parseString (it, str);
    }
    else if (strncmp (p, "true", 4) == 0){
        it->type = JSON_STATIC;
        it->value = (char *) calloc (5, sizeof (char));
        if (it->value == NULL){
            errstr = "save value true malloc mem error!";
            deleteItem (it);
            return NULL;
        }
        strcpy (it->value, "true");
    }
    else if (strncmp (p, "false", 5) == 0){
        it->type = JSON_STATIC;
        it->value = (char *) calloc (6, sizeof (char));
        if (it->value == NULL){
            errstr = "save value false malloc mem error!";
            deleteItem (it);
            return NULL;
        }
        strcpy (it->value, "false");
    }
    else if (strncmp (p, "null", 4) == 0){
        it->type = JSON_STATIC;
        it->value = (char *) calloc (5, sizeof (char));
        if (it->value == NULL){
            errstr = "save value null malloc mem error!";
            deleteItem (it);
            return NULL;
        }
        strcpy (it->value, "null");
    }
    else {
        it = parseNum (it, str);
    }
    return it;
}

void printTab (int tab){
    int i = 0;
    for (; i < tab; ++i){
        putchar ('\t');
    }
    return;
}

void printJson (json_item *it, int tab){
    json_item *cur = it;
    if (it == NULL) return;
    printTab (tab);
    if (it->title != NULL)
        printf ("\"%s\":", it->title);
    switch (it->type){
    case JSON_ARRAY:
        printf ("[\n");
        printJson (cur->child, tab+1);
        printf ("\n");
        printTab (tab);
        printf ("]");
        break;
    case JSON_OBJECT:
        printf ("{\n");
        printJson (cur->child, tab+1);
        printf ("\n");
        printTab (tab);
        printf ("}");
        break;
    case JSON_STRING:
        if (it->value != NULL)
            printf ("\"%s\"", it->value);
        break;
    default:
        if (it->value != NULL)
            printf ("%s", it->value);
        break;
    }
    if (cur->next != NULL){
        printf (",\n");
        printJson (cur->next, tab);
    }
    return;
}


void doPrase (char *str){
    json_item *root = newItem (&str);
    if (root == NULL){
        printf ("%s\n", getError ());
        return;
    }
    json_item *tmp = root;
    root = parseJson (root, &str);
    if (root == NULL){
        printf ("%s\n", getError ());
        deleteItem (tmp);
        return;
    }
    // test the result
    printJson (root, 0);
    deleteItem (root);
    return;
}

#if 1
int main (int argc, char *argv[]){
    if (argc != 2){
        printf ("usage: prsJson filename\n");
        return -1;
    }
    FILE *fp = NULL;
    char *str = NULL;
    fp = fopen (argv[1], "r");
    if (fp == NULL){
        printf ("open file [%s] error!\n", argv[1]);
        return -1;
    }
    fseek (fp, 0, SEEK_END);
    int len = ftell (fp);
    fseek (fp, 0, SEEK_SET);
    str = (char *)calloc (len + 1, sizeof (char));
    if (str == NULL){
        printf ("calloc error!\n");
        fclose (fp);
        return -1;
    }
    fread (str, len, 1, fp);
    fclose (fp);
    
    char *p = str;
    doPrase (str);
    free (p);
    
	char text1[]="{\n\"name\": \"Jack (\\\"Bee\\\") Nimble\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}";	
	char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[]="[\n    [0, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n			\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\"\n	 }\n	 ]";

    printf ("\n");
    doPrase (text1);
    printf ("\n");
    doPrase (text2);
    printf ("\n");
    doPrase (text3);
    printf ("\n");
    doPrase (text4);
    printf ("\n");
    doPrase (text5);
    
    return 0;
}

#endif
