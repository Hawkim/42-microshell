#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

int puterror(char *str , char *arg){


        while (str && *str)            
            write(2,str++,1);

        while (arg && *arg)
            write(2,arg++,1);
        
        write(2,"\n",1);

    return 1;
        
}


int execute(int i , int savefd, char **av, char **env){

    av[i]=NULL;
    dup2(savefd,STDIN_FILENO)  ;
    close(savefd);
    execve(av[0],av,env);
    puterror("error: cannot execute ", av[0]);
    return 1;
}

int main(int ac , char **av , char **env){

    (void)ac;
    int i =0;
    int savefd;
    int fd[2];

    savefd=dup(STDIN_FILENO);

    while (av[i] && av[i+1])
    {   
        av=&av[i+1];
        i=0;

        while(av[i] && strcmp(av[i],";") && strcmp(av[i],"|"))
        i++;

        if (i==0)
        continue;       
        
        if (strcmp(av[0],"cd")==0)
        {
            if (i!=2)
                puterror("error: cd: bad arguments",NULL);
    
            else if (chdir(av[1])!=0)
                 puterror("error: cd: cannot change directory to ", av[1]);
        }
        else if (av[i]==NULL || (strcmp(av[i],";")==0))
        {
            if (fork()==0)
            {
                execute(i ,savefd,av,env);
            }
            else
            {
                close(savefd);
                while (waitpid(-1,NULL,WUNTRACED)!=-1);
               savefd=dup(STDIN_FILENO);
                
            }
            
        }
        else if ((strcmp(av[i],"|")==0))
        {
             pipe(fd);
               if (fork()==0)
            {
                dup2(fd[1],STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execute(i ,savefd,av,env);

            }
            else
            {
                close(savefd);
                close(fd[1]);
               savefd=fd[0];
                
            }

        }
    }

close(savefd);
    return 1;

}