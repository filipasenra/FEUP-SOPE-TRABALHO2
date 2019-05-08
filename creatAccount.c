#include "creatAccount.h"

int creatSalt(char salt[SALT_LEN + 1])
{
    //there is a better, more secure way to do this
    //but i'll leave now
    sprintf(salt, "%lx", clock());

    return 0;
}

int getHash(char salt[SALT_LEN + 1], char password[], char hash[HASH_LEN + 1])
{
    int n, fd1[2], fd2[2];
    pid_t pid;

    pipe(fd1);
    pipe(fd2);
    pid = fork();

    if (pid > 0)
    { /* parent */
        close(fd1[0]);
        close(fd2[1]);

        char *result = malloc(strlen(password));
        strcpy(result, password);
        strcat(result, salt);

        n = strlen(result);
        write(fd1[1], result, n);
        close(fd1[1]);

        n = read(fd2[0], hash, HASH_LEN);

        if (n == 0)
        {
            printf("child closed pipe\n");
            return 1;
        }

        hash[n] = 0;

        return 0;
    }
    else
    { /* child */
        close(fd1[1]);
        close(fd2[0]);
        if (fd1[0] != STDIN_FILENO)
        {
            dup2(fd1[0], STDIN_FILENO);
            close(fd1[0]);
        }

        if (fd2[1] != STDOUT_FILENO)
        {
            dup2(fd2[1], STDOUT_FILENO);
            close(fd2[1]);
        }

        execlp("sha256sum", "sha256sum", NULL);
    }

    return 0;
}

int creatAccount(bank_account_t *account, char password[], int accound_id, int balance)
{
    //echo -n “<senha><sal>” | sha256sum
    //echo -n $salt | sha256sum
    account->account_id = accound_id;
    account->balance = balance;
    creatSalt(account->salt);

    getHash(account->salt, password, account->hash);

    return 0;
}