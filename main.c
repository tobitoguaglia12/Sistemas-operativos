#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>   // Para la API de Windows (MessageBox, GetOpenFileName)
#include <commdlg.h>   // Para la estructura OPENFILENAME

// --- ESTRUCTURAS ---
// (Usamos char normal para la lógica interna y la impresión en consola)
struct Proceso {
    char id[50];
    int tamaño;
    int tiempoArribo;
    int tiempoInterrupcion;
    int tiempoRestante;
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

// --- PROTOTIPOS DE FUNCIONES ---
void liberarListaProcesos(struct Proceso* cabeza);
void liberarListaParticiones(struct Particion* cabeza);
struct Particion* inicializarMemoria();
struct Proceso* encontrarSRT(struct Proceso* cabeza);
void moverProceso(struct Proceso** origen, struct Proceso** destino, const char* idProceso);
void imprimirEstado(int tiempo, struct Particion* p, struct Proceso* n, struct Proceso* ls, struct Proceso* l, struct Proceso* f, const char* evento);
int seleccionarArchivo(wchar_t* ruta, size_t tamañoRuta);

// --- FUNCIÓN PRINCIPAL Y SIMULACIÓN ---
int main() {
    // 1. Selección de archivo con la GUI de Windows
    wchar_t rutaArchivo[MAX_PATH] = {0}; // Usamos wchar_t para la API de Windows
    if (!seleccionarArchivo(rutaArchivo, MAX_PATH)) {
        MessageBoxW(NULL, L"No se seleccionó ningún archivo. El programa se cerrará.", L"Archivo no seleccionado", MB_OK | MB_ICONWARNING);
        return 1;
    }
    
    // _wfopen es la versión de fopen que acepta rutas con wchar_t
    FILE *f = _wfopen(rutaArchivo, L"r"); 
    if (!f) {
        printf("Error: No se pudo abrir el archivo seleccionado.\n");
        return 1;
    }

    // El resto de la carga y simulación es idéntica
    struct Proceso* nuevos = NULL;
    char linea[256];
    while (fgets(linea, sizeof(linea), f)) {
        struct Proceso* p = (struct Proceso*)malloc(sizeof(struct Proceso));
        if (!p) continue;
        if (sscanf(linea, "%49[^,],%d,%d,%d", p->id, &p->tamaño, &p->tiempoArribo, &p->tiempoInterrupcion) == 4) {
            p->tiempoRestante = p->tiempoInterrupcion;
            p->prox = nuevos;
            nuevos = p;
        } else {
            free(p);
        }
    }
    fclose(f);

    // 2. Inicialización
    struct Particion* particiones = inicializarMemoria();
    struct Proceso* listos_suspendidos = NULL;
    struct Proceso* listos = NULL;
    struct Proceso* finalizados = NULL;
    int tiempo = 0;

    printf("Iniciando simulacion con el archivo seleccionado...\n\n");

    // --- 3. BUCLE PRINCIPAL DE SIMULACIÓN ---
    while (nuevos != NULL || listos_suspendidos != NULL || listos != NULL) {
        char evento[512] = "";
        
        // A. LLEGADA DE PROCESOS
        struct Proceso* pNuevos = nuevos;
        while (pNuevos != NULL) {
            struct Proceso* sig = pNuevos->prox;
            if (pNuevos->tiempoArribo <= tiempo) {
                sprintf(evento + strlen(evento), "Llega %s. ", pNuevos->id);
                moverProceso(&nuevos, &listos_suspendidos, pNuevos->id);
            }
            pNuevos = sig;
        }

        // B. ADMISIÓN EN MEMORIA (Best-Fit)
        struct Proceso* pSuspendido = listos_suspendidos;
        while(pSuspendido != NULL) {
            struct Proceso* sig = pSuspendido->prox;
            struct Particion* mejorParticion = NULL;
            struct Particion* pMem = particiones;
            while(pMem != NULL) {
                if(strcmp(pMem->idProceso, "") == 0 && pSuspendido->tamaño <= pMem->tamaño) {
                    if(mejorParticion == NULL || pMem->tamaño < mejorParticion->tamaño) mejorParticion = pMem;
                }
                pMem = pMem->prox;
            }
            if (mejorParticion != NULL) {
                strcpy(mejorParticion->idProceso, pSuspendido->id);
                mejorParticion->memoriaUsada = pSuspendido->tamaño;
                mejorParticion->fragmentacionInterna = mejorParticion->tamaño - pSuspendido->tamaño;
                sprintf(evento + strlen(evento), "%s entra en particion %s. ", pSuspendido->id, mejorParticion->idParticion);
                moverProceso(&listos_suspendidos, &listos, pSuspendido->id);
            }
            pSuspendido = sig;
        }

        // C. PLANIFICACIÓN DE CPU (SRTF)
        struct Proceso* procesoEnEjecucion = encontrarSRT(listos);
        if (procesoEnEjecucion != NULL) {
            sprintf(evento + strlen(evento), "CPU ejecuta %s (restante %d).", procesoEnEjecucion->id, procesoEnEjecucion->tiempoRestante);
            procesoEnEjecucion->tiempoRestante--;
            // D. FINALIZACIÓN DE PROCESO
            if (procesoEnEjecucion->tiempoRestante == 0) {
                sprintf(evento + strlen(evento), " %s TERMINA.", procesoEnEjecucion->id);
                struct Particion* pMem = particiones;
                while(pMem != NULL) {
                    if (strcmp(pMem->idProceso, procesoEnEjecucion->id) == 0) {
                        strcpy(pMem->idProceso, ""); pMem->memoriaUsada = 0; pMem->fragmentacionInterna = 0;
                        break;
                    }
                    pMem = pMem->prox;
                }
                moverProceso(&listos, &finalizados, procesoEnEjecucion->id);
            }
        } else {
            strcat(evento, "CPU Ocioso.");
        }

        imprimirEstado(tiempo, particiones, nuevos, listos_suspendidos, listos, finalizados, evento);
        
        tiempo++;
        if (tiempo > 200) { printf("\n\nSIMULACION DETENIDA POR LIMITE DE TIEMPO (200s).\n"); break; }
    }
    
    printf("\n\n==============================================\n");
    printf("--- SIMULACION FINALIZADA EN TIEMPO: %d ---\n", tiempo > 0 ? tiempo - 1 : 0);
    printf("==============================================\n");

    // 5. Limpieza
    liberarListaProcesos(nuevos);
    liberarListaProcesos(listos_suspendidos);
    liberarListaProcesos(listos);
    liberarListaProcesos(finalizados);
    liberarListaParticiones(particiones);

    return 0;
}

// --- FUNCIONES AUXILIARES ---

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
    return GetOpenFileNameW(&ofn); // Devuelve 1 si se seleccionó, 0 si se canceló
}

