#!/bin/bash

# Si se invoca el script con el programa como parámetro:
#
#     $ ../test_recode.sh ./recode57
#
# se imprime un resumen de todos los tests.
#
# Con el argumento --diff:
#
#     $ ../test_recode.sh ./recode57 --diff
#
# se muestran las diferencias en hexadecimal entre lo obtenido y lo
# esperado.
#
# Notas: para cada codificación, se comprueba también la conversión de
# esa codificación, a la *misma* codificación. Esto sirve para depurar
# cada codificación por separado, cuando no está claro en cuál está el
# error.
#
# O, usando el Makefile proporcionado, se puede compilar y correr en
# en una sola instrucción con:
#
#     $ make test
#
# Se pueden comentar las llamadas a "test_type" al final del archivo
# para controlar qué categoría de archivos probar.

set -eu

PROG="${1}"
DIFF="${2-}"
TESTDATA="$(readlink -f ${BASH_SOURCE%/*})/testdata"
ENCODINGS="UTF-8 UTF-16BE UTF-16LE UTF-32BE UTF-32LE"

# Esta variable ignora no incluye UTF-8; descomentar probar solo
# las codificaciones de ancho fijo.
# ENCODINGS="UTF-32LE UTF-32BE UTF-16LE UTF-16BE"

cmp_output() {
    local type="$1"
    local orig_enc="$2"
    local dest_enc="$3"
    local input="$TESTDATA/${type}_${orig_enc}.txt"
    local expected="$TESTDATA/${type}_${dest_enc}.txt"

    ./$PROG "$dest_enc" <"${input}" 2>/dev/null | cmp -s "$expected" -
}

diff_hex() {
    local type="$1"
    local orig_enc="$2"
    local dest_enc="$3"
    local input="$TESTDATA/${type}_${orig_enc}.txt"
    local expected="$TESTDATA/${type}_${dest_enc}.txt"

    diff -u --label esperado --label obtenido \
        <(od -An -t x1 < "$expected")         \
        <(./$PROG "$dest_enc" < "$input" | od -An -t x1) || true
}

test_type() {
    local type="$1"
    local pass=0
    local total=0
    echo "=== TEST: '$type' ==="

    for src in $ENCODINGS; do
        for dst in $ENCODINGS; do
            # if [[ $src != $dst ]]; then
                ((++total))
                echo -n "$src -> $dst: "
                if cmp_output "$type" "$src" "$dst"; then
                    ((++pass))
                    echo -e "\e[92mOK\e[0m"
                else
                    echo -e "\e[91mFAIL\e[0m"
                    if [[ $DIFF == "--diff" ]]; then
                        diff_hex "$type" "$src" "$dst"
                    fi
                fi
            # fi
        done
    done

    # Imprimir resumen.
    echo -e "[OK: $pass/$total]\n"

    # Devolver éxito solo si pasan todos los tests.
    [ $pass -eq $total ]
}

test_type "simple"
test_type "bmp"
test_type "surrogate"
