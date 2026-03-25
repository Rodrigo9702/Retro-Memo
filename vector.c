#include "vector.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

// Setup inicial del vector, pidiendo el primer bloque de memoria para arrancar.
int vector_init(tVector *v, size_t elem_size) {
  if (!v || elem_size == 0)
    return 0;

  v->size = 0;
  v->capacity = INITIAL_CAPACITY; // Arrancamos con un bloque para 8 elementos
  v->elem_size = elem_size;       // guardamos de qué tamaño son los chabones

  v->data = malloc(v->capacity * elem_size); // malloc clásico para el buffer
  if (!v->data)
    return 0;

  return 1;
}

// Lógica para agrandar o reajustar el buffer interno (realloc).
int vector_reserve(tVector *v, size_t new_capacity) {
  if (!v || new_capacity <= v->capacity)
    return 1;

  void *newData = realloc(v->data, new_capacity * v->elem_size);
  if (!newData)
    return 0;

  v->data = newData;
  v->capacity = new_capacity;
  return 1;
}

// Me inserta un elemento al final, y si no entra, duplico el tamaño general.
int vector_push_back(tVector *v, const void *elem) {
  if (!v || !elem)
    return 0;

  if (v->size >= v->capacity) {
    if (!vector_reserve(v, v->capacity * 2))
      return 0;
  }

  // Copio byte por byte la data del elemento original a nuestro bloque
  // dinámico.
  memcpy((char *)v->data + v->size * v->elem_size, elem, v->elem_size);

  v->size++; // contabilizamos que entró uno más

  return 1;
}

// Recupera el puntero crudo al elemento en la posición "index".
void *vector_get(const tVector *v, size_t index) {
  if (!v || index >= v->size)
    return NULL;

  return (char *)v->data + index * v->elem_size; // calculo la dirección exacta
}

size_t vector_size(const tVector *v) {
  if (!v)
    return 0;

  return v->size;
}

// Liberación total, importante llamarla cuando el vector muere.
void vector_destroy(tVector *v) {
  if (!v)
    return;

  free(v->data);
  v->data = NULL;
  v->size = 0;
  v->capacity = 0;
}

void vector_pop(tVector *v) {
  if (!v || v->size == 0)
    return;

  // Lógicamente, le decimos al vector que el último ya no es parseable (pop).
  v->size--;
}

void *vector_lastElement(tVector *v) {
  if (!v || v->size == 0)
    return NULL;

  // Calculo de puntero para agarrar al de arriba de todo.
  return (char *)v->data + ((int)(v->size - 1) * (int)v->elem_size);
}

// Un sort genérico usando selection sort. Es O(n^2) pero como mis vectores son
// chicos (el top de ranking etc) me sirve y no quise traer qsort estándar de
// stdlib.
void vector_sort(tVector *v, int (*cmp)(const void *, const void *)) {
  if (!v || v->size < 2)
    return;

  char temp[v->elem_size]; // Guardo acá de manera provisoria para intercambiar

  for (size_t i = 0; i < v->size - 1; i++) {
    size_t minIdx = i;

    for (size_t j = i + 1; j < v->size; j++) {
      if (cmp(vector_get(v, j), vector_get(v, minIdx)) < 0)
        minIdx = j;
    }

    if (minIdx != i) {
      memcpy(temp, vector_get(v, i), v->elem_size);
      memcpy(vector_get(v, i), vector_get(v, minIdx), v->elem_size);
      memcpy(vector_get(v, minIdx), temp, v->elem_size);
    }
  }
}
