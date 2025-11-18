// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <commdlg.h>
#include <conio.h> // para _getch()

// -------------------- ESTRUCTURAS --------------------
struct Proceso {
    char id[50];
    int tamaño;
    int tiempoArribo;
    int tiempoInterrupcion;
    int tiempoRestante;
    int tiempoFinalizacion;
    struct Proceso* prox;
};

struct Particion {
    char idParticion[50];
    int tamaño;
    char idProceso[50];
    int memoriaUsada;
    int fragmentacionInterna;
    struct Particion* prox;
};

// -------------------- ESTRUCTURA DE HISTORIAL --------------------
struct Estado {
    int tiempo;
    struct Particion* particiones;
    struct Proceso* nuevos;
    struct Proceso* listos_suspendidos;
    struct Proceso* listos;
    struct Proceso* finalizados;
    struct Estado* anterior;
    struct Estado* siguiente;
};

// -------------------- PROTOTIPOS --------------------
int seleccionarArchivo(wchar_t *ruta, size_t tamañoRuta);
struct Particion* inicializarMemoria();
struct Proceso* encontrarSRT(struct Proceso* cabeza);
void moverProceso(struct Proceso** origen, struct Proceso** destino, const char* idProceso);
void liberarListaProcesos(struct Proceso* cabeza);
void liberarListaParticiones(struct Particion* cabeza);
void imprimirEstado(int tiempo, struct Particion* p, struct Proceso* n, struct Proceso* ls, struct Proceso* l, struct Proceso* f, const char* evento);
void imprimirTabla(const char* titulo, struct Proceso* lista);
void imprimirTablasDobles(const char* titulo1, struct Proceso* lista1, const char* titulo2, struct Proceso* lista2);
void imprimirParticiones(struct Particion* p);
void mostrarBienvenida();
void imprimirEstadisticas(struct Proceso* finalizados, int tiempoTotal);
char esperarAccion();
int contarProcesosEnMemoria(struct Particion* particiones);
struct Proceso* copiarListaProcesos(struct Proceso* original);
struct Particion* copiarListaParticiones(struct Particion* original);
struct Estado* guardarEstado(int tiempo, struct Particion* p, struct Proceso* n,
                             struct Proceso* ls, struct Proceso* l, struct Proceso* f,
                             struct Estado* actual);
void liberarEstado(struct Estado* e);

// -------------------- IMPLEMENTACIÓN --------------------
int seleccionarArchivo(wchar_t *ruta, size_t tamañoRuta) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Archivos CSV\0*.csv\0Todos los archivos\0*.*\0";
    ofn.lpstrFile = ruta;
    ofn.nMaxFile = tamañoRuta;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"csv";
    ofn.lpstrTitle = L"Seleccionar archivo de procesos";
    return GetOpenFileNameW(&ofn);
}

struct Particion* inicializarMemoria() {
    struct Particion* pSO = malloc(sizeof(struct Particion));
    struct Particion* pGrande = malloc(sizeof(struct Particion));
    struct Particion* pMediana = malloc(sizeof(struct Particion));
    struct Particion* pPeque = malloc(sizeof(struct Particion));
    if (!pSO || !pGrande || !pMediana || !pPeque) return NULL;

    strcpy(pSO->idParticion, "SO"); strcpy(pSO->idProceso, "SO");
    pSO->tamaño = 100; pSO->memoriaUsada = 100; pSO->fragmentacionInterna = 0;

    strcpy(pGrande->idParticion, "Grande"); strcpy(pGrande->idProceso, "");
    pGrande->tamaño = 250; pGrande->memoriaUsada = 0; pGrande->fragmentacionInterna = 0;

    strcpy(pMediana->idParticion, "Mediana"); strcpy(pMediana->idProceso, "");
    pMediana->tamaño = 150; pMediana->memoriaUsada = 0; pMediana->fragmentacionInterna = 0;

    strcpy(pPeque->idParticion, "Pequeña "); strcpy(pPeque->idProceso, "");
    pPeque->tamaño = 50; pPeque->memoriaUsada = 0; pPeque->fragmentacionInterna = 0;

    pSO->prox = pGrande; pGrande->prox = pMediana; pMediana->prox = pPeque; pPeque->prox = NULL;
    return pSO;
}

