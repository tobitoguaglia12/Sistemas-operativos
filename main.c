#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

struct Proceso {
    wchar_t id[50];
    int tamaño;
    int tiempoArribo;
    int tiempoInterrupcion;
    struct Proceso* prox;
};

struct Particion {
    wchar_t idParticion[50];
    int tamaño;
    int direccionComienzo;
    int direccionFinal;
    wchar_t idProceso[50];
    int memoriaUsada;
    int fragmentacionInterna;
    struct Particion* prox;
};

int seleccionarArchivo(wchar_t *ruta, size_t tamañoRuta) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Archivos CSV\0*.csv\0Todos los archivos\0*.*\0";
    ofn.lpstrFile = ruta;
    ofn.nMaxFile = tamañoRuta;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = L"csv";
    ofn.lpstrTitle = L"Seleccionar archivo CSV";

    return GetOpenFileNameW(&ofn); // devuelve 1 si se seleccionó archivo
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmdLine, int nShowCmd) {
    wchar_t ruta[MAX_PATH] = L"";

    while (!seleccionarArchivo(ruta, sizeof(ruta) / sizeof(wchar_t))) {
        int respuesta = MessageBoxW(
            NULL,
            L"No se seleccionó ningún archivo.\n¿Desea cerrar el programa?",
            L"Archivo no seleccionado",
            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2
        );

        if (respuesta == IDYES) {
            MessageBoxW(NULL, L"Programa cerrado.", L"Salida", MB_OK | MB_ICONINFORMATION);
            return 1;
        }
    }

    FILE *f = _wfopen(ruta, L"r"); // versión wide de fopen
    if (!f) {
        MessageBoxW(NULL, L"Error al abrir el archivo.", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    wchar_t linea[200];
    wchar_t id[50];
    int tamaño, arribo, interrupcion;
    int contador = 0;

    // Leer líneas del CSV
    while (fgetws(linea, sizeof(linea) / sizeof(wchar_t), f)) {
        if (swscanf(linea, L"%49[^,],%d,%d,%d", id, &tamaño, &arribo, &interrupcion) == 4) {
            contador++;
            if (contador > 10) {
                MessageBoxW(NULL, L"❌ ERROR: El archivo tiene más de 10 procesos.", L"Error", MB_OK | MB_ICONERROR);
                fclose(f);
                return 1;
            }
        }
    }

    rewind(f);
    fgetws(linea, sizeof(linea) / sizeof(wchar_t), f); // saltar encabezado

    wchar_t salida[1024] = L"Procesos leídos:\n";
    while (fgetws(linea, sizeof(linea) / sizeof(wchar_t), f)) {
        if (swscanf(linea, L"%49[^,],%d,%d,%d", id, &tamaño, &arribo, &interrupcion) == 4) {
            wchar_t buffer[128];
            swprintf(buffer, 128, L"%s | Tamaño=%d | Arribo=%d | Interrupción=%d\n",
                     id, tamaño, arribo, interrupcion);
            wcscat(salida, buffer);
        }
    }

    MessageBoxW(NULL, salida, L"Archivo leído correctamente", MB_OK | MB_ICONINFORMATION);
    fclose(f);
    return 0;
}
