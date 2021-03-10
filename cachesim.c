#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "cachesim.h"

int main(int argc, char *argv[]){ 
    argumentos_t params;
    if (!parse_args(argc, argv, &params)){
        printf("\nERROR: Numero invalido de argumentos\n");
        return 1;
    }

    cache_t* cache = crear_cache(params);
    stats_t* stats = crear_stats(0);
    FILE* tracefile = fopen(params.file, "r");

    if (tracefile == NULL){
        printf("\nERROR: El archivo de trazas especificado no existe\n");
        return 1;
    }

    char* line = NULL;
    size_t len = 0;

    if (cache != NULL){
        while(getline(&line, &len, tracefile) != -1){
            trace_t t;
            if (!parse_trace(line, &t, cache->S, cache->B)){
                printf("\nERROR: Formato de traza invalida\n");
                return 1;
            }
            cache_op(cache, t.op, t.address, stats);
        }
        free(line);
        fclose(tracefile);
        print_stats(cache, stats);
        free(stats);
        cache_destruir(cache);
        return 0;

    }else{
        printf("\nERROR: Parametros invalidos, C, E y S deben ser potencia de 2\n");
        return 1;
    }
}

bool parse_args(int n_arg, char* args[], argumentos_t* params){
    if ((n_arg == MIN_ARG) || (n_arg == MAX_ARG)){ // Nos aseguramos de que haya 4 o 7 args
        params->file = args[1];
        params->C = atoi(args[2]);
        params->E = atoi(args[3]);
        params->S = atoi(args[4]);
        if (n_arg == MAX_ARG){
            params->n = atoi(args[6]);
            params->m = atoi(args[7]);
        }
        return true;
    }
    else
        return false;
}

cache_t* crear_cache(argumentos_t params){
    if (all_power2(params)){    // Nos aseguramos que los argumentos sean potencia de 2
        cache_t* cache = malloc(sizeof(cache_t));
        cache->C = params.C;
        cache->E = params.E;
        cache->S = params.S;
        cache->B = params.C / (params.E * params.S);
        cache->m = (params.m || params.m == 0) ? params.m : -1;
        cache->n = (params.n || params.n == 0) ? params.n : -1;
        cache->time = 0;
        cache->sets = malloc(params.S * sizeof(set_t));
        for (int i = 0; i < params.S; i++){
            set_t* current_set = cache->sets + i; 
            current_set->lines = malloc(params.E * sizeof(line_t));
            for (int j = 0; j < params.E; j++){
                line_t* current_lines = current_set->lines + j;
                current_lines->idx = j;
                current_lines->valid_bit = false;
                current_lines->dirty_bit = false;
                current_lines->time_used = 0;
                current_lines->tag = -1;
            }
        }
        return cache;
    }else{
        return NULL;
    }
}

bool all_power2(argumentos_t params){
    if ((params.C & (params.C - 1)) != 0){
        return false;
    }else if ((params.E & (params.E - 1)) != 0){
        return false;
    }else if ((params.S & (params.S - 1)) != 0){
        return false;
    }else{
        if ((params.C == 0) || (params.E == 0) || (params.S == 0)){
            return false;
        }else{
            return true;
        }
    }
}

bool parse_trace(char* line, trace_t* trace_read, int S, int B){
    unsigned int addr;
    char letra;
    int n = sscanf(line, "%x: %c %x %d %*x\n",
                      &trace_read->ip,
                      &letra,
                      &addr,
                      &trace_read->n_bytes);
    if (letra == 'W'){
        trace_read->op = WRITE;
    }else{
        trace_read->op = READ;
    }
    trace_read->address = parse_address(addr, S, B);
    if (n == 4)
        return true;
    else
        return false;
}

address_t parse_address(unsigned int address, int S, int B){
    address_t address_read;
    int s = log2(S);
    int b = log2(B);
    int s_mask = S - 1;
    address_read.tag = address >> (s + b);
    address_read.set_index = (address >> b) & s_mask;
    return address_read;
}