struct Proceso* encontrarSRT(struct Proceso* cabeza) {
    if (!cabeza) return NULL;
    struct Proceso* min = cabeza;
    for (struct Proceso* actual = cabeza->prox; actual; actual = actual->prox)
        if (actual->tiempoRestante < min->tiempoRestante)
            min = actual;
    return min;
}

void moverProceso(struct Proceso** origen, struct Proceso** destino, const char* idProceso) {
    struct Proceso* actual = *origen;
    struct Proceso* anterior = NULL;
    while (actual && strcmp(actual->id, idProceso) != 0) {
        anterior = actual;
        actual = actual->prox;
    }
    if (!actual) return;
    if (!anterior) *origen = actual->prox;
    else anterior->prox = actual->prox;
    actual->prox = *destino;
    *destino = actual;
}

void liberarListaProcesos(struct Proceso* cabeza) {
    while (cabeza) {
        struct Proceso* sig = cabeza->prox;
        free(cabeza);
        cabeza = sig;
    }
}

void liberarListaParticiones(struct Particion* cabeza) {
    while (cabeza) {
        struct Particion* sig = cabeza->prox;
        free(cabeza);
        cabeza = sig;
    }
}

void imprimirParticiones(struct Particion* p) {
    printf("\n--- Particiones de Memoria ---\n");
    while(p) {
        if (strcmp(p->idProceso, "") != 0)
            printf("  > %-8s: Ocupada por [%s] | Tam: %3dK, Usado: %3dK, Frag: %3dK\n",
                   p->idParticion, p->idProceso, p->tamaño, p->memoriaUsada, p->fragmentacionInterna);
        else
            printf("  > %-8s: Libre            | Tam: %3dK\n", p->idParticion, p->tamaño);
        p = p->prox;
    }
}

void imprimirTabla(const char* titulo, struct Proceso* lista) {
    printf("\n%s\n", titulo);
    printf("+---------+--------+------------+----------------+----------------+\n");
    printf("|  ID     | Tamaño | Arribo     | Interrupción   | Restante       |\n");
    printf("+---------+--------+------------+----------------+----------------+\n");

    if (!lista)
        printf("| (vacío)                                                         |\n");
    else
        for (; lista; lista = lista->prox)
            printf("| %-7s | %-6d | %-10d | %-14d | %-14d |\n",
                   lista->id, lista->tamaño, lista->tiempoArribo,
                   lista->tiempoInterrupcion, lista->tiempoRestante);
    printf("+---------+--------+------------+----------------+----------------+\n");
}

// Función para imprimir dos tablas lado a lado
// Función para imprimir dos tablas lado a lado (fix de padding y formato)
// Reemplazar la función existente por esta versión
// Función corregida para imprimir dos tablas lado a lado
void pad_right(const char* temp, char* dest, int ancho) {
    int len = (int)strlen(temp);
    int copy = (len > ancho) ? ancho : len;
    memcpy(dest, temp, copy);
    for (int i = copy; i < ancho; ++i) dest[i] = ' ';
    dest[ancho] = '\0';
}

