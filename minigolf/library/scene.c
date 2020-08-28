#include "forces.h"
#include "scene.h"
#include "force_aux.h"
#include <stdlib.h>

typedef struct scene {
    list_t *body_list;
    list_t *force_list;
    list_t *force_bodies_list;
    list_t *aux_list;
    list_t *free_list;
} scene_t;

const size_t INITIAL = 10;

scene_t *scene_init(void) {
    scene_t *scene = malloc(sizeof(scene_t));
    scene->body_list = list_init(INITIAL, (free_func_t)body_free);
    scene->force_list = list_init(INITIAL, (free_func_t)null_free);
    scene->force_bodies_list = list_init(INITIAL, (free_func_t)list_free);
    scene->aux_list = list_init(INITIAL, (free_func_t)null_free);
    scene->free_list = list_init(INITIAL, (free_func_t)null_free);
    return scene;
}

void scene_free(scene_t *scene) {
    for (size_t i = 0; i < list_size(scene->aux_list); i++) {
        free_func_t freer = list_get(scene->free_list, i);
        freer(list_get(scene->aux_list, i));
    }
    list_free(scene->aux_list);
    list_free(scene->force_list);
    list_free(scene->body_list);
    list_free(scene->force_bodies_list); //problem child
    list_free(scene->free_list);
    free(scene);
}

size_t scene_bodies(scene_t *scene) {
    return list_size(scene->body_list);
}

body_t *scene_get_body(scene_t *scene, size_t index) {
    return list_get(scene->body_list, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
    list_add(scene->body_list, body);
}

/**
 * @deprecated Use body_remove() instead
 */
void scene_remove_body(scene_t *scene, size_t index) {
    body_remove(list_get(scene->body_list, index));
}

/**
 * @deprecated Use scene_add_bodies_force_creator()
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
    list_t *empty = list_init(0, null_free);
    scene_add_bodies_force_creator(scene, forcer, aux,
        empty, freer);
}


void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
    void *aux, list_t *bodies, free_func_t freer) {

    if (freer == NULL) {
        freer = null_free;
    }

    list_add(scene->force_list, forcer);
    list_add(scene->aux_list, aux);
    list_add(scene->force_bodies_list, bodies);
    list_add(scene->free_list, freer);
}


void scene_tick(scene_t *scene, double dt) {
    for (size_t i = 0; i < list_size(scene->force_list); i++) {
        force_creator_t forcer = list_get(scene->force_list, i);
        void *aux = list_get(scene->aux_list, i);
        forcer(aux);
    }

    for (size_t i = 0; i < list_size(scene->body_list); i++) {
        body_t *body = list_get(scene->body_list, i);
        body_tick(body, dt);
        if (body_is_removed(body)) {
            for (size_t j = 0; j < list_size(scene->force_bodies_list); j++) {
                for (size_t k = 0; k < list_size(list_get(scene->force_bodies_list, j)); k++) {
                    body_t *b = list_get(list_get(scene->force_bodies_list, j), k);
                    if (b == body) {
                        list_t *bodies = list_remove(scene->force_bodies_list, j);
                        force_creator_t force = list_remove(scene->force_list, j);
                        void *aux = list_remove(scene->aux_list, j);
                        free_func_t freer = list_remove(scene->free_list, j);
                        list_free(bodies);
                        freer(aux);
                        null_free(force);
                        null_free(freer);
                        j--;
                        break;
                    }
                }
            }
            body_t *body = list_remove(scene->body_list, i);
            body_free(body);
            i--;
        }
    }


}
