#ifndef __FORCE_AUX_H__
#define __FORCE_AUX_H__

#include <stdlib.h>
#include <stdbool.h>
#include "body.h"
#include "list.h"
#include "forces.h"

typedef struct force_aux {
    list_t *body_list;
    double constant;
    collision_handler_t collision_handler;
    bool is_collision_handled;
    void *extra_aux;
    free_func_t freer;
} force_aux_t;

void force_free(force_aux_t *aux);

force_aux_t *force_init(list_t *bodies, double constant);

void force_set_collision_handler(force_aux_t *aux, collision_handler_t handler);

collision_handler_t force_get_collision_handler(force_aux_t *aux);

void force_set_is_collision_handled(force_aux_t *aux, bool is_collision_handled);

bool force_get_is_collision_handled(force_aux_t *aux);

void force_set_extra_aux(force_aux_t *aux, void *extra_aux);

void force_set_freer(force_aux_t *aux, free_func_t freer);

void *force_get_extra_aux(force_aux_t *aux);

void force_add_body(force_aux_t *aux, body_t *body);

body_t *force_get_body(force_aux_t *aux, size_t index);

list_t *force_get_body_list(force_aux_t *aux);

float force_get_constant(force_aux_t *aux);

void *force_get_extra_aux(force_aux_t *aux);

free_func_t force_get_freer(force_aux_t *aux);

#endif // #ifndef __FORCE_AUX_H__