void imprimirTablasDobles(const char* titulo1, struct Proceso* lista1,
                          const char* titulo2, struct Proceso* lista2) {
    // buffers para líneas (suficientemente grandes)
    char lineas1[128][128];
    char lineas2[128][128];
    int filas1 = 0, filas2 = 0;

    // definimos separador/header y calculamos ancho desde el separador
    const char* SEPARADOR = "+---------+--------+----------+--------------+--------------+";
    const char* HEADER    = "|  ID     | Tamano | Arribo   | Interrup.    | Restante     |";

    int ANCHO = (int)strlen(SEPARADOR); // <-- importante: calculado dinámicamente

    // Tabla 1 header
    pad_right(titulo1, lineas1[filas1++], ANCHO);
    pad_right(SEPARADOR, lineas1[filas1++], ANCHO);
    pad_right(HEADER, lineas1[filas1++], ANCHO);
    pad_right(SEPARADOR, lineas1[filas1++], ANCHO);

    if (!lista1) {
        // Construimos la línea vacía usando el mismo formato que las filas
        char tmp[128];
        snprintf(tmp, sizeof(tmp), "| %-7s | %-6s | %-8s | %-12s | %-12s |",
                 "(vacio)", "", "", "", "");
        pad_right(tmp, lineas1[filas1++], ANCHO);
    } else {
        for (; lista1; lista1 = lista1->prox) {
            char temp[128];
            snprintf(temp, sizeof(temp),
                     "| %-7s | %-6d | %-8d | %-12d | %-12d |",
                     lista1->id, lista1->tamaño, lista1->tiempoArribo,
                     lista1->tiempoInterrupcion, lista1->tiempoRestante);
            pad_right(temp, lineas1[filas1++], ANCHO);
        }
    }
    pad_right(SEPARADOR, lineas1[filas1++], ANCHO);

    // Tabla 2 header
    pad_right(titulo2, lineas2[filas2++], ANCHO);
    pad_right(SEPARADOR, lineas2[filas2++], ANCHO);
    pad_right(HEADER, lineas2[filas2++], ANCHO);
    pad_right(SEPARADOR, lineas2[filas2++], ANCHO);

    if (!lista2) {
        char tmp[128];
        snprintf(tmp, sizeof(tmp), "| %-7s | %-6s | %-8s | %-12s | %-12s |",
                 "(vacio)", "", "", "", "");
        pad_right(tmp, lineas2[filas2++], ANCHO);
    } else {
        for (; lista2; lista2 = lista2->prox) {
            char temp[128];
            snprintf(temp, sizeof(temp),
                     "| %-7s | %-6d | %-8d | %-12d | %-12d |",
                     lista2->id, lista2->tamaño, lista2->tiempoArribo,
                     lista2->tiempoInterrupcion, lista2->tiempoRestante);
            pad_right(temp, lineas2[filas2++], ANCHO);
        }
    }
    pad_right(SEPARADOR, lineas2[filas2++], ANCHO);

    // Imprimir lado a lado
    int maxFilas = (filas1 > filas2) ? filas1 : filas2;
    for (int i = 0; i < maxFilas; ++i) {
        if (i < filas1) printf("%s", lineas1[i]);
        else { for (int k = 0; k < ANCHO; ++k) putchar(' '); }

        printf("    "); // separador entre tablas

        if (i < filas2) printf("%s", lineas2[i]);
        else { for (int k = 0; k < ANCHO; ++k) putchar(' '); }

        printf("\n");
    }
}


void imprimirEstado(int tiempo, struct Particion* particiones, struct Proceso* nuevos,
                    struct Proceso* listos_suspendidos, struct Proceso* listos,
                    struct Proceso* finalizados, const char* evento) {
    printf("==================================================================================================\n");
    printf("TIEMPO: %d\n", tiempo);
    printf("EVENTO: %s\n", evento[0] ? evento : " - ");
    imprimirParticiones(particiones);
    
    // Imprimir tablas en dos columnas
    imprimirTablasDobles("NUEVOS", nuevos, "LISTOS-SUSPENDIDOS", listos_suspendidos);
    imprimirTablasDobles("LISTOS (en memoria)", listos, "FINALIZADOS", finalizados);
    
    printf("==================================================================================================\n");
}

void mostrarBienvenida() {
    system("cls");
    printf("============================================\n");
    printf("      SIMULADOR DE SISTEMA OPERATIVO\n");
    printf("               GRUPO ERROR404\n");
    printf("============================================\n\n");
    printf("Participantes:\n");
    printf(" - Bentolila Elias Ezequiel\n");
    printf(" - Guaglianone Tobias\n");
    printf(" - Ibarra Santiago\n");
    printf(" - Mac Lachlan Guadalupe Denise\n");
    printf(" - Pabon Juan Ignacio\n\n");
    printf("Presione ENTER para continuar...");
    system("pause >nul");
    system("cls");
}

