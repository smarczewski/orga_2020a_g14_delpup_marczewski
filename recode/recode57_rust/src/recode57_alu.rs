use crate::Codepoint;
use crate::Encoding::{self, *};
use std::cmp::min;

// Esta función es idéntica a la versión en C, excepto que también
// devuelve el tamaño del bom que se encontró. Esto permite a main.rs
// procesar los bytes restantes.
pub fn bom_to_enc(bom: &[u8; 4]) -> (Encoding, usize) {

    if (bom[0] == 0xFE) && (bom[1] == 0xFF){
        (UTF16BE, 2)
    }
    else if (bom[0] == 0x0) && (bom[1] == 0x0) &&
    (bom[2] == 0xFE) && (bom[3] == 0xFF){
        (UTF32BE, 4)
    }
    else if (bom[0] == 0xFF) && (bom[1] == 0xFE) &&
    (bom[2] == 0x0) && (bom[3] == 0x0){
        (UTF32LE, 4)
    }
    else if (bom[0] == 0xFF) && (bom[1] == 0xFE){
        (UTF16LE, 2)
    }
    else{
        (UTF8, 0)
    }
}

pub fn write_bom(enc: Encoding, buf: &mut [u8; 4]) -> usize {
    match enc {
        UTF32BE => {
            buf[0] = 0x0;
            buf[1] = 0x0;
            buf[2] = 0xFE;
            buf[3] = 0xFF;
            4
        }
        UTF32LE => {
            buf[0] = 0xFF;
            buf[1] = 0xFE;
            buf[2] = 0x0;
            buf[3] = 0x0;
            4
        }
        UTF16BE => {
            buf[0] = 0xFE;
            buf[1] = 0xFF;
            2
        }
        UTF16LE => {
            buf[0] = 0xFF;
            buf[1] = 0xFE;
            2
        }
        UTF8 => 0
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

    let buf = input[..min(n, 4)]
        .iter()
        .map(|&x| x as u32)
        .collect::<Vec<_>>();

    match enc {
        UTF32BE => {
            if n >= 4 {
                cp = buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
                bytes = 4;
            }else {
                bytes = 0;
            }
        }

        UTF32LE => {
            if n >= 4 {
                cp = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
                bytes = 4;
            }else {
                bytes = 0;
            }
        }

        UTF16BE => {
            let mut hs = (buf[0] << 8) | buf[1];
            if (hs < 0xD800) || (hs > 0xE000) {
                cp = hs;
                bytes = 2;
            }else if n >= 4 {
                let mut ls = (buf[2] << 8) | buf[3];
                ls -= 0xDC00;
                hs -= 0xD800;
                hs *= 0x400;
                cp = hs + ls;
                cp += 0x10000;
                bytes = 4;
            }
        }

        UTF16LE => {
            let mut hs = (buf[1] << 8) | buf[0];
            if (hs < 0xD800) || (hs > 0xE000) {
                    cp = hs;
                    bytes = 2;
            }else if n >= 4 {
                let mut ls = (buf[3] << 8) | buf[2];
                ls -= 0xDC00;
                hs -= 0xD800;
                hs *= 0x400;
                cp = hs + ls;
                cp += 0x10000;
                bytes = 4;
            }
        }

        UTF8 => {
            if buf[0] <= 0x7F {
                cp = buf[0];
                bytes = 1;
            }else if (n > 1) && ((buf[0]) & 0x20) == 0 {
                cp = ((buf[0] & 0x1F) << 6) | (buf[1] & 0x3F);
                bytes = 2;
            }else if (n > 2) && ((buf[0]) & 0x10) == 0 {
                cp = ((buf[0] & 0xF) << 12) | ((buf[1] & 0x3F) << 6);
                cp |= buf[2] & 0x3F;
                bytes = 3;
            }else if (n > 3) && ((buf[0]) & 0x8) == 0 {
                cp = ((buf[0] & 0x7) << 18) | ((buf[1] & 0x3F) << 12);
                cp |= ((buf[2] & 0x3F) << 6) | (buf[3] & 0x3F);
                bytes = 4;
            }
        }
    }

    if bytes > 0 {
        Some((cp, bytes))
    } else {
        None
    }
    
}

pub fn write_codepoint(enc: Encoding, cp: Codepoint, outbuf: &mut [u8; 4]) -> usize {

    match enc {
        UTF32BE => {
            outbuf[3] = cp as u8;
            outbuf[2] = (cp >> 8) as u8;
            outbuf[1] = (cp >> 16) as u8;
            outbuf[0] = (cp >> 24) as u8;
            4
        }
        UTF32LE => {
            outbuf[0] = cp as u8;
            outbuf[1] = (cp >> 8) as u8;
            outbuf[2] = (cp >> 16) as u8;
            outbuf[3] = (cp >> 24) as u8;
            4
        }
        UTF16BE => {
            if cp <= 0xFFFF {
                outbuf[1] = cp as u8;
                outbuf[0] = (cp >> 8) as u8;
                2
            }else {
                let a = cp - 0x010000;
                let b = (a >> 10) + 0xD800;
                let c = (a << 22 >> 22) + 0xDC00;

                outbuf[0] = (b >> 8) as u8;
                outbuf[1] = b as u8;
                outbuf[2] = (c >> 8) as u8;
                outbuf[3] = c as u8;
                4
            }
        }
        UTF16LE => {
            if cp <= 0xFFFF {
                outbuf[0] = cp as u8;
                outbuf[1] = (cp >> 8) as u8;
                2
            }else {
                let a = cp - 0x010000;
                let b = (a >> 10) + 0xD800;
                let c = (a << 22 >> 22) + 0xDC00;

                outbuf[1] = (b >> 8) as u8;
                outbuf[0] = b as u8;
                outbuf[3] = (c >> 8) as u8;
                outbuf[2] = c as u8;
                4
            }
        }
        UTF8 => {
            if cp <= 0x7F {
                outbuf[0] = cp as u8;
                1
            }else if cp <= 0x7FF {
                outbuf[1] = (0x80 | (cp & 0x3F)) as u8;
                outbuf[0] = (0xC0 | ((cp >> 6) & 0x1F)) as u8;
                2
            }else if cp <= 0xFFFF {
                outbuf[2] = (0x80 | (cp & 0x3F)) as u8;
                outbuf[1] = (0x80 | ((cp >> 6) & 0x3F)) as u8;
                outbuf[0] = (0xE0 | ((cp >> 12) & 0xF)) as u8;
                3
            }else if cp <= 0x10FFFF {
                outbuf[3] = (0x80 | (cp & 0x3F)) as u8;
                outbuf[2] = (0x80 | ((cp >> 6) & 0x3F)) as u8;
                outbuf[1] = (0x80 | ((cp >> 12) & 0x3F)) as u8;
                outbuf[0] = (0xF0 | ((cp >> 24) & 0x7)) as u8;
                4
            }else {
                0
            }
        }   
    }
}
