#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include<conio.h>

extern char **environ;  // for the environ command

#define COMMAND_BUFSIZE 1024
#define STRTOK_BUFSIZE 64
#define STRTOK_DELIM " \t\r\n\a"

/*--------------------------------------------------------------------------------------------------------------------*/

/*  Declaring our running commands:    */

int myshell_cd(char **args);
int myshell_clr(char **args);
int myshell_dir(char **args);
int myshell_environ(char **args);
int myshell_help(char **args);
int myshell_pause(char**args);
int myshell_quit(char **args);
int myshell_echo(char **args);

char *running_str[] = {
        "cd",
        "clr",
        "dir",
        "environ",
        "help",
        "pause",
        "quit",
        "echo"
};

int (*running_func[]) (char **) = {
        &myshell_cd,
        &myshell_clr,
        &myshell_dir,
        &myshell_environ,
        &myshell_help,
        &myshell_pause,
        &myshell_quit,
        &myshell_echo
};

int myshell_num_running() {
    return sizeof(running_str) / sizeof(char *);
}

/*  Running the built-in commands:  */

int myshell_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "myshell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("myshell");
        }
    }
    return 1;
}

int myshell_clr(char **args) {
    printf("\033[H\033[2J");
    return 1;
}

int myshell_dir(char **args) {
    struct dirent *de;  // Pointer for directory entry
    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(".");
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 0;
    }
    while ((de = readdir(dr)) != NULL)
        printf("%s\n", de->d_name);
    closedir(dr);
    return 1;

}

int myshell_environ(char **args) {
    char **s = environ;

    for (; *s; s++) {
        printf("%s\n", *s);
    }

    return 1;
}

int myshell_help(char **args)
{
//    int i;
//
//    for (i = 0; i < myshell_num_running(); i++) {
//        printf("  %s\n", running_str[i]);
//    }

    return 1;
}

int myshell_pause(char **args) {
    getchar();
    return 1;
}

int myshell_quit(char **args)
{
    return 0;
}

int myshell_echo(char **args) {
    char str[50];

    printf("\n Enter input: ");
    scanf("%[^\n]+", str);

    printf(" Echo : %s\n", str);

    return 1;
}
/*--------------------------------------------------------------------------------------------------------------------*/

/*  Function to read command line from STDIN until hitting `Enter` or EOF.
 *  Returns string command line  */
char *myshell_reading_commands(void) {
    int position;
    char * buffer = malloc(sizeof(char) *COMMAND_BUFSIZE);
    if(!buffer) {
        fprintf(stderr, "myshell: memory allocation error");
        exit(EXIT_FAILURE);
    }

    while (1) {
        position = read(STDIN_FILENO, buffer, COMMAND_BUFSIZE); //read(file descriptor, buffer to read data from, length of buffer)
        if (position == -1) {
            fprintf(stderr, "myshell: memory allocation error");
            exit(EXIT_FAILURE);
        }
        buffer[position] = '\0';
        return buffer;
    }
}

/*  Function to parse the command into a list of arguments  */
char **myshell_parsing_commands(char *line) {
    int position = 0;
    char **tokens = malloc(sizeof(char *) * STRTOK_BUFSIZE);
    char *token;
    if (!tokens) {
        fprintf(stderr, "myshell: memory allocation error");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, STRTOK_DELIM);   // get the first word
    while (token != NULL) {
        tokens[position] = token; // Store parse string in tokens following position
        position++;   // Update new position
        token = strtok(NULL, STRTOK_DELIM);   // NULL must be used to get tokens from the previous string
    }
    tokens[position] = NULL;
    return tokens;
}

/*  Function to execute the command line from user  */
int myshell_execute(char **args) {
    if(args[0] == NULL) // if an empty command was entered
        return 1;

    //  Compare args with running_str to determine what command was chosen
    for(int i =0;i < myshell_num_running(); i++ )
    {
        if(strcmp(args[0],running_str[i]) == 0)
            return (*running_func[i])(args);
    }

    //Launching the commands
    pid_t pid, childPID;

    pid = fork();
    switch (pid) {
        case -1:
            perror("myshell: Error with fork \n");
            exit(EXIT_FAILURE);

        case 0: // Child process
            if(execvp(args[0], args)== -1)
            {
                perror("myshell: Error with execvp \n");
                exit(EXIT_FAILURE);
            }
            _exit(EXIT_SUCCESS);
        default:
            childPID = wait(NULL);
            if(childPID == -1)
            {
                perror("myshell: Error with wait \n");
                exit(EXIT_FAILURE);
            }
    }
    return 1;
}

void myshell_mainLoop(void) {
    char *line, **args;
    int status;
    do {
        printf("myshell> ");    // prints the prompt
        fflush(stdout);
        line = myshell_reading_commands();  // function to read the lines
        args = myshell_parsing_commands(line);  // function to split the line into args
        status = myshell_execute(args); // execute the args
    } while (status != 0);  // determines when to exit
}

/*--------------------------------------------------------------------------------------------------------------------*/

int main(int argc, char **argv) {
    myshell_mainLoop();

    return EXIT_SUCCESS;
}