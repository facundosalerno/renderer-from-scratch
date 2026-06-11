#include <stdlib.h>
#include <string.h>

#include "wavefront.h"
#include "file.h"

Model wavefront_read(const char* filename){
    Model model = {
        .v = NULL, 
        .v_len = 0, 
        .v_x_lower = 0,
        .v_x_higher = 0,
        .v_y_lower = 0,
        .v_y_higher = 0,
        .v_z_lower = 0,
        .v_z_higher = 0,
        .f = NULL, 
        .f_len = 0, 
        .error = ""
    };
    char** content = file_read(filename);
    if(content == NULL){
        model.error = "could not read file";
        return model;
    }

    for(int i=0; content[i] != NULL; i++){
        char* line_copy = malloc(strlen(content[i]) + 1);
        strcpy(line_copy, content[i]);
        
        char* token = strtok(line_copy, " ");
        if(token == NULL){
            free(line_copy);
            continue;
        }

        if(strcmp(token, "v") == 0){
            char* raw_x = strtok(NULL, " ");
            char* raw_y = strtok(NULL, " ");
            char* raw_z = strtok(NULL, " ");
            if(raw_x == NULL || raw_y == NULL || raw_z == NULL){
                model.error = "failed to parse v line";
                free(line_copy);
                file_free(content);
                wavefront_free(&model);
                return model;
            }
            model.v = realloc(model.v, ++model.v_len * sizeof(Vertex));
            float x = model.v[model.v_len-1].x = atof(raw_x);
            float y = model.v[model.v_len-1].y = atof(raw_y);
            float z = model.v[model.v_len-1].z = atof(raw_z);
            if(model.v_len == 1){
                model.v_x_lower = model.v_x_higher = x;
                model.v_y_lower = model.v_y_higher = y;
                model.v_z_lower = model.v_z_higher = z;
            } else {
                if(x > model.v_x_higher)
                    model.v_x_higher = x;
                if(y > model.v_y_higher)
                    model.v_y_higher = y;
                if(z > model.v_z_higher)
                    model.v_z_higher = z;

                if(x < model.v_x_lower)
                    model.v_x_lower = x;
                if(y < model.v_y_lower)
                    model.v_y_lower = y;
                if(z < model.v_z_lower)
                    model.v_z_lower = z;
            }
        }

        if(strcmp(token, "f") == 0){
            char* raw_f1 = strtok(NULL, " ");
            char* raw_f2 = strtok(NULL, " ");
            char* raw_f3 = strtok(NULL, " ");
            if(raw_f1 == NULL || raw_f2 == NULL || raw_f3 == NULL){
                model.error = "failed to parse f line";
                free(line_copy);
                file_free(content);
                wavefront_free(&model);
                return model;
            }

            char* raw_idx_1 = strtok(raw_f1, "/");
            char* raw_idx_2 = strtok(raw_f2, "/");
            char* raw_idx_3 = strtok(raw_f3, "/");
            if(raw_idx_1 == NULL || raw_idx_2 == NULL || raw_idx_3 == NULL){
                model.error = "failed to parse f line segment";
                free(line_copy);
                file_free(content);
                wavefront_free(&model);
                return model;
            }
            model.f = realloc(model.f, ++model.f_len * sizeof(Face));
            model.f[model.f_len-1].v[0] = atoi(raw_idx_1) - 1;
            model.f[model.f_len-1].v[1] = atoi(raw_idx_2) - 1;
            model.f[model.f_len-1].v[2] = atoi(raw_idx_3) - 1;
        }

        free(line_copy);
    }
    file_free(content);
    return model;
}

void wavefront_free(Model* model){
    free(model->v);
    model->v = NULL;
    free(model->f);
    model->f = NULL;
}