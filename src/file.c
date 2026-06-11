#include <stdio.h>
#include <stdlib.h>

#include "file.h"

char** file_read(const char* filename){
    // No es la mejor implementacion de lectura de files pero es comoda de manipular
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        printf("can't open file %s\n", filename);
        return NULL;
    }

    // Se garantiza que content siempre tiene al menos un elemento (NULL)
    char** content = malloc(sizeof(char*));

    char c;
    int j = 0;
    while((c = fgetc(fd)) != EOF){
        // Se garantiza que cada linea contiene al menos un caracter (\0)
        char* line = calloc(1, sizeof(char));
        int i = 0;
        while(c != '\n'){
            line[i] = c;
            line = realloc(line, (++i + 1) * sizeof(char));
            c = fgetc(fd);
        }
        // \0 como condicion de corte de los caracteres de una linea
        line[i] = '\0';
        content[j] = line;
        content = realloc(content, (++j + 1) * sizeof(char*));
    }
    // NULL como condicion de corte de las lineas
    content[j] = NULL;
    fclose(fd);
    return content;
}

void file_free(char** content){
    if(content != NULL){
        for(int i=0; content[i] != NULL; i++){
            free(content[i]);
        }
        free(content);
    }
}