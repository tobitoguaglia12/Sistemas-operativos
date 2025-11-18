# Manual de Uso - Simulador de Gestión de Memoria y Planificación de Procesos

## Índice
1. [Introducción](#introducción)
2. [Requisitos del Sistema](#requisitos-del-sistema)
3. [Compilación del Programa](#compilación-del-programa)
4. [Preparación de Archivos de Entrada](#preparación-de-archivos-de-entrada)
5. [Ejecución del Simulador](#ejecución-del-simulador)
6. [Controles de Navegación](#controles-de-navegación)
7. [Interpretación de la Interfaz](#interpretación-de-la-interfaz)
8. [Estadísticas Finales](#estadísticas-finales)
9. [Ejemplos de Uso](#ejemplos-de-uso)
10. [Solución de Problemas](#solución-de-problemas)

---

## Introducción

Este simulador implementa un sistema de gestión de memoria con **particiones fijas** y un algoritmo de planificación de CPU **SRTF (Shortest Remaining Time First)**. 

### Características principales:
- **Algoritmo de asignación de memoria**: Best-Fit
- **Algoritmo de planificación**: SRTF (con desalojo)
- **Grado de multiprogramación**: Máximo 5 procesos simultáneos en memoria
- **Navegación interactiva**: Avance paso a paso con historial completo (avanzar y retroceder)
- **Estadísticas completas**: TR, TE, promedios y throughput del sistema

---

## Requisitos del Sistema

### Software necesario:
- **Sistema Operativo**: Windows (7, 8, 10, 11)
- **Compilador**: GCC (MinGW o MinGW-w64)
- **Librerías**: 
  - `windows.h` (incluida en MinGW)
  - `commdlg.h` (incluida en MinGW)
  - `conio.h` (incluida en MinGW)

### Instalación de GCC en Windows:
1. Descargar MinGW-w64 desde [https://www.mingw-w64.org/](https://www.mingw-w64.org/)
2. Instalar y agregar la carpeta `bin` al PATH del sistema
3. Verificar la instalación abriendo PowerShell y ejecutando: `gcc --version`

---

## Compilación del Programa

### Paso 1: Abrir la terminal
Navega a la carpeta donde se encuentra `main.c`:
```powershell
cd c:\Users\Tobias\Desktop\SO\Sistemas-operativos
```

### Paso 2: Compilar el código
Ejecuta el siguiente comando:
```powershell
gcc main.c -o simulador.exe -lcomdlg32 -lgdi32
```

**Explicación de los parámetros**:
- `main.c`: Archivo fuente
- `-o simulador.exe`: Nombre del ejecutable generado
- `-lcomdlg32`: Librería para el diálogo de selección de archivos
- `-lgdi32`: Librería para funciones gráficas de Windows

### Verificación:
Si la compilación es exitosa, se generará el archivo `simulador.exe` en la misma carpeta.

---

## Preparación de Archivos de Entrada

### Formato del archivo CSV
El simulador lee archivos CSV con el siguiente formato:

```
ID,tamaño,tiempoArribo,tiempoInterrupcion
P1,80,0,5
P2,120,1,4
P3,200,2,6
```

### Especificación de columnas:

| Columna | Descripción | Unidad | Ejemplo |
|---------|-------------|--------|---------|
| **ID** | Identificador único del proceso | Texto | P1, P2, P3 |
| **tamaño** | Tamaño del proceso en memoria | KB | 80, 120, 200 |
| **tiempoArribo** | Momento en que llega el proceso | Unidades de tiempo | 0, 1, 2 |
| **tiempoInterrupcion** | Tiempo total de CPU requerido | Unidades de tiempo | 5, 4, 6 |

### Ejemplo de archivo completo (`5 procesos.csv`):
```csv
ID,tamaño,tiempoArribo,tiempoInterrupcion
P1,80,0,5
P2,30,1,3
P3,120,1,6
P4,200,2,4
P5,40,3,2
```

### Ubicación recomendada:
Guarda tus archivos CSV en la carpeta `archivos/` del proyecto.

---

## Ejecución del Simulador

### Método 1: Doble clic
1. Localiza el archivo `simulador.exe`
2. Haz doble clic sobre él

### Método 2: Desde la terminal
```powershell
.\simulador.exe
```

### Selección del archivo de entrada:
1. Al ejecutar el programa, se abrirá automáticamente un **diálogo de Windows**
2. Navega hasta la carpeta `archivos/`
3. Selecciona el archivo CSV que deseas simular (ejemplo: `5 procesos.csv`)
4. Haz clic en **Abrir**

**Nota**: Si cancelas la selección, el programa se cerrará.

---

## Controles de Navegación

El simulador funciona en modo **paso a paso interactivo**. Después de cada unidad de tiempo, puedes controlar la ejecución:

| Tecla | Acción | Cuándo usar |
|-------|--------|-------------|
| **ENTER** | Avanzar al siguiente estado | Para continuar la simulación hacia adelante |
| **B** | Retroceder al estado anterior | Para revisar estados anteriores (historial) |
| **ESC** | Salir del programa | Solo disponible al finalizar la simulación |

### Flujo de uso típico:
1. **ENTER** → Avanza tiempo 0 → **ENTER** → Avanza tiempo 1 → **B** → Regresa a tiempo 0
2. Puedes navegar libremente hacia adelante y atrás en cualquier momento
3. Al finalizar todos los procesos, presiona **ESC** para ver las estadísticas finales

---

## Interpretación de la Interfaz

### Estructura de la pantalla

La interfaz se divide en **cuatro tablas** organizadas en dos columnas:

```
┌────────────────────────────┬────────────────────────────┐
│   PARTICIONES DE MEMORIA   │  LISTOS (en memoria)       │
├────────────────────────────┼────────────────────────────┤
│   PROCESOS NUEVOS          │  FINALIZADOS               │
└────────────────────────────┴────────────────────────────┘
```

### 1. PARTICIONES DE MEMORIA

```
+----------------------------------------------------------+
| Partición           | Tamaño | Proceso | Estado          |
+----------------------------------------------------------+
| Sistema Operativo   | 100    | SO      | Ocupada         |
| Grande              | 250    | P3      | Ocupada         |
| Mediana             | 150    | P2      | Ocupada         |
| Pequeña             | 50     | P1      | Ocupada         |
+----------------------------------------------------------+
```

**Interpretación**:
- Muestra las **4 particiones fijas** del sistema
- **Tamaño**: Capacidad en KB de cada partición
- **Proceso**: Qué proceso ocupa la partición (o "Libre" si está disponible)
- **Estado**: "Ocupada" o "Libre"

**Particiones del sistema**:
- **Sistema Operativo**: 100 KB (siempre ocupada por el SO)
- **Grande**: 250 KB
- **Mediana**: 150 KB
- **Pequeña**: 50 KB

### 2. PROCESOS NUEVOS (cola de nuevos)

```
+----------------------------------------------------------+
| ID  | Tamaño | T.Arribo | T.Interrupción | T.Restante   |
+----------------------------------------------------------+
| P5  | 40     | 3        | 2              | 2            |
| P6  | 60     | 4        | 3              | 3            |
+----------------------------------------------------------+
```

**Interpretación**:
- Procesos que **aún no han llegado** o están **esperando para entrar en memoria**
- Máximo de multiprogramación: **5 procesos en memoria simultáneamente**
- Si hay 5 procesos en "LISTOS", los nuevos esperan aquí

### 3. LISTOS (en memoria)

```
+----------------------------------------------------------+
| ID  | Tamaño | T.Arribo | T.Interrupción | T.Restante   |
+----------------------------------------------------------+
| P1* | 80     | 0        | 5              | 3            |
| P2  | 120    | 1        | 4              | 4            |
| P3  | 200    | 2        | 6              | 6            |
+----------------------------------------------------------+
```

**Interpretación**:
- Procesos que **están en memoria** y listos para ejecutar
- El proceso con **asterisco (*)** es el que está **ejecutando actualmente**
- Se selecciona según **SRTF**: el de menor tiempo restante
- **T.Restante** disminuye a medida que el proceso ejecuta

### 4. FINALIZADOS

```
+----------------------------------------------------------+
| ID  | Tamaño | T.Arribo | T.Inicio | T.Final | TR | TE  |
+----------------------------------------------------------+
| P2  | 120    | 1        | 1        | 5       | 4  | 0   |
| P1  | 80     | 0        | 0        | 6       | 6  | 1   |
+----------------------------------------------------------+
```

**Interpretación**:
- Procesos que completaron su ejecución
- **T.Inicio**: Momento en que comenzó a ejecutar por primera vez
- **T.Final**: Momento en que terminó completamente
- **TR (Tiempo de Retorno)**: T.Final - T.Arribo
- **TE (Tiempo de Espera)**: TR - T.Interrupción

### Información del encabezado

```
========================================
SIMULADOR DE GESTIÓN DE MEMORIA Y PLANIFICACIÓN DE PROCESOS
Tiempo actual: 3
Proceso en ejecución: P1
========================================
```

- **Tiempo actual**: Unidad de tiempo de la simulación
- **Proceso en ejecución**: ID del proceso que está usando la CPU (o "Ninguno")

---

## Estadísticas Finales

Al presionar **ESC** después de que todos los procesos finalizan, verás:

```
========================================
         ESTADÍSTICAS FINALES
========================================

TIEMPO TOTAL DE SIMULACIÓN: 15 unidades

PROCESOS FINALIZADOS:
  P1: TR=6, TE=1
  P2: TR=4, TE=0
  P3: TR=10, TE=4
  P4: TR=8, TE=4
  P5: TR=11, TE=9

PROMEDIOS:
  Tiempo de Retorno Promedio: 7.80
  Tiempo de Espera Promedio: 3.60

THROUGHPUT: 0.33 procesos/unidad de tiempo

========================================
Presiona ESC para salir...
```

### Explicación de las métricas:

| Métrica | Fórmula | Significado |
|---------|---------|-------------|
| **TR (Tiempo de Retorno)** | T.Final - T.Arribo | Tiempo total desde que llegó hasta que finalizó |
| **TE (Tiempo de Espera)** | TR - T.Interrupción | Tiempo que pasó esperando (no ejecutando) |
| **TR Promedio** | Suma(TR) / N | Rendimiento general del sistema |
| **TE Promedio** | Suma(TE) / N | Eficiencia del planificador |
| **Throughput** | N / Tiempo Total | Procesos completados por unidad de tiempo |

---

## Ejemplos de Uso

### Ejemplo 1: Simulación básica con 5 procesos

**Archivo**: `archivos/5 procesos.csv`

1. Ejecuta `simulador.exe`
2. Selecciona `5 procesos.csv`
3. Presiona **ENTER** repetidamente para avanzar
4. Observa cómo:
   - Los procesos entran en memoria según Best-Fit
   - El proceso con menor tiempo restante ejecuta (SRTF)
   - Los procesos terminan y liberan particiones
5. Al finalizar, presiona **ESC** para ver estadísticas

### Ejemplo 2: Revisión de un momento específico

1. Avanza hasta el tiempo 5 (presionando **ENTER** 5 veces)
2. Presiona **B** tres veces para retroceder al tiempo 2
3. Observa el estado del sistema en ese momento
4. Presiona **ENTER** para avanzar nuevamente

### Ejemplo 3: Verificación de multiprogramación

**Archivo**: `archivos/10 procesos.csv`

1. Ejecuta el simulador con este archivo
2. Observa que en **LISTOS (en memoria)** nunca hay más de **5 procesos**
3. Los procesos P6, P7, etc. permanecen en **PROCESOS NUEVOS** hasta que haya espacio

---

## Solución de Problemas

### Problema 1: "No se pudo compilar el programa"

**Síntomas**: Error al ejecutar `gcc main.c ...`

**Soluciones**:
1. Verifica que GCC esté instalado: `gcc --version`
2. Asegúrate de estar en la carpeta correcta: `cd c:\Users\Tobias\Desktop\SO\Sistemas-operativos`
3. Verifica que `main.c` exista en la carpeta: `ls main.c`

### Problema 2: "El diálogo de archivos no se abre"

**Síntomas**: El programa se cierra inmediatamente

**Soluciones**:
1. Recompila con las librerías correctas: `-lcomdlg32 -lgdi32`
2. Verifica que el archivo `simulador.exe` sea reciente

### Problema 3: "Error al leer el archivo CSV"

**Síntomas**: Mensaje de error al seleccionar el archivo

**Soluciones**:
1. Verifica que el archivo tenga el formato correcto (4 columnas)
2. Asegúrate de que la primera línea sea: `ID,tamaño,tiempoArribo,tiempoInterrupcion`
3. No debe haber líneas vacías al final del archivo
4. Los números deben ser enteros positivos

### Problema 4: "El proceso no entra en memoria"

**Síntomas**: Un proceso permanece en "PROCESOS NUEVOS" indefinidamente

**Causas posibles**:
1. **Multiprogramación**: Ya hay 5 procesos en memoria (límite del sistema)
   - Solución: Espera a que algún proceso termine
2. **Tamaño excesivo**: El proceso es más grande que cualquier partición disponible
   - Ejemplo: Un proceso de 300 KB no cabe en ninguna partición (máximo 250 KB)
   - Solución: Reduce el tamaño del proceso o espera a que se libere la partición Grande

### Problema 5: "Las teclas no responden"

**Síntomas**: ENTER o B no funcionan

**Soluciones**:
1. Asegúrate de que la ventana de la consola esté enfocada (haz clic en ella)
2. Presiona la tecla una sola vez y espera (no mantengas presionada)
3. ESC solo funciona al final de la simulación, no durante

---

## Notas Técnicas Adicionales

### Algoritmo Best-Fit
- Busca la **partición más pequeña** que pueda contener el proceso
- Si hay varias particiones del mismo tamaño, elige la primera disponible
- Reduce la fragmentación interna

### Algoritmo SRTF
- Selecciona el proceso con **menor tiempo restante** de CPU
- Es un algoritmo **con desalojo** (preemptive)
- Si llega un proceso nuevo con menos tiempo restante, se produce un cambio de contexto
- Minimiza el tiempo promedio de espera

### Grado de Multiprogramación
- **Máximo**: 5 procesos simultáneos en memoria (sin contar el SO)
- Si hay 5 procesos en "LISTOS", los nuevos esperan en la cola
- Cuando un proceso termina, se admite el siguiente de la cola de nuevos

---

## Contacto y Soporte

Para reportar problemas o sugerencias sobre este simulador:
- **Autor**: Tobias Guaglia
- **Proyecto**: Simulador de SO - Universidad
- **Fecha**: Noviembre 2025

---

## Licencia

Este software es de uso educativo para el curso de Sistemas Operativos.

---

**¡Gracias por usar el Simulador de Gestión de Memoria y Planificación de Procesos!**
