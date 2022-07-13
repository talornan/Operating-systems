#include "main.h"

static char prompt[512];
char cwd[1024];
char cmd[MAX_SIZE_CMD];

int sockfd = -1;
int stdoutfd;

void out(char *output)
{
    if (sockfd == -1)
    {
        printf("%s\n", output);
        return;
    }
    send(sockfd, output, strlen(output), 0);
}
//Q1
// taken from https://github.com/kalpishs/Unix-Shell-Implementation-in-C/blob/master/shell.c
void shell_prompt()
{

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {

        strcpy(prompt, "Our_Shell: ");
        strcat(prompt, cwd);
        strcat(prompt, "$ ");
        printf("%s\n", prompt);
    }
    else
    {

        perror("Error in getting current working directory: ");
    }
    return;
}
//Q2
// taken from https://github.com/AlphaArslan/C-Simple-Linux-Shell-Wrapper/blob/master/code/shell.c
void get_cmd()
{
    // get command from user
    printf("\nyes master?\n");
    fgets(cmd, MAX_SIZE_CMD, stdin);
    // remove trailing newline
    if ((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n'))
        cmd[strlen(cmd) - 1] = '\0';
    // printf("%s\n", cmd);
}
//Q4.1
// taken from https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html
void openTcpConnection()
{
    struct sockaddr_in servaddr;

    // Create a socket for the client
    // If sockfd<0 there was an error in the creation of the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Problem in creating the socket\n");
    }

    // Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(localHost);
    servaddr.sin_port = htons(ServerPort);

    // Connection of the client to the socket
    int cconnect = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (cconnect < 0)
    {
        sockfd = -1;
        perror("Problem in connecting to the server\n");
        return;
    }
    dup2(sockfd, 1);
}
//Q5
void localOutput()
{
    printf("LOCAL\n");
    close(sockfd);
    sockfd = -1;
    dup2(stdoutfd, 1);
}

// because there is a system call on UNIX we can understand a system function    
//Q10- taken from - https://www.programmingsimplified.com/c-program-copy-file
    void copyFile(){
        char source_file[20], target_file[20];
        FILE *source, *target;

        printf("Enter name of file to copy\n");
        fgets(source_file, 20, stdin);
        source_file[strlen(source_file) - 1] = '\0';

        source = fopen(source_file, "r");

        if (source == NULL)
        {
            printf("Error opening %s file \n", source_file);
            exit(EXIT_FAILURE);
        }

        printf("Enter name of target file\n");
        fgets(target_file, 20, stdin);
        target_file[strlen(target_file) - 1] = '\0';

        target = fopen(target_file, "w");

        if (target == NULL)
        {
            fclose(source);
            printf("Error opening %s file\n", target_file);
            exit(EXIT_FAILURE);
        }

        char buf[25];
        int ch;
        while ((ch = fread(buf, sizeof(char), 25, source)))
        {
            fwrite(buf, sizeof(char), ch, target);
        }
        printf("File copied successfully.\n");
        fclose(source);
        fclose(target);
        return;
    }

    // This is a system call, it is used to deletes files
    //Q11 - use in - https://github.com/6arek212/Operating-Systems-BASH-C/blob/master/main.c
    void DeleteFile(){
        char buffer[101];
        printf("Name of file to delete:  ");
        fgets(buffer, 20, stdin);
        buffer[strlen(buffer) - 1] = '\0';

        if (unlink(buffer) == 0)
        {
            printf("File %s  deleted.\n", buffer);
        }
        else
        {
            printf("Error deleting the file %s.\n", buffer);
        }
}

int main(int argc, char const *argv[])
{
    shell_prompt();
    while (1)
    {
        get_cmd();
        if (strcmp(cmd, "EXIT") == 0)
        {
            return 0;
        }
        //Q3
        else if (strncmp(cmd, "ECHO", 4) == 0)
        {
            char subtext[strlen(cmd) - 4];
            subtext[strlen(cmd) - 5] = '\0';
            strncpy(subtext, &cmd[5], strlen(cmd) - 1);
            // puts(subtext);
            out(subtext);
        }
        else if (strcmp(cmd, "TCP PORT") == 0)
        {
            openTcpConnection();
        }

        else if (strcmp(cmd, "LOCAL") == 0)
        {
            printf("Server shutdown\n");
            localOutput();
            printf("the output is in standard output!\n");
            
        }
        // chdir is a system function that used to change the current working directory.
        //Q6
        // taken from https://c-for-dummies.com/blog/?p=3246
        else if (strcmp(cmd, "DIR") == 0)
        {
            DIR *folder;
            struct dirent *entry;
            int files = 0;

            folder = opendir(".");
            if (folder == NULL)
            {
                perror("Unable to read directory");
                return (1);
            }

            while ((entry = readdir(folder)))
            {
                files++;
                out(entry->d_name);
                printf("File %3d: %s\n",
                       files,
                       entry->d_name);
            }
            closedir(folder);
        }
        //Q7
        // taken from https://www.geeksforgeeks.org/chdir-in-c-language-with-examples/
        //  this function is system calling
        else if (strcmp(cmd, "CD") == 0)
        {
            char s[100];
            // printing current working directory
            // printf("%s\n", getcwd(s, 100));
            out(getcwd(s, 100));
            // using the command
            chdir("..");
            // printing current working directory
            printf("%s\n", getcwd(s, 100));
            // after chdir is executed
        }
        else if(strcmp(cmd, "COPY") == 0)
            {
                copyFile();
            }

            /*
            The unlink function is a system function.
            */
            else if(strcmp(cmd, "DELETE") == 0)
            {
                DeleteFile();
            } 

        else if(strlen(cmd) > 0)
        {
            // this a system function
            //Q8
            // system(cmd);
            
            //Q9
            char *p = cmd;
            int count = 0;

            // counting the number of arguments in the cmd
            while ((p = strstr(p, " ")))
            {
                if (*(p + 1) != '\0')
                {
                    p++;
                    count++;
                }
                else
                {
                    break;
                }
            }
            count++;
            // this is the split function
            char **args = (char **)malloc(sizeof(char *) * (count + 1));
            char *token = strtok(cmd, " ");
            int i = 0;
            while (token != NULL && i < count)
            {
                args[i] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
                strcpy(args[i], token);
                i++;
                token = strtok(NULL, " ");
            }
            args[count] = NULL;

            // fork this thread
            pid_t pid = fork();

            if (pid < 0)
            {
                printf("error, failed to fork()");
            }
            else if (pid > 0)
            {
                // the parent will wait here
                int status;
                waitpid(pid, &status, 0);
            }
            else
            {
                execvp(args[0], args);
                // the child will run another program and finish here
            }

            // free pointers
            for (int i = 0; i < count; i++)
            {
                free(args[i]);
            }
            free(args);
        }
    }
}
