#include "recode57.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NBUF 32   // Tamaño de buffer para la corrección.
#define MAXBOM 4  // Bytes que ocupa como máximo un byte-order-mark.


/*
 * Devuelve el encoding correspondiente al argumento.
 */
static enum encoding str_to_enc(const char *enc) {
    if (strcmp(enc, "UTF-8") == 0)
        return UTF8;
    else if (strcmp(enc, "UTF-16BE") == 0)
        return UTF16BE;
    else if (strcmp(enc, "UTF-16LE") == 0)
        return UTF16LE;
    else if (strcmp(enc, "UTF-32BE") == 0)
        return UTF32BE;
    else if (strcmp(enc, "UTF-32LE") == 0)
        return UTF32LE;
    else
        return UNKNOWN;
}


/*
 * Decodifica la entrada, convirtiéndola a una secuencia de codepoints.
 *
 * Argumentos:
 *
 *   - enc: el encoding de los datos en buf.
 *   - buf: los bytes a decodificar.
 *   - nbytes: el número de bytes disponibles en buf.
 *   - destbuf: codepoints resultantes de la conversión.
 *
 * Devuelve: el número de CODEPOINTS obtenidos (número
 *           de elementos escritos en destbuf).
 *
 * Actualiza: nbytes contiene el número de bytes que no se
 *            pudieron consumir (secuencia o surrogate incompleto).
 *
 * Se debe garantiza que "destbuf" puede albergar al menos nbytes
 * elementos (caso enc=UTF-8, buf=ASCII).
 */
static size_t decode_input(enum encoding enc,
                           uint8_t *buf,
                           size_t *nbytes,
                           codepoint_t *destbuf) {
    size_t b = 0, n = 0;

    while ((b = read_codepoint(enc, buf, *nbytes, &destbuf[n])) > 0) {
        n++;
        buf += b;
        *nbytes -= b;
    }

    return n;
}


/*
 * Codifica una secuencia de codepoints al encoding especificado.
 *
 * Argumentos:
 *
 *   - enc: el encoding destino.
 *   - input: los codepoints a codificar.
 *   - npoints: el número de codepoints en input.
 *   - output: resultado de la conversión.
 *
 * Devuelve: el número de BYTES que resultaron de la codificación
 *           (número de bytes escritos en destbuf).
 *
 * Se debe garantizar que "destbuf" puede albergar al menos npoints*4 bytes.
 */
static size_t encode_output(enum encoding enc,
                            codepoint_t *input,
                            size_t npoints,
                            uint8_t *outbuf) {
    size_t b = 0;

    for (size_t i = 0; i < npoints; i++) {
        b += write_codepoint(enc, input[i], &outbuf[b]);
    }

    return b;
}


static size_t BOM_SIZES[] = {
    // clang-format off
    [UTF8] = 0,
    [UTF16BE] = 2,
    [UTF16LE] = 2,
    [UTF32LE] = 4,
    [UTF32BE] = 4,
    // clang-format on
};


int main(int argc, char *argv[]) {
    enum encoding orig_enc, dest_enc;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <encoding>\n", argv[0]);
        return 1;
    }

    if ((dest_enc = str_to_enc(argv[1])) >= UNKNOWN) {
        fprintf(stderr, "Encoding no válido: %s\n", argv[1]);
        return 1;
    }

    // Detectar codificación origen con byte order mark.
    uint8_t bom[MAXBOM];
    size_t bom_bytes;

    read(STDIN_FILENO, bom, MAXBOM);
    orig_enc = bom_to_enc(bom);
    bom_bytes = BOM_SIZES[orig_enc];

    // Buffers para entrada, salida, y conversión intermedia a codepoints.
    uint8_t inbuf[NBUF];
    uint8_t outbuf[NBUF * 4 + MAXBOM];  // Caso peor: UTF-32 + BOM.
    codepoint_t codepoints[NBUF];       // Caso peor: ASCII.

    // Si orig_enc no fue UTF-32, quedaron 2 o 4 bytes en "bom" que
    // deben ser prefijados en inbuf.
    size_t prevbytes = MAXBOM - bom_bytes;
    ssize_t inbytes;

    memcpy(inbuf, bom + bom_bytes, prevbytes);

    // La última tarea entes de entrar en el loop, es escribir el nuevo BOM.
    bom_bytes = write_bom(dest_enc, outbuf);

    while ((inbytes = read(STDIN_FILENO, inbuf + prevbytes,
                           sizeof(inbuf) - prevbytes)) > 0) {
        inbytes += prevbytes;
        prevbytes = inbytes;
        // fprintf(stderr, "Processing: %zu bytes, ", prevbytes);

        size_t npoints = decode_input(orig_enc, inbuf, &prevbytes, codepoints);
        size_t outbytes =
            encode_output(dest_enc, codepoints, npoints, outbuf + bom_bytes);
        // fprintf(stderr, "codepoints: %d, output: %d bytes, remaining: %zu bytes\n",
        //         npoints, outbytes, prevbytes);

        write(STDOUT_FILENO, outbuf, outbytes + bom_bytes);
        bom_bytes = 0;  // Solo se usa en la primera iteración.

        if (prevbytes > 0) {
            memcpy(inbuf, inbuf + inbytes - prevbytes, prevbytes);
        }
    }
}