void imprimirEstadisticas(struct Proceso* finalizados, int tiempoTotal) {
    printf("\n=================== INFORME ESTADÍSTICO ===================\n");
    printf("+---------+---------------+--------------+---------------+\n");
    printf("|  ID     | Retorno (TR)  | Espera (TE)  | Finalización  |\n");
    printf("+---------+---------------+--------------+---------------+\n");

    struct Proceso* temp = finalizados;
    int cantidad = 0;
    double sumaR = 0, sumaE = 0;

    while (temp) {
        int TR = temp->tiempoFinalizacion - temp->tiempoArribo;
        int TE = TR - temp->tiempoInterrupcion;
        printf("| %-7s | %-13d | %-12d | %-13d |\n", temp->id, TR, TE, temp->tiempoFinalizacion);
        sumaR += TR;
        sumaE += TE;
        cantidad++;
        temp = temp->prox;
    }

    printf("+---------+---------------+--------------+---------------+\n");
    if (cantidad > 0) {
        printf("\nPromedio Retorno: %.2f", sumaR / cantidad);
        printf("\nPromedio Espera : %.2f", sumaE / cantidad);
        printf("\nRendimiento     : %.3f trabajos/unidad de tiempo\n", (double)cantidad / tiempoTotal);
    }
    printf("===========================================================\n");
}

char esperarAccion() {
    printf("\n[ENTER] Avanzar  |  [B] Retroceder  |  [ESC] Salir\n");
    printf("Acción: ");
    while (1) {
        char c = _getch();
        if (c == '\r') {
            printf("Avanzar\n");
            return 'E';  // ENTER
        }
        if (c == 'b' || c == 'B') {
            printf("Retroceder\n");
            return 'B';
        }
        if (c == 27) {  // ESC
            printf("Salir\n");
            return 'S';  // Salir
        }
    }
}

// Función para contar procesos en memoria (excluyendo el SO)
int contarProcesosEnMemoria(struct Particion* particiones) {
    int contador = 0;
    struct Particion* temp = particiones;
    while (temp != NULL) {
        // Contar solo si hay un proceso asignado Y no es el SO
        if (strcmp(temp->idProceso, "") != 0 && strcmp(temp->idProceso, "SO") != 0) {
            contador++;
        }
        temp = temp->prox;
    }
    return contador;
}

// -------------------- FUNCIONES DE COPIA --------------------
struct Proceso* copiarListaProcesos(struct Proceso* original) {
    if (!original) return NULL;
    struct Proceso* copia = NULL, *ultimo = NULL;
    while (original) {
        struct Proceso* nuevo = malloc(sizeof(struct Proceso));
        memcpy(nuevo, original, sizeof(struct Proceso));
        nuevo->prox = NULL;
        if (!copia) copia = nuevo;
        else ultimo->prox = nuevo;
        ultimo = nuevo;
        original = original->prox;
    }
    return copia;
}

struct Particion* copiarListaParticiones(struct Particion* original) {
    if (!original) return NULL;
    struct Particion* copia = NULL, *ultimo = NULL;
    while (original) {
        struct Particion* nuevo = malloc(sizeof(struct Particion));
        memcpy(nuevo, original, sizeof(struct Particion));
        nuevo->prox = NULL;
        if (!copia) copia = nuevo;
        else ultimo->prox = nuevo;
        ultimo = nuevo;
        original = original->prox;
    }
    return copia;
}

struct Estado* guardarEstado(int tiempo, struct Particion* p, struct Proceso* n,
                             struct Proceso* ls, struct Proceso* l, struct Proceso* f,
                             struct Estado* actual) {
    struct Estado* nuevo = malloc(sizeof(struct Estado));
    nuevo->tiempo = tiempo;
    nuevo->particiones = copiarListaParticiones(p);
    nuevo->nuevos = copiarListaProcesos(n);
    nuevo->listos_suspendidos = copiarListaProcesos(ls);
    nuevo->listos = copiarListaProcesos(l);
    nuevo->finalizados = copiarListaProcesos(f);
    nuevo->anterior = actual;
    nuevo->siguiente = NULL;
    if (actual) actual->siguiente = nuevo;
    return nuevo;
}

void liberarEstado(struct Estado* e) {
    liberarListaProcesos(e->nuevos);
    liberarListaProcesos(e->listos_suspendidos);
    liberarListaProcesos(e->listos);
    liberarListaProcesos(e->finalizados);
    liberarListaParticiones(e->particiones);
    free(e);
}

