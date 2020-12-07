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
            else if ((nbytes >= 2) && (0xD800 > ((buf[0] << 8) | buf[1]))
                     && (((buf[0] << 8) | buf[1]) < 0xDBFF))
                result = true;
            else
                result = false;
            break;

        case UTF16LE:
            if (nbytes >= 4)
                result = true;
            else if (nbytes < 2)
                result = false;
            else if ((nbytes >= 2) && (0xD800 > ((buf[1] << 8) | buf[0]))
                     && (((buf[1] << 8) | buf[0]) < 0xDBFF))
                result = true;
            else
                result = false;
            break;

        case UTF8:
            if (nbytes < 1)
                result = false;
            else if ((nbytes == 1) && ((buf[0]) & 0x80) == 0)
                result = true;
            else if ((nbytes >= 2) && ((buf[0]) & 0xC0) == 0)
                result = true;
            else if ((nbytes >= 3) && ((buf[0]) & 0xE0) == 0)
                result = true;
            else if ((nbytes >= 4) && ((buf[0]) & 0xF0) == 0)
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

    codepoint_t cp = 0;
    codepoint_t hs;
    codepoint_t ls;
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
                hs = ((buf[0] << 8) | buf[1]);
                if ((hs < 0xD800) || (hs > 0xE000)){
                    cp = hs;
                    n = 2;
                }else if (nbytes >= 4){
                    ls = ((buf[2] << 8) | buf[3]);
                    ls -= 0xDC00;
                    hs -= 0xD800;
                    hs *= 0x400;
                    cp = hs + ls;
                    cp += 0x10000;
                    n = 4;
                }break;

            case UTF16LE:
                hs = ((buf[1] << 8) | buf[0]);
                if ((hs < 0xD800) || (hs > 0xE000)){
                    cp = hs;
                    n = 2;
                }else if (nbytes >= 4){
                    ls = ((buf[3] << 8) | buf[2]);
                    ls -= 0xDC00;
                    hs -= 0xD800;
                    hs *= 0x400;
                    cp = hs + ls;
                    cp += 0x10000;
                    n = 4;
                }break;

            case UTF8:
                if (buf[0] <= 0x7F){
                    cp = buf[0];
                    n = 1;
                }else if ((nbytes > 1) && ((buf[0]) & 0x20) == 0){
                    cp = ((buf[0] & 0x1F) << 6) | (buf[1] & 0x3F);
                    n = 2;
                }else if ((nbytes > 2) && ((buf[0]) & 0x10) == 0){
                    cp = ((buf[0] & 0xF) << 12) | ((buf[1] & 0x3F) << 6);
                    cp |= (buf[2] & 0x3F);
                    n = 3;
                }else if ((nbytes > 3) && ((buf[0]) & 0x8) == 0){
                    cp = ((buf[0] & 0x7) << 18) | ((buf[1] & 0x3F) << 12);
                    cp |= ((buf[2] & 0x3F) << 6) | (buf[3] & 0x3F);
                    n = 4;
                }break;
        }
    }
    *dest = cp;
    return n;
}

size_t write_codepoint(enum encoding enc,
                       codepoint_t codepoint,
                       uint8_t *outbuf){

        codepoint_t a = 0;
        codepoint_t b = 0;
        codepoint_t c = 0;
        size_t n = 0;

        switch(enc){
            case UTF32BE:
                outbuf[3] = codepoint;
                outbuf[2] = codepoint >> 8;
                outbuf[1] = codepoint >> 16;
                outbuf[0] = codepoint >> 24;
                n = 4;
                break;

            case UTF32LE:
                outbuf[0] = codepoint;
                outbuf[1] = codepoint >> 8;
                outbuf[2] = codepoint >> 16;
                outbuf[3] = codepoint >> 24;
                n = 4;
                break;

            case UTF16BE:
                if (codepoint <= 0xFFFF){
                    outbuf[1] = codepoint;
                    outbuf[0] = codepoint >> 8;
                    n = 2;
                }else{
                    a = codepoint - 0x010000;
                    b = (a >> 10) + 0xD800;
                    c = (a << 22 >> 22) + 0xDC00;

                    outbuf[0] = b >> 8 ;
                    outbuf[1] = b ;
                    outbuf[2] = c >> 8 ;
                    outbuf[3] = c ;

                    n = 4;
                }
                break;

            case UTF16LE:
                if (codepoint <= 0xFFFF){
                    outbuf[0] = codepoint;
                    outbuf[1] = codepoint >> 8;
                    n = 2;
                }else{
                    a = codepoint - 0x010000;
                    b = (a >> 10) + 0xD800;
                    c = (a << 22 >> 22) + 0xDC00;

                    outbuf[1] = b >> 8 ;
                    outbuf[0] = b ;
                    outbuf[3] = c >> 8 ;
                    outbuf[2] = c ;

                    n = 4;
                }
                break;

            case UTF8:
                if (codepoint <= 0x7F){
                    outbuf[0] = codepoint;
                    n = 1;
                }else if (codepoint <= 0x7FF){
                    outbuf[1] = 0x80 | (codepoint & 0x3F);
                    outbuf[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                    n = 2;
                }else if (codepoint <= 0xFFFF){
                    outbuf[2] = 0x80 | (codepoint & 0x3F);
                    outbuf[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                    outbuf[0] = 0xE0 | ((codepoint >> 12) & 0xF);
                    n = 3;
                }else if (codepoint <= 0x10FFFF){
                    outbuf[3] = 0x80 | (codepoint & 0x3F);
                    outbuf[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                    outbuf[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                    outbuf[0] = 0xF0 | ((codepoint >> 24) & 0x7);
                    n = 4;
                }
                break;
        }
        return n;
}