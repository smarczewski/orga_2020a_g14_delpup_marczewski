use crate::Codepoint;
use crate::Encoding::{self, *};

// Esta función es idéntica a la versión en C, excepto que también
// devuelve el tamaño del bom que se encontró. Esto permite a main.rs
// procesar los bytes restantes.
pub fn bom_to_enc(bom: &[u8; 4]) -> (Encoding, usize) {
    match bom {
        // ...
        _ => (UTF8, 0),
    }
}

pub fn write_bom(enc: Encoding, buf: &mut [u8; 4]) -> usize {
    match enc {
        // ...
        _ => 0,
    }
}

// Esta función devuelve lo mismo que su equivalente en C (el codepoint, y la
// cantidad de bytes que se usaron para producirlo), preo en una tupla (ya que
// Rust tiene soporte nativo). Además, en lugar de usar el 0 como indicador que
// no había suficientes bytes para producir un codepoint, se usa None, mucho más
// idiomático en Rust.
pub fn read_codepoint(enc: Encoding, input: &[u8]) -> Option<(Codepoint, usize)> {
    let mut cp = 0; // Variable para construir que será devuelto.
    let mut bytes = 0; // Cantidad de bytes consumidos al leer el codepoint.
    let n = input.len(); // Cantidad de bytes disponibles.

    /**
     * Uso opcional: convertir el buffer de entrada a u32, para evitar casts.
    let buf = input[..min(n, 4)]
        .iter()
        .map(|&x| x as u32)
        .collect::<Vec<_>>();
     */

    if bytes > 0 {
        Some((cp, bytes))
    } else {
        None
    }
}

pub fn write_codepoint(enc: Encoding, cp: Codepoint, outbuf: &mut [u8; 4]) -> usize {
    match enc {
        UTF32LE => {
            // ...
            4
        }
        UTF32BE => {
            // ...
            4
        } // ...
        _ => 0, // Borrar una vez implmentados todos los casos.
    }
}
