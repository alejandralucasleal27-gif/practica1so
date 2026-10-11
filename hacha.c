
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void partir(char *nombre, int tamanyo);
int calcularTrozos(struct stat propFichero, int tamanyo);
void generarNombre(char *nombre, int i, char *nombreFichero);
void enviarTrozo(int fEntrada, int tubo[], char *trozoLeido, int tamanyo);
void guardarTrozo(char *nombre, int i, int tubo[], char *trozoLeido, int tamanyo);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Error. Uso: %s fichero tam_trozo\n", argv[0]);
    } else {
        partir(argv[1], atoi(argv[2]));
    }

    return 0;
}

int calcularTrozos(struct stat propFichero, int tamanyo) {
    int numTrozos;

    numTrozos = propFichero.st_size / tamanyo;

    if (propFichero.st_size % tamanyo != 0) {
        numTrozos++;
    }

    return numTrozos;
}

void generarNombre(char *nombre, int i, char *nombreFichero) {
    if (i < 10) {
        sprintf(nombreFichero, "%s.h0%d", nombre, i);
    } else {
        sprintf(nombreFichero, "%s.h%d", nombre, i);
    }
}

void enviarTrozo(int fEntrada, int tubo[], char *trozoLeido, int tamanyo) {
    int numLeidos;

    numLeidos = read(fEntrada, trozoLeido, tamanyo);
    write(tubo[1], trozoLeido, numLeidos);
}

void guardarTrozo(char *nombre, int i, int tubo[], char *trozoLeido, int tamanyo) {
    char nombreFichero[50];
    int fSalida;
    int numLeidos;

    generarNombre(nombre, i, nombreFichero);
    fSalida = creat(nombreFichero, 0666);

    numLeidos = read(tubo[0], trozoLeido, tamanyo);
    write(fSalida, trozoLeido, numLeidos);

    close(fSalida);
    free(trozoLeido);
}

void partir(char *nombre, int tamanyo) {
    int i;
    int tubo[2];
    int fEntrada;
    struct stat propFichero;
    int numTrozos;
    char *trozoLeido;

    trozoLeido = (char *) malloc(sizeof(char) * tamanyo);
    fEntrada = open(nombre, O_RDONLY);

    if (fEntrada < 0) {
        printf("Error. No existe el fichero\n");
    } else {
        stat(nombre, &propFichero);
        numTrozos = calcularTrozos(propFichero, tamanyo);

        for (i = 0; i < numTrozos; i++) {
            pipe(tubo);

            if (fork() != 0) {
                enviarTrozo(fEntrada, tubo, trozoLeido, tamanyo);
            } else {
                guardarTrozo(nombre, i, tubo, trozoLeido, tamanyo);
                break;
            }
        }

        if (i == numTrozos) {
            free(trozoLeido);

            for (i = 0; i < numTrozos; i++) {
                wait(NULL);
            }
        }
    }
}

