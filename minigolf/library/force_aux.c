#include "force_aux.h"

void force_free(force_aux_t *aux) {
  free_func_t freer = force_get_freer(aux);
  if (freer != NULL) {
    void* extra_aux = force_get_extra_aux(aux);
    freer(extra_aux);
  }
  free(aux);
}

force_aux_t *force_init(list_t *bodies, double constant) {
    force_aux_t *aux = malloc(sizeof(force_aux_t));
    aux->body_list = bodies;
    aux->constant = constant;
    aux->collision_handler = NULL;
    aux->is_collision_handled = false;
    aux->extra_aux = NULL;
    aux->freer = NULL;
    return aux;
}

void force_set_collision_handler(force_aux_t *aux, collision_handler_t handler) {
  aux->collision_handler = handler;
}

collision_handler_t force_get_collision_handler(force_aux_t *aux) {
  return aux->collision_handler;
}

void force_set_is_collision_handled(force_aux_t *aux, bool is_collision_handled) {
  aux->is_collision_handled = is_collision_handled;
}

bool force_get_is_collision_handled(force_aux_t *aux) {
  return aux->is_collision_handled;
}

void force_set_extra_aux(force_aux_t *aux, void *extra_aux) {
  aux->extra_aux = extra_aux;
}

void force_set_freer(force_aux_t *aux, free_func_t freer) {
  aux->freer = freer;
}

void *force_get_extra_aux(force_aux_t *aux) {
  return aux->extra_aux;
}

void force_add_body(force_aux_t *aux, body_t *body) {
    list_add(aux->body_list, body);
}

body_t *force_get_body(force_aux_t *aux, size_t index) {
    return list_get(aux->body_list, index);
}

list_t *force_get_body_list(force_aux_t *aux) {
    return aux->body_list;
}

float force_get_constant(force_aux_t *aux) {
    return aux->constant;
}

free_func_t force_get_freer(force_aux_t *aux) {
  return aux->freer;
}
