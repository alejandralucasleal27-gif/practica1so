
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void gestionarSenal(int numeroSenal) {
    (void) numeroSenal;
}

void crearProcesosColumna(int x) {
    int filaActual;
    int procesoHijo;

    for (filaActual = 1; filaActual <= x - 1; filaActual++) {
        procesoHijo = fork();

        if (procesoHijo != 0) {
            wait(NULL);
            exit(0);
        }
    }

    if (filaActual == x) {
        signal(SIGALRM, gestionarSenal);
        alarm(10);
        pause();
        exit(0);
    }
}

void crearProcesosMalla(int x, int y) {
    int columnaActual;
    int procesoHijo;

    for (columnaActual = 1; columnaActual <= y; columnaActual++) {
        procesoHijo = fork();

        if (procesoHijo == 0) {
            crearProcesosColumna(x);
            exit(0);
        }
    }

    if (columnaActual == y + 1) {
        for (columnaActual = 1; columnaActual <= y; columnaActual++) {
            wait(NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    int x, y;

    if (argc == 3) {
        x = atoi(argv[1]);
        y = atoi(argv[2]);

        if (x > 0 && y > 0) {
            crearProcesosMalla(x, y);
        }
    }

    return 0;
}