void cache_op(cache_t* cache, operation_t op, address_t address, stats_t* stats){
    verboso_t verboso;
    verboso.indice = cache->time;
    verboso.cache_index = address.set_index;
    verboso.cache_tag = address.tag;

    line_t* target_line = fetch_line(cache, address);   // Traemos una linea sobre la que vamos a operar

    verboso.cache_line = target_line->idx;
    verboso.line_tag = target_line->tag;
    verboso.valid_bit = target_line->valid_bit;
    verboso.dirty_bit = target_line->dirty_bit;
    verboso.last_used = target_line->time_used;

    const char* caso = id_caso(target_line, address.tag);   // Identificamos a que caso corresponde la operacion

    target_line->time_used = cache->time;   // Actualizamos el LU de la linea con el tiempo global
    cache->time += 1;   // Aumentamos el tiempo global en cada operacion

    if(target_line != NULL){

        if(strcmp( caso, "1") == 0){
            if (op == WRITE){
                target_line->dirty_bit = true;
                stats->cycles_w += 1;
                stats->stores += 1;
            }else{
                stats->cycles_r += 1;
                stats->loads += 1;
            }
            verboso.caso = HIT;
        }
        
        if(strcmp( caso, "2a") == 0){
            if (op == WRITE){
                target_line->dirty_bit = true;
                stats->cycles_w += (1 + PENALTY);
                stats->stores += 1;
                stats->wmiss += 1;
            }else{
                stats->cycles_r += (1 + PENALTY);
                stats->loads += 1;
                stats->rmiss += 1;
            }
            target_line->valid_bit = true;
            target_line->tag = address.tag;
            verboso.caso = CMISS;
        }

        if(strcmp( caso, "2b") == 0){
            if (op == WRITE){
                target_line->dirty_bit = true;
                stats->cycles_w += (1 + 2 * PENALTY);
                stats->stores += 1;
                stats->wmiss += 1;
                stats->dirty_wmiss += 1;
            }else{
                target_line->dirty_bit = false;
                stats->cycles_r += (1 + 2 * PENALTY);
                stats->loads += 1;
                stats->rmiss += 1;
                stats->dirty_rmiss += 1;
            }
            target_line->tag = address.tag;
            verboso.caso = DMISS;
        }

        if(cache->m != -1 && cache->n != -1){
            if((cache-> time >= cache->n) && (cache-> time <= (cache->m + 1))){
                print_verboso(verboso, cache->E);
            }
        }

    }else{
        printf("ERROR: Se solicito un set inexistente");
    }
}

line_t* fetch_line(cache_t* cache, address_t address){

    int i = address.set_index;
    set_t* target_set = cache->sets + i;
    line_t* target_line;
    int lowest_time = MAX_TIME;
    int to_eject = 0;   // Aca guardamos la linea con menor LU, por si el set esta lleno
    int empty_line = cache->E + 1;  // Aca guardamos, si existe, la linea vacia con menor indice
    bool empty = false; // Aca anotamos si el set tiene alguna linea vacia 

    if (i <= (cache->S)){
        line_t* current_line = target_set->lines;
        for (int j = 0; j < cache->E; j++){  // Buscamos en las lineas sel set pedido
            line_t* current_line = target_set->lines + j;
            if (current_line->valid_bit){  // Buscamos lineas validas
                if (current_line->tag == address.tag){  // Si coincide el tag es un HIT
                    target_line = current_line;
                    return target_line;
                }else if(current_line->time_used < lowest_time){    // Si no coincide el tag nos fijamos el LU 
                    lowest_time = current_line->time_used;
                    to_eject = j;    // Si el LU es el menor hasta ahora lo guardamos por si hay que desalojar
                }
            }else if(j < empty_line){
                empty_line = j;
                empty = true;   // Guardamos la linea vacia con menor indice
            }
        }
        if (empty){
            target_line = current_line + empty_line;    // Si no hubo HIT y hay linea vacia
        }else{
            target_line = current_line + to_eject;  // Si no hubo HIT y hay que desalojar
        }
        return target_line;
    }else{
        return NULL;
    }
}

