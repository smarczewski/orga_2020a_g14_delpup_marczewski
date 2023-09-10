# **Organización del Computador (95.57)**

## **Introducción**

Este repositorio contiene todas las actividades prácticas correspondientes a la materia **[9557] Organización del Computador (Curso Moreno) - 2C2020 - FIUBA**  

Los labs son prácticas de menor envergadura que los TPs. El nombre se debe a que asi se refieren a ellos en el libro en el que se basa la cátedra, **Computer Systems: A Programmer's Perspective (CS:APP)**, los labs que provengan del mismo se corresponden a los del libro y al curso de **CMU** de los autores, por ende estan marcados con esas siglas. 

## **Tabla de contenidos**

- [**Organización del Computador (95.57)**](#organización-del-computador-9557)
  - [**Introducción**](#introducción)
  - [**Tabla de contenidos**](#tabla-de-contenidos)
  - [**Lab 1: Datalab (CS:APP)**](#lab-1-datalab-csapp)
  - [**Lab 2: Recode**](#lab-2-recode)
  - [**Lab 3: Lenguaje assembly**](#lab-3-lenguaje-assembly)
    - [**1. asmlab**](#1-asmlab)
    - [**2. bomblab (CS:APP)**](#2-bomblab-csapp)
  - [**TP 1: Simulador de caché**](#tp-1-simulador-de-caché)
    - [**Especificaciones:**](#especificaciones)
      - [Archivo de traza:](#archivo-de-traza)
      - [Línea de comandos:](#línea-de-comandos)
      - [Parámetros fijos:](#parámetros-fijos)
      - [Modo verboso:](#modo-verboso)
      - [Resumen y estadísticas:](#resumen-y-estadísticas)
    - [**Operación:**](#operación)
      - [Sobre las métricas:](#sobre-las-métricas)
      - [Detalle de casos:](#detalle-de-casos)
      - [Ejemplo](#ejemplo)
  - [**Autores (Grupo 14)**](#autores-grupo-14)


## **Lab 1: Datalab (CS:APP)**

Son 26 ejercicios en lenguaje C que consisten en manipular bits para obtener determinado resultado, esto debe lograrse cumpliendo con ciertas restricciones.
Tienen el proposito de familiarizarse con la representacion de numeros enteros y de punto flotante a nivel de bits.

Herramientas:

```bash
# Compilar
$ make

# Pruebas
$ make test

# Calificar implementación
$ make grade
```

## **Lab 2: Recode**

Este lab consiste en la implementación, tanto en C como en Rust, de una utilidad de línea de comando sencilla, _recode57_, capaz de convertir entre distintas codificaciones de texto. En particular, la herramienta será capaz de detectar y convertir entre `UTF-8`, `UTF-16` y `UTF-32`. Toma un único argumento será la codificación deseada para la salida, que podrá ser uno de los siguientes valores:

* `UTF-8`
* `UTF-16BE`
* `UTF-16LE` 
* `UTF-32BE`
* `UTF-32LE`

En cada invocación el programa lee de `stdin`, detecta automáticamente la codificación de entrada y la convierte a la codificación indicada como parámetro, finalmente se imprime el resultado por `stdout`.

Se puede trabajar con archivos en disco mediante redirecciones del intérprete de comandos.

Ejemplo de uso:

```bash
# Detecta el encoding de archivo.txt y lo convierte a UTF-8
$ ./recode57 UTF-8 < archivo.txt > utf8.txt

# Detecta el encoding y lo convierte a UTF-32 (little endian)
$ ./recode57 UTF-32LE < utf8.txt > utf32le.txt
```

## **Lab 3: Lenguaje assembly**

Este lab consiste de dos etapas para trabajar lenguaje assembly:

1. **asmlab:** implementar una cola enlazada en assembly
2. **bomblab:** trabajar con un ejecutable del que no se posee el código fuente, e investigar su estructura y funcionamiento mediante GDB

### **1. asmlab**
---

La primera parte del lab pedía implementar en `assembly x86_64` una cola enlazada de punteros genéricos. Para la misma debimos escribir tests unitarios que verifican su correcto comportamiento.

### **2. bomblab (CS:APP)**
---

La segunda parte del lab buscaba ejercitar el uso de GDB para examinar y determinar la entrada que "desactivara" un binario de código desconocido.
En otras palabras hay un binario estructurado en "fases de ejecución", cada una de las cuales precisa de una contraseña para pasar a la siguiente fase.

El esqueleto de invocación de GDB propuesto en clase es:

```bash
$ gdb -x gdb.txt ./bomb
```

Donde gdb tiene formatos de inicialización del estilo:

```bash
set dissasemble-next-line on
break explode_bomb
break phase_N
run sol.txt
```

La idea es poner breaks para evitar las explosiones y encontrar las contraseñas de cada fase examinando mediante:

```bash
(gdb) disas
```

## **TP 1: Simulador de caché**

Este trabajo práctico implementa un **simulador de caché parametrizado**. Las distintas caracteristicas de la caché simulada son configurables (tamaño, número de sets y asociatividad), y el programa lee y simula una secuencia de operaciones desde un archivo de trazas.
En nuestro caso lo implementamos en C, pero también podía hacerse Rust.

### **Especificaciones:**
---

#### Archivo de traza:
Todas las simulaciones se realizan sobre archivos de trazas de accesos a memoria. Cada uno de esos archivos enumera una serie de operaciones e indica el tipo de operación (lectura o escritura) y la dirección de memoria en donde se realiza.

Un ejemplo:

```
0xb7fc7489: W 0xbff20468 4 0xb7fc748e
0xb7fc748e: R 0xbff20468 4 0xb7fc748e
0xb7fc7495: W 0xbff20478 4 0xbff204b0
0xb7fc749e: R 0xb7fd9ff4 4 0x15f24
```

Los cinco campos de cada línea representan:

1. El primer número en hexadecimal es el _instruction pointer_, esto es, la ubicación en memoria de la instrucción que esta siendo ejecutada.

2. Un caracter ASCII indicando si la operación es de lectura: `R`; o de escritura: `W`.

3. El siguiente valor en hexadecimal es la dirección de memoria a la que se realiza el acceso.

4. Un número entero positivo (por ejemplo, 4 u 8) que indica la cantidad de bytes que la instrucción lee, o escribe.

5. El último valor en hexadecimal corresponde a los datos que se leyeron o escribieron.

El programa simulará estos accesos con una caché del tipo indicado, y reportará las estadísticas correspondientes.

#### Línea de comandos:

La interfaz del programa de la línea de comandos es:

```bash
$ ./cachesim tracefile.xex C E S [ -v n m ]
```

Los cuatro primeros argumentos son:
1. el archivo de traza a simular
2. el tamaño de la caché `C`, en bytes
3. la asociatividad de la caché, `E`
4. el número de sets de la caché, `S`

Ejemplo de invocación:

```bash
$ ./cachesim blowfish.xex 2048 4 8
```

El parámetro `-v` es opcional pero de estar presente, siempre aparece en quinto lugar, seguido de dos números enteros (el rango inclusivo de operaciones de las que queremos información detallada). Su presencia activa un "modo verboso" en que se imprime información detallada para un subconjunto de las operaciones.

Por ejemplo, si se especificase:

```bash
$ ./cachesim blowfish.xex 2048 4 8 -v 0 9
```

Se mostraría información detallada para los primeros diez accesos a memoria.

Además el programa debe imprimir un mensaje de error por stderr, y terminar con estado distinto de cero, en cada uno de los casos siguientes:

* si el número de argumentos no es 4 o 7;
* si el archivo de trazas especificado no existe;
* si alguno de los parámetros C, E o S no son potencia de dos;
* si alguna combinación de parámetros de C, E y S es inválida;
* si los argumentos n y m del modo verboso no son números enteros que cumplan 0 ≤ n ≤ m

Como precondición el programa puede asumir que, si el archivo especificado existe, entonces es un archivo de trazas válido, y todas sus líneas respetan el formato especificado anteriormente.

#### Parámetros fijos:

Hay dos cosas que no se parametrizan en el simulador:
* la política de desalojo, que es siempre _least-recently used_(LRU)
* la _penalty_ por accesos a memoria en el cómputo de tiempos, que es siempre 100 ciclos

#### Modo verboso:

Si se especifica un rango [_n,m_] para el que mostrar información detallada, para cada operación del rango se debe imprimir una línea con la siguiente información:
1. el **índice de la operación**
2. el **identificador de caso**, que será uno de los siguientes valores:
    * '1' para _cache hit_
    * '2a' para _clean cache miss_
    * '2b' para _dirty cache miss_
3. **cache index:** el índice (en hexadecimal) del set correspondiente a la dirección, que será un valor en el rango [0, S)
4. **cache tag:** el valor (en hexadecimal) del correspondiente a la dirección de la operación
5. **cache line:** número de la línea leída o escrita en el set, que será un valor decimal en el rango [0,E)
6. **line tag:** el tag presente anteriormente en la línea (muestra -1 si no había datos válidos)
7. **valid bit:** 1 o 0 según la línea de caché elegida tuviera previamente datos válidos, o
8. **dirty bit:** 0 o 1 según el bloque estuviera previamente sincronizado con memoria principal, o no
9. **last used:** solo para cachés con 
asociatividad _E > 1_, el índice de la operación que usó este bloque por última vez

#### Resumen y estadísticas:

Durante la simulación, se deben recolectar ciertas ,métricas, que serán mostradas al final de la ejecución en el formato exacto que se muestra abajo. Las métricas necesarias son:
* número de lecturas (_loads_)
* número de escrituras (_stores_)
* número total de accesos (_loads + stores_)
* número de misses de lectura (_rmiss_)
* número de misses de escritura (_wmiss_)
* número total de misses (_rmiss + wmiss_)
* número de "dirty read misses" y "dirty write misses"
* cantidad de bytes leídos de memoria (_bytes read_)
* cantidad de bytes escritos en memoria (_bytes written_)
* tiempo de acceso acumulado (en ciclos) para todas las lecturas
* tiempo de acceso acumulado (en ciclos) para todas las escrituras
* miss rate total

Una vez finalizada la simulación, se imprimen en el siguiente formato:

```
2-way, 64 sets, size = 4KB
loads 65672 stores 34328 total 100000
rmiss 679 wmiss 419 total 1098
dirty rmiss 197 dirty wmiss 390
bytes read 17568 bytes written 9392
read time 153272 write time 115228
miss rate 0.010980
```

### **Operación:**
---

En esta sección se explica:

* para cada métrica, la definición exacta de qué debe medir
* para cada acceso, los distintos casos que pueden ocurrir, y la penalización (en tiempo) de cada uno

#### Sobre las métricas:

Las dos primeras métricas, _loads_ y _stores_, corresponden simplemente al número de operaciones de cada tipo (`R` y `W`), y su suma corresponde al total de líneas del archivo de trazas.

Un "miss de lectura" ocurre ante cualquier operación de tipo `R` que resulte en un acceso a memoria. Un "miss de escritura" es el equivalente pero para operaciones de tipo `W`. En ambos casos se incrementan los valores de _bytes read_ y _bytes written_ según el tamaño del bloque.

Las métricas _dirty rmiss_ y _dirty wmiss_ son el subconjunto de misses en que se escribe en memoria, esto es: se remplaza un bloque de la caché, y ese bloque tenía datos que no habían sido enviados aún a memoria principal (En otras palabras, el dirty bit de la línea remplazada estaba a 1).

Finalmente el _miss rate_ total es la división del número total de _misses_ por el total de accesos

#### Detalle de casos:

Cada operación en la caché resultará en uno de estos tres casos:

1. _hit_
2. _miss_, que puede ser:
    <ol type="a">
    <li>clean</li>
    <li>dirty</li>
    </ol>

Sea un acceso a la dirección M, cuyo set (_cache index_) resulta ser _i_; dicho set contiene _E_ líneas, que numeramos de 0 a _E-1_. Entonces:
1. si hay un _hit_, significa que la línea número _k_ con _0 ≤ k < E _, tiene una coincidencia con _M_ en su tag; en ese caso:
    * el tiempo que toma la operación es 1 ciclo
    * el campo _last-used_ de la línea _k_ se actualiza con el índice de la operación actual (para el mecanismo LRU)
    * si el acceso es de escritura, se pone a 1 el _dirty bit_ del bloque
2. si se produce un _miss_, se debe elegir una línea _k_ donde alojar el bloque; ésta siempre será: bien la línea no válida de menor índice, bien la línea con menor valor de _last-used_. Entonces puede suceder:
    <ol type="a">
    <li>la linea a desalojar no tiene datos válidos, o bien los tiene pero el <i>dirty bit</i> es 0: <b>clean cache miss</b>. Se lee el bloque <i>M</i> de memoria y:
        
    <ul>
        <li>el tiempo de acceso en ciclos es 1 + <i>penalty</i></li>
        <li>se actualiza el campo <i>last-used</i></li>
        <li>si el acceso es de escritura, se pone a 1 el <i>dirty bit</i> del bloque</li>
    </ul>

    </li>
    <li>la línea a desalojar tiene su <i>dirty bit</i> a 1: <b>dirty cache miss</b>. Se escribe en memoria el bloque existente y:
    
    <ul>
        <li>se lee el bloque <i>M</i> de memoria</li>
        <li>el tiempo de acceso en ciclos es 1 + 2 x <i>penalty</i></li>
        <li>se actualiza el campo <i>last-used</i></li>
        <li>si el acceso es de escritura, se pone a 1 el <i>dirty bit</i> del bloque</li>
    </ul>

    </li>
    </ol>

#### Ejemplo

Dado el archivo de traza [adpcm.xex](https://orgacomp.github.io/static/cachesim/trazas/adpcm.xex), se ofrecen muestras de la salida en las siguientes configuraciones:

* 2KiB, 2-way, 64 sets:

```bash
$ ./cachesim adpcm.xex 2048 2 64 -v 0 15000
```

Salida esperada: [adpcm_2048-2-64.txt.](https://orgacomp.github.io/static/cachesim/output/adpcm_2048-2-64.txt)

* 4KiB, direct-mapped, 256 sets:

```bash
$ ./cachesim adpcm.xex 4096 1 256 -v 0 10000
```

Salida esperada: [adpcm_2048-2-64.txt.](https://orgacomp.github.io/static/cachesim/output/adpcm_4096-1-256.txt)

## **Autores (Grupo 14)**
- Tomás Del Pup
- Santiago Marczewski












