

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LENGTH 1024 
#define MAX_LINE 80
#define sizeOfHistory 5
static char * History[sizeOfHistory];
static int currentSize = 0;


int should_run = 1; 
int waitFlag = 1;




char * tokenize(char *cmd)
{
	int i;
	int j = 0;
	char *tokenized = (char *)malloc((MAX_LINE * 2) * sizeof(char));
	for (i = 0; i < strlen(cmd); i++)
	{
		if (cmd[i] != '>' && cmd[i] != '<' && cmd[i] != '|')
		{
			tokenized[j++] = cmd[i];
		}
		else
		{
			tokenized[j++] = ' ';
			tokenized[j++] = cmd[i];
			tokenized[j++] = ' ';
		}
	}
	tokenized[j++] = '\0';
	char *end;
	end = tokenized + strlen(tokenized) - 1;
	end--;
	*(end + 1) = '\0';

	return tokenized;
}

void Run(char *cmd) //Ham chay command
{
	char *args[MAX_LINE / 2 + 1];
	char *tokens;
	tokens = tokenize(cmd);

	if (tokens[strlen(tokens) - 1] == '&')
	{
		tokens[strlen(tokens) - 1] = '\0';
		waitFlag = 0;
	}

	char *arg = strtok(tokens, " ");
	int i = 0;
	while (arg)
	{
		if (*arg == '<')
		{
			In(strtok(NULL, " "));
		}
		else if (*arg == '>')
		{
			Out(strtok(NULL, " "));
		}
		else if (*arg == '|')
		{
			args[i] = NULL;
			Tao_Pipe(args);
			i = 0;
		}
		else
		{
			args[i] = arg;
			i++;
		}
		arg = strtok(NULL, " ");
	}
	args[i] = NULL;
	pid_t pid;
	if (strcmp(args[0], "exit") == 0)
	{
		should_run = 0;
	}
	else
	{
		if (strcmp(args[0], "history") == 0)
		{
			int i;
			for (i = currentSize - 1; i >= 0; i--)
			{
				printf("%i %s\n", i, History[i]);
			}
		}
		else if (strcmp(args[0], "!!") == 0)
		{
			ThucThiLenhCMDCuoiCung(cmd);
		}
		else
		{
			AddCMDtoHistory(cmd);
			ThucThi(args);
		}
	}
}

void AddCMDtoHistory(char *cmd) //Ham them command vao lich su
{
	if (currentSize == (sizeOfHistory - 1)) 
	{
		int i;
		free(History[0]); 

		for (i = 1; i < currentSize; i++) 
			History[i - 1] = History[i];
		currentSize--;
	}
	History[currentSize++] = strdup(cmd); 
}

void ThucThiLenhCMDCuoiCung(char *cmd) 
{

	int count = 0;
	if (currentSize == 0)
	{
		printf("No commands in history\n");
		return;
	}
	if (cmd[1] == '!')
		count = currentSize - 1;
	else {
		count = atoi(&cmd[1]) - 1;
		if ((count < 0) || (count > currentSize))
		{
			fprintf(stderr, "No such command in history.\n");
			return;
		}
	}
	printf("%s %s\n", cmd, History[count]);
	char *args[MAX_LINE / 2 + 1];
	char *tokens;
	tokens = tokenize(History[count]);
	if (tokens[strlen(tokens) - 1] == '&') 
	{
		waitFlag = 0;
		tokens[strlen(tokens) - 1] = '\0';
	}

	char *arg = strtok(tokens, " ");
	int i = 0;
	while (arg)
	{
		if (*arg == '>')
		{
			Out(strtok(NULL, " "));
		}
		else if (*arg == '<')
		{
			In(strtok(NULL, " "));
		}
		else if (*arg == '|')
		{
			args[i] = NULL;
			Tao_Pipe(args);
			i = 0;
		}
		else
		{
			args[i] = arg;
			i++;
		}
		arg = strtok(NULL, " ");
	}
	args[i] = NULL;
	ThucThi(args);
}

void In(char *file)
{
	int filein = open(file, O_RDONLY);
	dup2(filein, 0); 
	close(filein);
}

void Out(char *file)
{
	int fileout = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	dup2(fileout, 1); 
	close(fileout);
}

void ThucThi(char *args[]) 
{
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		fprintf(stderr, "fork failed");
	}
	else if (pid == 0)
	{
		execvp(args[0], args);
	}
	else
	{
		if (waitFlag)
		{
			waitpid(pid, NULL, 0);
		}
		else
		{
			waitFlag = 0;
		}
	}
	In("/dev/tty");
	Out("/dev/tty");
}

void Tao_Pipe(char *args[]) 
{
	int fd[2];
	pipe(fd);

	dup2(fd[1], 1);
	close(fd[1]);

	printf("args = %s\n", *args);

	ThucThi(args);

	dup2(fd[0], 0);
	close(fd[0]);
}


int main(void)
{
	char * cmd = (char*)malloc(sizeof(char)*MAX_LENGTH);
	while (should_run)
	{
		printf("osh> ");
		fflush(stdout);
		fgets(cmd, MAX_LENGTH, stdin);
		Run(cmd);
	}
	free(cmd);
	return 0;
}
