#include <stdio.h>
#include <stdbool.h>

#define PENALTY 100
#define MIN_ARG 5
#define MAX_ARG 8
#define MAX_TIME 999999

typedef struct line{
    int idx;
    unsigned int tag;
    bool valid_bit; 
    bool dirty_bit;
    int time_used;  // Tiempo en el que la linea fue utilizada por ultima vez
} line_t;

typedef struct set{
    line_t* lines; // Array dinamico de lineas
} set_t;

typedef struct cache{
    int time; // Tiempo global
    int C;
    int E;
    int S;
    int B;
    int n;
    int m;
    set_t* sets; // Array dinamico de sets
} cache_t;

typedef struct address{
    int set_index;
    unsigned int tag;
} address_t;

typedef enum operation{READ, WRITE} operation_t;

typedef struct trace{
    int ip;
    operation_t op;
    address_t address;
    int n_bytes;
} trace_t;

typedef struct stats{
    int loads;
    int stores;
    int rmiss;
    int wmiss;
    int dirty_rmiss;
    int dirty_wmiss;
    int cycles_r;
    int cycles_w;
} stats_t;

typedef struct argumentos{
    const char* file;
    int C;
    int E;
    int S;
    int n;
    int m;
} argumentos_t;

typedef enum caso{HIT,CMISS,DMISS} caso_t;

typedef struct verboso{
    int indice;
    caso_t caso;
    unsigned int cache_index; //  Indice del set en hex
    unsigned int cache_tag;
    int cache_line; // Nro de linea en decimal
    int line_tag; // Tag anterior
    int valid_bit; // Valid bit previo 
    int dirty_bit; // Dirty bit previo
    int last_used;
}verboso_t;


/*
Función que parsea una linea del tracefile y la desglosa en un struct,
si lo hace con exito devuelve true, caso contrario devuelve false.
 */
bool parse_trace(char* line, trace_t* trace, int S, int B);

/*
Función que parsea los argumentos pasados por pantalla y los almacena en un struct,
si lo hace con exito devuelve true, caso contrario devuelve false.
 */
bool parse_args(int n_arg, char* args[], argumentos_t* params);

/*
Función que opera en la cache variando el comportamiento segun la operacion solicitada,
que puede ser WRITE o READ. Ademas actualiza las estadisticas segun corresponda.
 */
void cache_op(cache_t* cache, enum operation, address_t address, stats_t* stats);

/*
Función que crea la cache y se fija que los argumentos sean multiplos de 2.
 */
cache_t* crear_cache(argumentos_t params);

/*
Función que destruye la cache, liberando la memoria que tenia asignada.
 */
void cache_destruir(cache_t* cache);

/*
Función que parsea la direccion de una instruccion y la almacena en un struct.
 */
address_t parse_address(unsigned int address, int S, int B);

/*
Función que imprime las estadisticas con el formato especificado para el TP.
 */
void print_stats(cache_t* cache, stats_t* stats);

/*
Función que busca y devuelve siempre una linea segun lo pedido en la direccion.
 */
line_t* fetch_line(cache_t* cache, address_t address);

/*
Función que crea el struct donde se guardan las estadisticas de la simulacion.
 */
stats_t* crear_stats(int i);

/*
Función que devuelve un identificador segun el caso de una instruccion: hit,
clean miss o dirty miss (1, 2a y 2b respectivamente).
 */
const char* id_caso(line_t* line, unsigned int tag);

/*
Función que imprime por pantalla las lineas del modo verboso con el formato 
especificado para el TP.
 */
void print_verboso(verboso_t verboso, int E);

/*
Función que chequea si alguno de los parámetros C, E o S no son potencia de dos.
 */
bool all_power2(argumentos_t params);