// -------------------- MAIN --------------------
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    mostrarBienvenida();

    wchar_t rutaArchivo[MAX_PATH] = {0};
    if (!seleccionarArchivo(rutaArchivo, MAX_PATH)) {
        MessageBoxW(NULL, L"No se seleccionó ningún archivo.", L"Error", MB_OK | MB_ICONWARNING);
        return 1;
    }

    FILE *f = _wfopen(rutaArchivo, L"r");
    if (!f) {
        printf("Error al abrir el archivo.\n");
        return 1;
    }

    struct Proceso* nuevos = NULL;
    char linea[256];
    int contador = 0;

    while (fgets(linea, sizeof(linea), f) && contador < 10) {
        if (linea[0] == '\n' || linea[0] == '\r' || linea[0] == '#') continue;
        struct Proceso* p = malloc(sizeof(struct Proceso));
        if (!p) continue;
        if (sscanf(linea, "%49[^,],%d,%d,%d", p->id, &p->tamaño, &p->tiempoArribo, &p->tiempoInterrupcion) == 4) {
            p->tiempoRestante = p->tiempoInterrupcion;
            p->tiempoFinalizacion = -1;
            p->prox = nuevos;
            nuevos = p;
            contador++;
        } else free(p);
    }
    fclose(f);

    struct Particion* particiones = inicializarMemoria();
    struct Proceso* listos_suspendidos = NULL;
    struct Proceso* listos = NULL;
    struct Proceso* finalizados = NULL;

    // -------------------- BUCLE PRINCIPAL CORREGIDO --------------------
    struct Estado* estadoActual = guardarEstado(0, particiones, nuevos, listos_suspendidos, listos, finalizados, NULL);
    struct Estado* historial = estadoActual;
    int tiempo = 0;
    int simulacionTerminada = 0;

    while (!simulacionTerminada) {
        // Mostrar estado actual
        system("cls");
        
        // Calcular evento para mostrar
        char evento[512] = "";
        struct Proceso* enEjecucion = encontrarSRT(listos);
        
        // Mostrar información de llegadas en este tiempo
        for (struct Proceso* p = nuevos; p; p = p->prox) {
            if (p->tiempoArribo == tiempo) {
                sprintf(evento + strlen(evento), "Llega %s. ", p->id);
            }
        }
        
        // Mostrar información de procesos en memoria
        for (struct Proceso* p = listos; p; p = p->prox) {
            int encontrado = 0;
            for (struct Particion* mem = particiones; mem; mem = mem->prox) {
                if (strcmp(mem->idProceso, p->id) == 0) {
                    encontrado = 1;
                    break;
                }
            }
        }
        
        if (enEjecucion)
            sprintf(evento + strlen(evento), "CPU ejecuta %s (restante %d). ", enEjecucion->id, enEjecucion->tiempoRestante);
        else
            strcat(evento, "CPU Ocioso.");
        
        imprimirEstado(tiempo, particiones, nuevos, listos_suspendidos, listos, finalizados, evento);
        
        // Esperar acción del usuario
        char accion = esperarAccion();

        // ==================== SALIR ====================
        if (accion == 'S') {
            printf("\n¿Está seguro que desea salir? (S/N): ");
            char confirmar = _getch();
            if (confirmar == 's' || confirmar == 'S') {
                simulacionTerminada = 1;
                continue;
            } else {
                continue; // Volver a mostrar el mismo estado
            }
        }

        // ==================== RETROCEDER ====================
        if (accion == 'B') {
            if (estadoActual->anterior) {
                // Liberar estado actual
                liberarListaProcesos(nuevos);
                liberarListaProcesos(listos_suspendidos);
                liberarListaProcesos(listos);
                liberarListaProcesos(finalizados);
                liberarListaParticiones(particiones);

                // Ir al estado anterior
                estadoActual = estadoActual->anterior;
                tiempo = estadoActual->tiempo;
                
                // Restaurar listas desde el estado anterior
                nuevos = copiarListaProcesos(estadoActual->nuevos);
                listos_suspendidos = copiarListaProcesos(estadoActual->listos_suspendidos);
                listos = copiarListaProcesos(estadoActual->listos);
                finalizados = copiarListaProcesos(estadoActual->finalizados);
                particiones = copiarListaParticiones(estadoActual->particiones);
            }
            continue; // Volver a mostrar sin avanzar
        }

        // ==================== AVANZAR ====================
        // Si ya no hay procesos, terminar
        if (!nuevos && !listos_suspendidos && !listos) {
            simulacionTerminada = 1;
            continue;
        }
        
        // Si estamos en un estado que ya fue simulado (tiene siguiente), ir al siguiente
        if (estadoActual->siguiente) {
            // Liberar estado actual
            liberarListaProcesos(nuevos);
            liberarListaProcesos(listos_suspendidos);
            liberarListaProcesos(listos);
            liberarListaProcesos(finalizados);
            liberarListaParticiones(particiones);
            
            // Avanzar al siguiente estado guardado
            estadoActual = estadoActual->siguiente;
            tiempo = estadoActual->tiempo;
            
            // Restaurar listas
            nuevos = copiarListaProcesos(estadoActual->nuevos);
            listos_suspendidos = copiarListaProcesos(estadoActual->listos_suspendidos);
            listos = copiarListaProcesos(estadoActual->listos);
            finalizados = copiarListaProcesos(estadoActual->finalizados);
            particiones = copiarListaParticiones(estadoActual->particiones);
            
            continue;
        }
        
        // Si no hay siguiente, simular el próximo paso
        
        // A. LLEGADA DE PROCESOS
        struct Proceso* temp = nuevos;
        while (temp) {
            struct Proceso* sig = temp->prox;
            if (temp->tiempoArribo <= tiempo) {
                moverProceso(&nuevos, &listos_suspendidos, temp->id);
            }
            temp = sig;
        }

        // B. ADMISIÓN EN MEMORIA (Best-Fit) - CON CONTROL DE MULTIPROGRAMACIÓN
        temp = listos_suspendidos;
        while (temp) {
            struct Proceso* sig = temp->prox;
            
            // CONTROL DE MULTIPROGRAMACIÓN: Máximo 5 procesos en memoria
            int procesosEnMemoria = contarProcesosEnMemoria(particiones);
            if (procesosEnMemoria >= 5) {
                break; // No admitir más procesos si ya hay 5 en memoria
            }
            
            struct Particion* mejor = NULL;
            for (struct Particion* mem = particiones; mem; mem = mem->prox) {
                if (strcmp(mem->idProceso, "") == 0 && temp->tamaño <= mem->tamaño) {
                    if (!mejor || mem->tamaño < mejor->tamaño)
                        mejor = mem;
                }
            }
            if (mejor) {
                strcpy(mejor->idProceso, temp->id);
                mejor->memoriaUsada = temp->tamaño;
                mejor->fragmentacionInterna = mejor->tamaño - temp->tamaño;
                moverProceso(&listos_suspendidos, &listos, temp->id);
                break; // Solo un proceso por ciclo
            }
            temp = sig;
        }

        // C. EJECUCIÓN DE CPU
        enEjecucion = encontrarSRT(listos);
        if (enEjecucion) {
            enEjecucion->tiempoRestante--;
            
            // D. FINALIZACIÓN
            if (enEjecucion->tiempoRestante == 0) {
                enEjecucion->tiempoFinalizacion = tiempo + 1;
                
                // Liberar partición
                for (struct Particion* mem = particiones; mem; mem = mem->prox) {
                    if (strcmp(mem->idProceso, enEjecucion->id) == 0) {
                        strcpy(mem->idProceso, "");
                        mem->memoriaUsada = 0;
                        mem->fragmentacionInterna = 0;
                        break;
                    }
                }
                moverProceso(&listos, &finalizados, enEjecucion->id);
            }
        }

        // Avanzar tiempo
        tiempo++;

        // Guardar el nuevo estado
        estadoActual = guardarEstado(tiempo, particiones, nuevos, listos_suspendidos, listos, finalizados, estadoActual);
    }

    // Mostrar pantalla final con estadísticas
    system("cls");
    printf("===================================================================\n");
    printf("            SIMULACIÓN FINALIZADA EN TIEMPO: %d\n", tiempo);
    printf("===================================================================\n");
    
    imprimirEstadisticas(finalizados, tiempo);
    
    printf("\n\n>>> Presione ESC para cerrar el programa...");
    
    // Esperar hasta que se presione ESC (código 27)
    while (1) {
        char tecla = _getch();
        if (tecla == 27) {  // 27 es el código ASCII de ESC
            break;
        }
    }
    
    // Liberación de memoria del historial
    while (historial) {
        struct Estado* sig = historial->siguiente;
        liberarEstado(historial);
        historial = sig;
    }
    
    return 0;
}
