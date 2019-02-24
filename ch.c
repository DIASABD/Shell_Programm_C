

/**********************************************************************************
 * Ce programme est une implementation d'un Shell en utilisant les librairies     *
 * POSIX. Les commande implementées sont : ls , tail, echo , man sleep .          *
 * Cette implemenation tient compte du statut de terminaisant des appele systèmes *
 * Aussi les occurences de if sont egalement gérées.                              *
 * auteur: Abdramane Diasso - 20057513 et Foadjo Willy - 20059876                 *
 * date:  09 - 02 - 2019                                                          *
 * problèmes connus: Voir fichier rapport.tex                                     *
 * ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>

const char * buffer_sep;
size_t bufsize = 512;
char **if_split_1;
char *if_split_2;
char ** buffer_array ;
int exec_status;
bool if_var ;
char ** input_split_1;
char *input_split_2;
pid_t waitpid(pid_t pid, int *status, int options);


/***********************************************************
* Fonction permettant d'initialiser un tableau en memoire  *                               *
************************************************************/
char  *init_buffer (){
    char *buffer;
    printf("MonShell $ ");
    size_t input_buffer;
    buffer = (char *) malloc(bufsize * sizeof(char));
    input_buffer = getline(&buffer,&bufsize,stdin);
    if ((strcmp("exit", buffer) == 0) || (buffer == NULL)) {
        exit(0);
    }
    buffer[input_buffer-1]='\0';
    return  buffer;
}


/****************************************************************
* cette fonction fait une iteration sur les chaine de caractère *
****************************************************************/

char *buf_iterator(char *buffer,const char *separator,size_t buffer_len)
{
    while(buffer!=NULL && *buffer && memcmp(buffer, separator, buffer_len)==0)
    {
        memset(buffer, 0x0, buffer_len);
        buffer+=buffer_len;
    }
    return buffer;
}

/*************************************************************************************************
* cette fonction renvoie une sous chaines d'un buffer à partir d'un index et de la longeur voulu *
*************************************************************************************************/

char *substring(char *string, int position, int length)
{
    char *pointer;
    int c;

    pointer = malloc(length+1);

    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    for (c = 0 ; c < length ; c++)
    {
        *(pointer+c) = *(string+position-1);
        string++;
    }

    *(pointer+c) = '\0';

    return pointer;
}

/*******************************************************************************************
* cette fonction permet  de faire un split sur une chaine de caractère avec comme indice   *
* un separeur. Elle retourne un pointeur double sur la chaine obténue.                     *
********************************************************************************************/

char ** split_buffer(char *buffer, char **return_pointer, char *temp_buff, const char *separator)
{
    int  i= 0;
    // varible temporaire
    char *temp=NULL;
    //taille du separateur
    size_t len=strlen(separator);
    strcpy(temp_buff, buffer);
    temp=temp_buff;
    // Itteration sur la chaine .
    for(return_pointer[i]=NULL, temp=buf_iterator(temp, separator, len);
        temp!=NULL && *temp; temp=buf_iterator(temp, separator, len))
    {
        return_pointer[i]=temp;
        i++;
        return_pointer[i]=NULL;
        temp=strstr(temp, separator);
    }
    return return_pointer;
}

/*************************************************************************************
*  cette fonction supprime une sous chaine dans un buffer. Elle retourne un pointure *
*  dirigé sur la chaine obténue.                                                     *
**************************************************************************************/

