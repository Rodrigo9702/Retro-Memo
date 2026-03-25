#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

/**
 * IMPLEMENTACIONES DEL TDA VECTOR EN ESTE PROYECTO:
 * Decidí estructurar varios componentes del juego usando este TDA para manejar
 * colecciones dinámicas de forma más limpia:
 * 1. Tablero (board.h): `tBoard` usa un vector para guardar las cartas (`tCard`).
 * 2. Jugadores (game.h): `tGame` maneja a los jugadores de la partida (`tPlayer`) con un vector.
 * 3. Ranking (archive.c): Lo uso para levantar los puntajes del archivo, ordenarlos y quedarme con el TOP 5.
 * 4. Etiquetas de Menú (screen.h): En `tMainMenu`, un vector maneja las etiquetas (`tLabel`) del podio visualmente.
 * 5. Gestor de Pantallas (main.c): Implementé la navegación entre los distintos menús y la partida
 *    apilando estados (como un Stack) haciendo push y pop sobre este vector.
 * 6. MultiPlayer (screen.h): Me permite gestionar la interfaz dinámica cuando se agregan jugadores al juego.
 */

#include <stddef.h>

typedef struct
{
  void *data;       // Puntero al bloque de memoria dinámica donde se guardan los
                    // elementos
  size_t size;      // Cantidad de elementos actualmente cargados en el vector
  size_t capacity;  // Cantidad de elementos que pueden almacenarse sin realocar
                    // memoria
  size_t elem_size; // Tamaño en bytes de cada elemento (ej: sizeof(tCard))
} tVector;

int vector_init(tVector *v, size_t elem_size);
// Prepara el vector reservando la memoria inicial necesaria según el tamaño del dato.

int vector_reserve(tVector *v, size_t new_capacity); // AUMENTA CAPAC. DEL VECTOR
// Si el vector se queda corto, esta función tira de realloc para agrandar el buffer sin perder la data.

int vector_push_back(tVector *v, const void *elem); // AGREGA ELEM AL FINAL
// Agrega un nuevo elemento al final del bloque. Si justo nos quedamos sin espacio, 
// llama a vector_reserve internamente para duplicar la capacidad.

void *vector_get(const tVector *v, size_t index); // DEVUELVE POS DE UN VECTOR
// Me calcula el desplazamiento en memoria exacto para llegar al elemento en `index`.

size_t vector_size(const tVector *v); // DEVUELVE CANT DE ELEM CARGADOS
// Forma segura de consultar cuántos elementos hay cargados.

void vector_destroy(tVector *v); // LIBERA MEM
// Es mandatorio llamar esto cuando el vector ya no sirve más para evitar memory leaks.

void vector_pop(tVector *v); // Elimina el ultimo elemento (Reduce size)
// Útil cuando uso el vector como una Pila / Stack (por ejemplo, en el control de pantallas).

void *vector_lastElement(tVector *v); // Da la direccion de memoria del ultimo elemento
// Retorna lo que está en la cima del "Stack".

void vector_sort(tVector *v, int (*cmp)(const void *, const void *)); // Ordena el TDA
// Recibe un puntero a función para comparar. Lo usé bastante para el sistema de ranking.

#endif
