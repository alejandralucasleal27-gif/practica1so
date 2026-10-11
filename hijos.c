
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>

void despierta(int senal);
int crearVertical(int x, int *vx, pid_t pid_hijos);
void mostrarProceso(int *vx, int i, pid_t pid_hijos);
int crearHorizontales(int y, int *vy);
void mostrarSuperPadre(int y, int *vy);

int main(int argc, char *argv[]) {
    int x, y, i;
    int shmidx, shmidy;
    int *vx, *vy;
    pid_t pid_hijos;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s x y\n", argv[0]);
        return 1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);

    if (x <= 0 || y <= 0) {
        fprintf(stderr, "Error: x e y deben ser positivos.\n");
        return 1;
    }

    pid_hijos = getpid();

    shmidx = shmget(IPC_PRIVATE, sizeof(int) * x,
                    IPC_CREAT | 0600);
    if (shmidx == -1) {
        perror("shmget vx");
        return 1;
    }

    vx = (int *) shmat(shmidx, NULL, 0);
    if (vx == (void *) -1) {
        perror("shmat vx");
        shmctl(shmidx, IPC_RMID, NULL);
        return 1;
    }

    shmidy = shmget(IPC_PRIVATE, sizeof(int) * y,
                    IPC_CREAT | 0600);
    if (shmidy == -1) {
        perror("shmget vy");
        shmdt(vx);
        shmctl(shmidx, IPC_RMID, NULL);
        return 1;
    }

    vy = (int *) shmat(shmidy, NULL, 0);
    if (vy == (void *) -1) {
        perror("shmat vy");
        shmdt(vx);
        shmctl(shmidx, IPC_RMID, NULL);
        shmctl(shmidy, IPC_RMID, NULL);
        return 1;
    }

    i = crearVertical(x, vx, pid_hijos);

    if (i == 1) {
        mostrarSuperPadre(y, vy);
    } else if (i == x + 1) {
        crearHorizontales(y, vy);
    }

    shmdt(vx);
    shmdt(vy);

    if (i == 1) {
        shmctl(shmidx, IPC_RMID, NULL);
        shmctl(shmidy, IPC_RMID, NULL);
    }

    return 0;
}

void despierta(int senal) {
    (void) senal;
}

void mostrarProceso(int *vx, int i, pid_t pid_hijos) {
    int j;

    printf("Soy el proceso %d. Mis padres son: %d",
           (int) getpid(), (int) pid_hijos);

    for (j = 0; j < i - 1; j++) {
        printf(", %d", vx[j]);
    }

    printf("\n");
    fflush(stdout);
}

int crearVertical(int x, int *vx, pid_t pid_hijos) {
    int i;
    pid_t pid;

    for (i = 1; i <= x; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork vertical");
            exit(1);
        }

        if (pid != 0) {
            while (wait(NULL) == -1 && errno == EINTR) {
            }
            break;
        }

        vx[i - 1] = (int) getpid();
        mostrarProceso(vx, i, pid_hijos);
    }

    return i;
}

void mostrarSuperPadre(int y, int *vy) {
    int i;

    printf("Soy el super padre %d, mis hijos finales son: ",
           (int) getpid());

    for (i = 0; i < y; i++) {
        printf("%d", vy[i]);

        if (i != y - 1) {
            printf(", ");
        }
    }

    printf("\n");
}

int crearHorizontales(int y, int *vy) {
    int i;
    pid_t pid;

    for (i = 1; i <= y; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork horizontal");
            exit(1);
        }

        if (pid == 0) {
            vy[i - 1] = (int) getpid();

            signal(SIGALRM, despierta);
            alarm(10);
            pause();

            break;
        }
    }

    if (i == y + 1) {
        for (i = 1; i <= y; i++) {
            while (wait(NULL) == -1 && errno == EINTR) {
            }
        }
    }

    return i;
}
