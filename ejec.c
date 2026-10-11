
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

pid_t pidEjec, pidA, pidB, pidX, pidY, pidZ;

void ignorarSenal(int numeroSenal) {
    (void) numeroSenal;
}

void destruirEjec(int numeroSenal) {
    (void) numeroSenal;
    printf("ejec => arb\n");
    kill(pidA, SIGUSR2);
}

void destruirA(int numeroSenal) {
    (void) numeroSenal;
    printf("A => B\n");
    kill(pidB, SIGUSR2);
}

void mostrarArbol(int numeroSenal) {
    (void) numeroSenal;

    char pidComoTexto[100];
    sprintf(pidComoTexto, "%d", pidEjec);

    if (fork() == 0) {
        execlp("pstree", "pstree", "-c", pidComoTexto, NULL);
        exit(1);
    } else {
        wait(NULL);
        printf("mandando señal a arb: %d\n", pidEjec);
        kill(pidEjec, SIGUSR2);
        pause();
    }
}

void ejecutarX(void) {
    printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n",
           getpid(), pidB, pidA, pidEjec);

    signal(SIGUSR2, ignorarSenal);
    pause();

    printf("Soy X (%d) y muero\n", getpid());
    exit(0);
}

void ejecutarY(void) {
    printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n",
           getpid(), pidB, pidA, pidEjec);

    signal(SIGUSR2, ignorarSenal);
    pause();

    printf("Soy Y (%d) y muero\n", getpid());
    exit(0);
}

void ejecutarZ(int tiempo) {
    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo %d\n",
           getpid(), pidB, pidA, pidEjec);

    signal(SIGALRM, ignorarSenal);
    alarm(tiempo);
    pause();

    kill(pidA, SIGUSR1);

    signal(SIGUSR2, ignorarSenal);
    pause();

    printf("Soy Z (%d) y muero\n", getpid());
    exit(0);
}

void ejecutarB(int tiempo) {
    pidB = getpid();

    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           pidB, pidA, pidEjec);

    pidX = fork();
    if (pidX == 0) {
        ejecutarX();
    }

    pidY = fork();
    if (pidY == 0) {
        ejecutarY();
    }

    pidZ = fork();
    if (pidZ == 0) {
        ejecutarZ(tiempo);
    }

    signal(SIGUSR2, ignorarSenal);
    pause();

    printf("B manda señales a hijos\n");

    kill(pidX, SIGUSR2);
    wait(NULL);

    kill(pidY, SIGUSR2);
    wait(NULL);

    kill(pidZ, SIGUSR2);
    wait(NULL);

    exit(0);
}

void ejecutarA(int tiempo) {
    pidA = getpid();

    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n",
           pidA, pidEjec);

    pidB = fork();

    if (pidB == 0) {
        ejecutarB(tiempo);
    } else {
        signal(SIGUSR1, mostrarArbol);
        signal(SIGUSR2, destruirA);

        pause();
        wait(NULL);

        printf("Soy A (%d) y muero\n", pidA);
    }
}

void ejecutarRaiz(int tiempo) {
    pidEjec = getpid();

    printf("Soy el proceso ejec: mi pid es %d\n", pidEjec);

    pidA = fork();

    if (pidA != 0) {
        signal(SIGUSR2, destruirEjec);
        pause();

        wait(NULL);

        printf("Soy ejec (%d) y muero\n", pidEjec);
        exit(0);
    } else {
        ejecutarA(tiempo);
    }
}

int main(int argc, char *argv[]) {
    int tiempo;

    if (argc != 2) {
        printf("Usage: ./ejec tiempo\n");
    } else {
        tiempo = atoi(argv[1]);
        ejecutarRaiz(tiempo);
    }

    return 0;
}
