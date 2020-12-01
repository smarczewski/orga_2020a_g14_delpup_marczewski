use std::env;
use std::io::{self, Read, Write};
use std::str::FromStr;

mod recode57_alu;
use recode57_alu::*;

const NBUF: usize = 32; // Tamaño de buffer para la corrección.
const MAXBOM: usize = 4; // Bytes que ocupa como máximo un BOM.

type Codepoint = u32;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Encoding {
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
}

struct ProgArgs {
    dest_enc: Encoding,
}

fn main() -> Result<(), String> {
    let args = parse_args()?;

    // Se detecta la codificación origen por su byte order mark.
    let mut bom = [0u8; MAXBOM];
    io::stdin().read_exact(&mut bom).unwrap();
    let (orig_enc, bom_bytes) = bom_to_enc(&bom);

    // Buffer de entrada: se lee de entrada estándar a inbuf, y, en una función
    // auxiliar, se convertirán los bytes leídos a codepoints como representación
    // intermedia, emitiéndose por salida estándar en el encoding destino. Caso
    // particular: si bom_bytes < 4, quedaron "offset" bytes en "bom" que deben
    // ser prefijados en inbuf.
    let mut inbuf = [0u8; NBUF];
    let mut offset = MAXBOM - bom_bytes;

    if offset > 0 {
        inbuf[..offset].copy_from_slice(&bom[bom_bytes..]);
    }

    // Se escribe el BOM del encoding destino por salida estándar.
    let bom_written = write_bom(args.dest_enc, &mut bom);
    io::stdout().write_all(&bom[..bom_written]).unwrap();

    while let Ok(mut inbytes) = io::stdin().lock().read(&mut inbuf[offset..]) {
        let used;
        inbytes += offset;

        if inbytes > 0 {
            // Convertir los bytes leídos a dest_enc, e imprimirlos por salida
            // estándar.
            used = recode_encode(orig_enc, args.dest_enc, &inbuf[..inbytes]);
            offset = inbytes - used;
        } else {
            break;
        }

        if offset > 0 {
            // Copiar los bytes remanentes de vuelta al principio del buffer.
            inbuf.copy_within(used..inbytes, 0);
        }
    }

    Ok(())
}

/// Helper para parsear los argumentos del programa.
fn parse_args() -> Result<ProgArgs, String> {
    let args: Vec<_> = env::args().collect();

    let dest_enc = match &args[1..] {
        [enc] => Encoding::from_str(enc),
        _ => Err(format!("Uso: {} <encoding>\n", args[0])),
    };

    dest_enc.map(|dest_enc| ProgArgs { dest_enc })
}

/// Helper para codificar la entrada, emitiendo ya el resultado por stdout.
fn recode_encode(orig: Encoding, dest: Encoding, inbuf: &[u8]) -> usize {
    let mut used = 0;
    let mut outbuf = [0u8; 4];

    while let Some((cp, nbytes)) = read_codepoint(orig, &inbuf[used..]) {
        used += nbytes;
        let n = write_codepoint(dest, cp, &mut outbuf);
        io::stdout().write_all(&outbuf[..n]).unwrap();
    }

    // Se devuelve el número de bytes consumidos de inbuf.
    used
}

/// FromStr permite convertir fácilmente el argumento del programa a enum.
impl FromStr for Encoding {
    type Err = String;
    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "UTF-8" => Ok(Encoding::UTF8),
            "UTF-16BE" => Ok(Encoding::UTF16BE),
            "UTF-16LE" => Ok(Encoding::UTF16LE),
            "UTF-32BE" => Ok(Encoding::UTF32BE),
            "UTF-32LE" => Ok(Encoding::UTF32LE),
            _ => Err(format!("Encoding no válido: {:?}", s)),
        }
    }
}