const char* id_caso(line_t* line, unsigned int tag){
    if (line->valid_bit == true){  // Hay varias posibilidades
        if(line->tag == tag){ // Si coincide el tag es HIT
            return "1";
        }else{
            if (line->dirty_bit == true){ // Si no coincide el tag y esta dirty es DIRTY MISS
                return "2b";
            }else{  // Si no coincide el tag y no esta dirty es CLEAN MISS
                return "2a";
            }
        }
    }else{  // Si la linea esta vacia es CLEAN MISS
        return "2a";
    }
}

void print_verboso(verboso_t verboso, int E){
    char* caso;
    if(verboso.caso == HIT)caso = "1";
    else if(verboso.caso == CMISS)caso = "2a";
    else caso = "2b";

    if(E > 1){
        char* imprimir = "%d %s %x %x %d %x %d %d %d \n";
        if(verboso.line_tag == -1)imprimir = "%d %s %x %x %d %d %d %d %d \n";
        fprintf (stdout, imprimir   ,verboso.indice 
                                    ,caso
                                    ,verboso.cache_index
                                    ,verboso.cache_tag
                                    ,verboso.cache_line
                                    ,verboso.line_tag
                                    ,verboso.valid_bit
                                    ,verboso.dirty_bit
                                    ,verboso.last_used
                                    );
    }else{
        char* imprimir = "%d %s %x %x %d %x %d %d \n";
        if(verboso.line_tag == -1)imprimir = "%d %s %x %x %d %d %d %d \n";
        fprintf (stdout, imprimir   ,verboso.indice 
                                    ,caso
                                    ,verboso.cache_index
                                    ,verboso.cache_tag
                                    ,verboso.cache_line
                                    ,verboso.line_tag
                                    ,verboso.valid_bit
                                    ,verboso.dirty_bit
                                    );
    } 
}

void print_stats(cache_t* cache, stats_t* stats){
    if (cache->E == 1){
        printf("direct-mapped, %d sets, size = %dKB\n", cache->S, (cache->C / 1000));
    }else{
        printf("%d-way, %d sets, size = %dKB\n", cache->E, cache->S, (cache->C / 1000));
    }
    int total_access = stats->loads + stats->stores;
    int total_misses = stats->rmiss + stats->wmiss;
    int total_dirty_misses = (stats->dirty_wmiss + stats->dirty_rmiss);
    float miss_rate = (float)total_misses / (float)total_access;
    int bytes_r = total_misses * cache->B;
    int bytes_w = total_dirty_misses * cache->B;
    printf("loads %d stores %d total %d\n", stats->loads, stats->stores, total_access);
    printf("rmiss %d wmiss %d total %d\n", stats->rmiss, stats->wmiss, total_misses);
    printf("dirty rmiss %d dirty wmiss %d\n", stats->dirty_rmiss, stats->dirty_wmiss);
    printf("bytes read %d bytes written %d\n", bytes_r, bytes_w);
    printf("read time %d write time %d\n", stats->cycles_r, stats->cycles_w);
    printf("miss rate %f\n", miss_rate);
}

void cache_destruir(cache_t* cache){
    for (int i = 0; i < cache->S; i++){
        free(cache->sets[i].lines);
    }
    free(cache->sets);
    free(cache);
}

stats_t* crear_stats(int i){
    stats_t* stats = malloc(sizeof(stats_t));
    stats->cycles_r = i;
    stats->cycles_w = i;
    stats->dirty_rmiss = i;
    stats->dirty_wmiss = i;
    stats->loads = i;
    stats->stores = i;
    stats->rmiss = i;
    stats->wmiss = i;
    return stats;
}