void  *str_rmv(char *buffer, const     char *sub_buf) {
    size_t len = strlen(sub_buf);
    if (len > 0) {
        char *p = buffer;
        while ((p = strstr(p, sub_buf)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
}
/****************************************************************************************************
* cette fonction permet d'executer une commande . C'est la fonction qui exécute toutes les commande *
* en créant les proccessus et en faisant appel à exec.                                              *
*****************************************************************************************************/
int execute_process(char *buffer, pid_t process){

    int status;
    char * ret ;
    char * p_red = strdup(">");
    pid_t w , arr_plan;
    char  fd[2];
    const char and [5] = "&&";
    char ** token = (char **)malloc(bufsize * sizeof(char*));
    if (strcmp("exit",buffer)==0||(token== NULL))
    {
        exit(0);
    };
    char *line = (char *) malloc(1024* sizeof(char));
    char * sep;
    sep= " ";
    char ** arg_list  = split_buffer(buffer,token,line,sep);
    ret = strstr(buffer,p_red);
    int r =0;
    while (arg_list[r]!=0){
        r++ ;
    }
    int execResponse= 0;


    process=fork();
    if (process < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed\n");
        return 1;
    }
    if (process==0)
    {     execResponse = execvp(arg_list[0],arg_list);
        if (execResponse==-1) printf("%s\n",strerror(errno));
    }
    else{
        w = waitpid(process, &status, WUNTRACED | WCONTINUED);
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    free(token);
    free(line);
    return  status;
}


/*********************************************************************************************
* cette fonction gère l'exécution des occurrence des if . elle traite aussi les if imbriqués *
*********************************************************************************************/
char **  split_if_occurrences (char * buffer,char ** list,size_t bufsize){
    int i =0;
    int r =0;
    const char sep_ [5] = ";";
    const char if_ [5] = "if";
    const char done_ [10] = "done";
    const char do_ [5] = " do";
    input_split_1= (char **)malloc(bufsize * sizeof(char*));
    input_split_2 = (char *) malloc(bufsize * sizeof(char));
    str_rmv(buffer,done_);
    str_rmv(buffer,if_);
    str_rmv(buffer,do_);
    list = split_buffer (buffer,input_split_1,input_split_2,sep_);
    while (list[r]!=0){
        r++ ;
    }
    list[r-1]=NULL;
    return   list ;
}


/*********************************************************************************
* cette fonction separe les occurrence des and et or et les mets dans un tableau.*
*********************************************************************************/
char ** split_or_and_occurrences(char *buffer,size_t bufsize){
    char *ret;
    char ** return_pointer;
    char ** p;
    const char or [5] = "||";
    const char and [5]= "&&";
    ret = strstr(buffer, or);
    if(ret!=NULL){
        buffer_sep =strdup(or);
    } else{
        ret = strstr(buffer, "&&");
        if(ret!=NULL) buffer_sep=strdup(and);
    }
    if(ret==NULL){
        return_pointer= split_if_occurrences(buffer,p,bufsize);

    } else{
        input_split_1= (char **)malloc(bufsize * sizeof(char*));
        input_split_2= (char *) malloc(bufsize * sizeof(char));
        return_pointer = split_buffer (buffer,input_split_1,input_split_2,buffer_sep);
    }
    return  return_pointer;
}



/************************************************************************
* cette fonction gère l'exécution des occurrence des and et or .        *
*************************************************************************/

int exec_if_occurence(char *buffer,pid_t  process,size_t bufsize){
    int  index  = 0;

    bool y_choise = false;
    char ** p ;
    exec_status =0;
    const char or [5] = "||";
    int  y = 0;
    char ** imput_array;
    char * do_ret = strstr(buffer,"do");
    char * if_ret = strstr(buffer,"if");
    char * if_sep_  =strstr(buffer,";");

    if(if_var==false&&if_sep_==NULL){
        exec_status=  execute_process(buffer,process);
        y_choise =true;
    } else if (if_ret!=NULL &&do_ret!=NULL&&y_choise!=true&&if_sep_==NULL) {
        exec_status = execute_process(buffer, process);
    }
    else{
        imput_array = split_if_occurrences(buffer,p,bufsize);
        while(imput_array[index]!=NULL){
            exec_status= execute_process(imput_array[index],process);
            if(exec_status!=0){
                index+=2;
            } else{
                index++;
            }
        }
        free(imput_array);
    }
    return  exec_status;
}
/****************************************************************************************************
* cette fonction permet d'executer une commande . C'est la fonction qui exécute toutes les commande *
* en créant les proccessus et en faisant appel à exec.                                              *
*****************************************************************************************************/
int exec_input(char * buffer, pid_t process){

    buffer_array = split_or_and_occurrences(buffer,bufsize);
    int status_;
    char *ret ;
    char *p_red = strdup(">");
    const char and [5] = "&&";
    const char or [5] = "||";
    if(buffer_sep==NULL){
        exec_if_occurence(buffer,process,bufsize);
    }
    else if(buffer_sep!=NULL)
    {
        ret = strstr(buffer,buffer_sep);
    }
    if(ret==NULL&&buffer_sep!=NULL){
        exec_if_occurence(buffer,process,bufsize);
    }
    else if(ret!=NULL&&buffer_sep!=NULL){
        char **token = (char **)malloc(bufsize * sizeof(char*));
        char *input;
        int  index  = 0;
        const char if_ [5] = "if";
        while (buffer_array[index]!=NULL) {
            input = strdup(buffer_array[index]);
            char *_ret_b = strstr(input,if_);
            if (_ret_b==NULL) {
                if_var = false;
            } else {
                if_var = true;
            }
            exec_status =0;
            status_ = exec_if_occurence(buffer_array[index], process,bufsize);
            if (status_ != 0 && (strcmp(buffer_sep, and) == 0)) {
                exit(EXIT_FAILURE);
            } else if (status_ == 0 &&strcmp(buffer_sep, or) == 0) {
                exit(EXIT_SUCCESS);
            }
            free(input);
            index++;
        }
        free(token);
    }
    free(if_split_1);
    free(if_split_2);
    free(buffer_array);
}

pid_t waitpid(pid_t pid, int *status, int options);

/************************************************************
cette fonction est la fonction qui gère les taches  arrières
*************************************************************/
void gestion_arr_plan(char *buffer_,pid_t  process){
    char *line_ = (char *) malloc(bufsize* sizeof(char));
    char * sep;
    char *buffer_1;
    sep= " ";
    char *X_b = init_buffer();
    buffer_1 = strdup(X_b);
    exec_input(buffer_1,process);
    free(buffer_1);
    wait(&process);
    exec_input(buffer_,process);
    free(line_);
}


/******************************************************
cette fonction est la fonction qui gère les redirection
*******************************************************/

int redirection( char *buffer,pid_t process,char ** input_split_1,char *input_split_2,char * sep) {
    FILE *fd;
    int input, output;
    char ** arg_list = split_buffer(buffer, input_split_1, input_split_2,sep);
    char *output_file = strdup(arg_list[1]);
    fd = fopen(output_file, "w");
    input = open(output_file, O_RDONLY);
    output = open(output_file, O_WRONLY);
    dup2(input, 0);
    dup2(output, 1);
    close(input);
    close(output);
    execute_process(arg_list[0],process);
    free(if_split_1);
    free(if_split_2);
    exit(0);
}

/***********************************************************************
cette fonction est la fonction principale main. Elle s'occupe de       *
 l'allocation mémoire au début et la lecture en ligne de commande.     *
***********************************************************************/

int main (void) {

    printf("%s\n", " /*************************************************************************");
    printf("%s\n"," /* Ce programme est une implementation des commande echo , ls , tail man. *");
    printf("%s\n"," /* Il présente des fonctionnnalités similaires à  celles de /bin/sh:      *");
    printf("%s\n"," /* Il prend en compte certaines  spécifications notament l'utilisation du *");
    printf("%s\n"," /* && et du || de mêmes que les abstarctions du if dans les requêtes .    *");
    printf("%s\n"," /* Auteurs: Abdramane Diasso - 20057513 et Foadjo Willy - 20059876        *");
    printf("%s\n"," /**************************************************************************");
    printf("%s\n","                                                                            ");
    printf("%s\n","                                                                            ");
    printf("%s\n","                                                                            ");
    while (1) {
        char *buffer_;
        char * sep;
        sep= " ";
        char * p_arr = strdup("&");
        pid_t  process;
        char ** arg_list;

        const char if_ [5] = "if";
        buffer_ = strdup(init_buffer());
        char ** token_ = (char **)malloc(bufsize * sizeof(char*));
        char * list_ = (char *)malloc(bufsize * sizeof(char*));
        if (strcmp("exit",buffer_)==0||(token_== NULL))
        {
            exit(0);
        }
        else if (strlen(buffer_)>=1){
            char * redir_ret = strstr(buffer_,">");
            if(redir_ret!=NULL){
                redirection(buffer_,process,token_,list_,">");
            }
            else{

                arg_list  = split_buffer(buffer_,token_,list_,sep);
                int r =0;
                while (arg_list[r]!=0){
                    r++ ;
                }
                if(strcmp(arg_list[r-1],p_arr)==0) {
                    char * ret = strstr(buffer_,if_);
                    int y = strlen(buffer_);
                    char * X = substring(buffer_,1,y-2);
                    if(ret!=NULL){
                        exec_input(X,process);
                    } else{
                        gestion_arr_plan(X,process);
                    }
                }
                else{
                    exec_input(buffer_,process);
                }
            }
        }

        free(list_);
        free(token_);
        free(buffer_);
    }
}
