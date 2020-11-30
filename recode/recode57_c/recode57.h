#ifndef RECODE57_H
#define RECODE57_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t codepoint_t;

enum encoding { UTF8, UTF16BE, UTF16LE, UTF32BE, UTF32LE, UNKNOWN };


/*
 * Determina la codificación correspondiente a un byte order mark.
 *
 * Argumentos:
 *   - bom: un arreglo con los primeros 4 bytes del archivo.
 *
 * Devuelve:
 *   - el encoding detectado según el byte order mark (BOM) recibido.
 */
enum encoding bom_to_enc(uint8_t *bom);


/*
 * Escribe el byte order mark correspondiente a una codificación.
 *
 * Argumentos:
 *   - enc: el encoding cuyo byte order mark debe escribirse.
 *   - buf: el arreglo donde escribir el byte order mark; contiene
 *          espacio para al menos 4 bytes.
 *
 * Devuelve:
 *   - el número de bytes escritos, o 0 si no se escribió BOM (por
 *     ejemplo en el caso de UTF-8).
 */
size_t write_bom(enum encoding enc, uint8_t *buf);


/*
 * Determina si un buffer contiene un codepoint en el encoding especificado.
 *
 * Argumentos:
 *   - enc: el encoding de los bytes en buf.
 *   - buf: arreglo con los bytes a decodificar.
 *   - nbytes: el número de bytes en buf.
 *
 * Devuelve:
 *   - verdadero o falso según buf pueda producir un codepoint, considerando el
 *     encoding especificado. Por ejemplo, solo devolverá verdadero para UTF-32
 *     si nbytes >= 4. Y siempre devolverá falso para UTF-16 si n < 2 (entre
 *     otras condiciones).
 */
bool has_codepoint(enum encoding enc, uint8_t *buf, size_t nbytes);


/*
 * Decodifica UN codepoint a partir de una secuencia de bytes.
 *
 * Argumentos:
 *   - enc: el encoding de los bytes en buf.
 *   - buf: arreglo con los bytes a decodificar.
 *   - nbytes: el número de bytes disponibles en buf.
 *   - dest: región de memoria donde escribir el codepoint obtenido.
 *
 * Devuelve:
 *   - el número de bytes consumidos para decodificar el codepoint (por
 *     ejemplo, siempre 4 en UTF-32; 2 o 4 en UTF-16; de 1 a 4 para UTF-8).
 *
 *   - se devuelve 0 si no había suficientes bytes para decodificar un codepoint
 *     (esto ocurre para UTF-32 si nbytes < 4; para UTF-16 si nbytes < 2, o si
 *     nbytes < 4 y el primer codepoint es un surrogate; etc.); esta condición
 *     es detectada por la función auxiliar has_codepoint(enc, buf, nbytes).
 */
size_t read_codepoint(enum encoding enc,
                      uint8_t *buf,
                      size_t nbytes,
                      codepoint_t *dest);

/*
 * Codifica un codepoint a una secuencia de bytes.
 *
 * Argumentos:
 *   - enc: el encoding en el que desea codificar el codepoint.
 *   - codepoint: el valor numérico del codepoint a codificar.
 *   - outbuf: el buffer donde escribir el resultado de la codificación
 *             (este buffer siempre puede albergar hasta 4 bytes).
 *
 * Devuelve:
 *   - el número de bytes escritos en "outbuf" (siempre entre 1 y 4).
 */
size_t write_codepoint(enum encoding enc,
                       codepoint_t codepoint,
                       uint8_t *outbuf);

#endif
