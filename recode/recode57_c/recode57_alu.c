#include "recode57.h"

enum encoding bom_to_enc(uint8_t *bom){

    enum encoding enc;

    if ((bom[0] == 0xFE) && (bom[1] == 0xFF))
        enc = UTF16BE;

    else if ((bom[0] == 0x0) && (bom[1] == 0x0) &&
        (bom[2] == 0xFE) && (bom[3] == 0xFF))
        enc = UTF32BE;

    else if ((bom[0] == 0xFF) && (bom[1] == 0xFE) &&
        (bom[2] == 0x0) && (bom[3] == 0x0))
        enc = UTF32LE;

    else if ((bom[0] == 0xFF) && (bom[1] == 0xFE))
        enc = UTF16LE;

    else
        enc = UTF8;

    return enc;
}

size_t write_bom(enum encoding enc, uint8_t *buf){

    size_t n = 0;

    switch(enc){
        case UTF32BE:
            buf[0] = 0x0;
            buf[1] = 0x0;
            buf[2] = 0xFE;
            buf[3] = 0xFF;
            n = 4;
            break;

        case UTF32LE:
            buf[0] = 0xFF;
            buf[1] = 0xFE;
            buf[2] = 0x0;
            buf[3] = 0x0;
            n = 4;
            break;

        case UTF16BE:
            buf[0] = 0xFE;
            buf[1] = 0xFF;
            n = 2;
            break;

        case UTF16LE:
            buf[0] = 0xFF;
            buf[1] = 0xFE;
            n = 2;
            break;

        case UTF8:
            n = 0;
            break;
    }
    return n;
}

bool has_codepoint(enum encoding enc, uint8_t *buf, size_t nbytes){

    bool result = false;

    switch(enc){
        case UTF32BE:
        case UTF32LE:
            if (nbytes >= 4)
                result = true;
            else
                result = false;
            break;

        case UTF16BE:
            if (nbytes >= 4)
                result = true;
            else if (nbytes < 2)
                result = false;
            else if (nbytes >= 2) // FALTA HACER ESTA COMPARACION: fijarse que no es high surrogate buf[0:1]
                result = true;
            else
                result = false;
            break;

        case UTF16LE:
            if (nbytes >= 4)
                result = true;
            else if (nbytes < 2)
                result = false;
            else if (nbytes >= 2) // FALTA HACER COMPARACION: fijarse que no es high surrogate buf[1:0]
                result = true;
            else
                result = false;
            break;

        case UTF8:
            if (nbytes < 1)
                result = false;
            else if ((nbytes == 1) && ((buf[0]) & 0x80) == 0)
                result = true;
            else if ((nbytes == 2) && ((buf[1]) & 0x20) == 0)
                result = true;
            else if ((nbytes == 3) && ((buf[2]) & 0xA95F60) == 0)
                result = true;
            else if ((nbytes == 4) && ((buf[3]) & 0x8) == 0)
                result = true;
            else
                result = true;
            break;
    }
    return result;
}

size_t read_codepoint(enum encoding enc,
                      uint8_t *buf,
                      size_t nbytes,
                      codepoint_t *dest){

    codepoint_t cp;
    size_t n = 0;

    if (has_codepoint(enc, buf, nbytes)){
        switch(enc){
            case UTF32BE:
                cp = buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
                n = 4;
                break;

            case UTF32LE:
                cp = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
                n = 4;
                break;

            case UTF16BE:
                break;

            case UTF16LE:
                break;

            case UTF8:
                break;
        }
    }
    *dest = cp;
    return n;
}

size_t write_codepoint(enum encoding enc,
                       codepoint_t codepoint,
                       uint8_t *outbuf){
    size_t n = 0;

    if(enc == UTF32LE){
        outbuf[0] = codepoint;
        outbuf[1] &= codepoint >> 8;
        outbuf[2] &= codepoint >> 16;
        outbuf[3] &= codepoint >> 24;
        n = 4;
    }
    if(enc == UTF32BE){
        outbuf[3] = codepoint;
        outbuf[2] &= codepoint >> 8;
        outbuf[1] &= codepoint >> 16;
        outbuf[0] &= codepoint >> 24;
        n = 4;
    }
    return n;
}