struct Particion* inicializarMemoria() {
    struct Particion* pSO = (struct Particion*)malloc(sizeof(struct Particion));
    struct Particion* pGrande = (struct Particion*)malloc(sizeof(struct Particion));
    struct Particion* pMediana = (struct Particion*)malloc(sizeof(struct Particion));
    struct Particion* pPequeña = (struct Particion*)malloc(sizeof(struct Particion));
    if (!pSO || !pGrande || !pMediana || !pPequeña) return NULL;
    strcpy(pSO->idParticion, "SO"); strcpy(pSO->idProceso, "SO"); pSO->tamaño = 100; pSO->memoriaUsada = 100; pSO->fragmentacionInterna = 0;
    strcpy(pGrande->idParticion, "Grande"); strcpy(pGrande->idProceso, ""); pGrande->tamaño = 250; pGrande->memoriaUsada = 0; pGrande->fragmentacionInterna = 0;
    strcpy(pMediana->idParticion, "Mediana"); strcpy(pMediana->idProceso, ""); pMediana->tamaño = 150; pMediana->memoriaUsada = 0; pMediana->fragmentacionInterna = 0;
    strcpy(pPequeña->idParticion, "Pequeña"); strcpy(pPequeña->idProceso, ""); pPequeña->tamaño = 50; pPequeña->memoriaUsada = 0; pPequeña->fragmentacionInterna = 0;
    pSO->prox = pGrande; pGrande->prox = pMediana; pMediana->prox = pPequeña; pPequeña->prox = NULL;
    return pSO;
}

struct Proceso* encontrarSRT(struct Proceso* cabeza) {
    if (cabeza == NULL) return NULL;
    struct Proceso* procesoMin = cabeza;
    struct Proceso* actual = cabeza->prox;
    while (actual != NULL) {
        if (actual->tiempoRestante < procesoMin->tiempoRestante) procesoMin = actual;
        actual = actual->prox;
    }
    return procesoMin;
}

void moverProceso(struct Proceso** origen, struct Proceso** destino, const char* idProceso) {
    struct Proceso* actual = *origen, *anterior = NULL;
    while (actual != NULL && strcmp(actual->id, idProceso) != 0) {
        anterior = actual;
        actual = actual->prox;
    }
    if (actual == NULL) return;
    if (anterior == NULL) *origen = actual->prox;
    else anterior->prox = actual->prox;
    actual->prox = *destino;
    *destino = actual;
}

void liberarListaProcesos(struct Proceso* cabeza) {
    struct Proceso* actual = cabeza;
    while (actual != NULL) {
        struct Proceso* siguiente = actual->prox;
        free(actual);
        actual = siguiente;
    }
}

void liberarListaParticiones(struct Particion* cabeza) {
    struct Particion* actual = cabeza;
    while (actual != NULL) {
        struct Particion* siguiente = actual->prox;
        free(actual);
        actual = siguiente;
    }
}

void imprimirEstado(int tiempo, struct Particion* p, struct Proceso* n, struct Proceso* ls, struct Proceso* l, struct Proceso* f, const char* evento) {
    printf("==================== TIEMPO: %d ====================\n", tiempo);
    printf("EVENTO: %s\n", evento);
    printf("\n--- Particiones de Memoria ---\n");
    struct Particion* p_temp = p;
    while(p_temp) {
        if(strcmp(p_temp->idProceso, "") != 0) {
            printf("  > %-8s: Ocupada por [%-3s] | Tam: %3dK, Usado: %3dK, Frag: %3dK\n",
                   p_temp->idParticion, p_temp->idProceso, p_temp->tamaño, p_temp->memoriaUsada, p_temp->fragmentacionInterna);
        } else {
            printf("  > %-8s: Libre               | Tam: %3dK\n", p_temp->idParticion, p_temp->tamaño);
        }
        p_temp = p_temp->prox;
    }
    printf("\n--- Colas de Procesos ---\n");
    struct Proceso* temp;
    printf("  NUEVOS              : [ "); temp = n; while(temp) { printf("%s ", temp->id); temp = temp->prox; } printf("]\n");
    printf("  LISTOS-SUSPENDIDOS  : [ "); temp = ls; while(temp) { printf("%s ", temp->id); temp = temp->prox; } printf("]\n");
    printf("  LISTOS (en memoria) : [ "); temp = l; while(temp) { printf("%s ", temp->id); temp = temp->prox; } printf("]\n");
    printf("  FINALIZADOS         : [ "); temp = f; while(temp) { printf("%s ", temp->id); temp = temp->prox; } printf("]\n");
    printf("==================================================\n\n");
}