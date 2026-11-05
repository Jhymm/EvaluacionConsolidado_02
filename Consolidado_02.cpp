#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

// ===== 1. ESTRUCTURAS DE DATOS BASE (REGISTROS) =====

struct Proceso {
    int id;
    string nombre;
    int prioridad;         // 1: Alta, 2: Media, 3: Baja
    int tiempoEjecucion;   // En segundos
    int memoria;           // En MB
};

// Nodo para Lista Enlazada (Gestor de Procesos)
struct NodoLista {
    Proceso dato;
    NodoLista* siguiente;
};

// Nodo para Cola de Prioridad (Planificador CPU)
struct NodoCola {
    Proceso dato;
    NodoCola* siguiente;
};

// Estructura y Nodo para Pila Dinamica (Gestor de RAM)
struct BloqueMemoria {
    int idBloque;
    int tamanoMB;
    int idProceso; // -1 indica que esta LIBRE
};

struct NodoPila {
    BloqueMemoria dato;
    NodoPila* siguiente;
};
// ===== 2. GESTOR DE PROCESOS (LISTA ENLAZADA) =====

void registrarProceso(NodoLista*& cabeza, Proceso p) {
    NodoLista* nuevo = new NodoLista();
    nuevo->dato = p;
    nuevo->siguiente = NULL;

    if (cabeza == NULL) {
        cabeza = nuevo;
    } else {
        NodoLista* actual = cabeza;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo;
    }
    cout << "-> Proceso [" << p.nombre << "] (ID: " << p.id << ") registrado en la lista.\n";
}

void mostrarProcesos(NodoLista* cabeza) {
    if (cabeza == NULL) {
        cout << "\nNo hay procesos registrados en el sistema.\n";
        return;
    }
    cout << "\n=== LISTA DE PROCESOS REGISTRADOS ===\n";
    NodoLista* actual = cabeza;
    while (actual != NULL) {
        cout << "ID: " << actual->dato.id 
             << " | Nombre: " << actual->dato.nombre 
             << " | Prioridad: " << actual->dato.prioridad 
             << " | Tiempo: " << actual->dato.tiempoEjecucion << "s"
             << " | Memoria: " << actual->dato.memoria << " MB\n";
        actual = actual->siguiente;
    }
}

NodoLista* buscarProcesoPorId(NodoLista* cabeza, int id) {
    NodoLista* actual = cabeza;
    while (actual != NULL) {
        if (actual->dato.id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

void buscarProcesoPorNombre(NodoLista* cabeza, string nombre) {
    NodoLista* actual = cabeza;
    bool encontrado = false;
    cout << "\n=== RESULTADOS DE BUSQUEDA ('" << nombre << "') ===\n";
    while (actual != NULL) {
        if (actual->dato.nombre == nombre) {
            cout << "ID: " << actual->dato.id 
                 << " | Nombre: " << actual->dato.nombre 
                 << " | Prioridad: " << actual->dato.prioridad 
                 << " | Memoria: " << actual->dato.memoria << " MB\n";
            encontrado = true;
        }
        actual = actual->siguiente;
    }
    if (!encontrado) cout << "No se encontro ningun proceso con ese nombre.\n";
}

void modificarPrioridad(NodoLista* cabeza, int id, int nuevaPrioridad) {
    NodoLista* nodo = buscarProcesoPorId(cabeza, id);
    if (nodo != NULL) {
        nodo->dato.prioridad = nuevaPrioridad;
        cout << "-> Prioridad del proceso ID " << id << " actualizada a " << nuevaPrioridad << ".\n";
    } else {
        cout << "-> Proceso no encontrado.\n";
    }
}

bool eliminarProceso(NodoLista*& cabeza, int id) {
    if (cabeza == NULL) return false;

    if (cabeza->dato.id == id) {
        NodoLista* temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp;
        return true;
    }

    NodoLista* actual = cabeza;
    while (actual->siguiente != NULL && actual->siguiente->dato.id != id) {
        actual = actual->siguiente;
    }

    if (actual->siguiente != NULL) {
        NodoLista* temp = actual->siguiente;
        actual->siguiente = actual->siguiente->siguiente;
        delete temp;
        return true;
    }
    return false;
}
// ===== 3. PLANIFICADOR DE CPU (COLA DE PRIORIDAD) =====

void encolarPorPrioridad(NodoCola*& frente, Proceso p) {
    NodoCola* nuevo = new NodoCola();
    nuevo->dato = p;
    nuevo->siguiente = NULL;

    // Insercion ordenada: Menor numero = Mayor prioridad
    if (frente == NULL || p.prioridad < frente->dato.prioridad) {
        nuevo->siguiente = frente;
        frente = nuevo;
    } else {
        NodoCola* actual = frente;
        while (actual->siguiente != NULL && actual->siguiente->dato.prioridad <= p.prioridad) {
            actual = actual->siguiente;
        }
        nuevo->siguiente = actual->siguiente;
        actual->siguiente = nuevo;
    }
    cout << "-> Proceso ID " << p.id << " agregado a la Cola de Prioridad de CPU.\n";
}

void ejecutarProcesoCPU(NodoCola*& frente) {
    if (frente == NULL) {
        cout << "\nNo hay procesos pendientes en la cola de CPU.\n";
        return;
    }
    NodoCola* aEjecutar = frente;
    frente = frente->siguiente;
    cout << "\n>>> [CPU EJECUTANDO] Proceso ID " << aEjecutar->dato.id 
         << " (" << aEjecutar->dato.nombre << ") | Prioridad: " << aEjecutar->dato.prioridad << " <<<\n";
    delete aEjecutar;
}

void mostrarColaCPU(NodoCola* frente) {
    if (frente == NULL) {
        cout << "\nLa cola de CPU esta vacia.\n";
        return;
    }
    cout << "\n=== COLA DE PRIORIDAD DE CPU ===\n";
    NodoCola* actual = frente;
    int pos = 1;
    while (actual != NULL) {
        cout << pos++ << ". [Prioridad " << actual->dato.prioridad << "] ID: " 
             << actual->dato.id << " - " << actual->dato.nombre << "\n";
        actual = actual->siguiente;
    }
}
// ===== 4. GESTOR DE MEMORIA (PILA DINAMICA) =====

void apilarBloque(NodoPila*& tope, BloqueMemoria b) {
    NodoPila* nuevo = new NodoPila();
    nuevo->dato = b;
    nuevo->siguiente = tope;
    tope = nuevo;
}

BloqueMemoria desapilarBloque(NodoPila*& tope) {
    if (tope == NULL) {
        BloqueMemoria nulo = {-1, 0, -1};
        return nulo;
    }
    NodoPila* temp = tope;
    BloqueMemoria b = temp->dato;
    tope = tope->siguiente;
    delete temp;
    return b;
}

void inicializarMemoria(NodoPila*& tope) {
    int tamanos[] = {1024, 512, 256, 128, 64};
    for (int i = 0; i < 5; i++) {
        BloqueMemoria b = {i + 1, tamanos[i], -1};
        apilarBloque(tope, b);
    }
}

bool asignarMemoria(NodoPila*& tope, int idProceso, int memoriaSolicitada) {
    NodoPila* pilaAux = NULL;
    bool asignado = false;

    while (tope != NULL) {
        BloqueMemoria b = desapilarBloque(tope);
        if (!asignado && b.idProceso == -1 && b.tamanoMB >= memoriaSolicitada) {
            b.idProceso = idProceso;
            asignado = true;
            cout << "-> Bloque " << b.idBloque << " (" << b.tamanoMB 
                 << " MB) asignado al Proceso ID " << idProceso << ".\n";
        }
        apilarBloque(pilaAux, b);
    }

    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }

    if (!asignado) cout << "-> No hay bloque de memoria disponible con suficiente capacidad.\n";
    return asignado;
}

bool liberarMemoria(NodoPila*& tope, int idProceso) {
    NodoPila* pilaAux = NULL;
    bool liberado = false;

    while (tope != NULL) {
        BloqueMemoria b = desapilarBloque(tope);
        if (b.idProceso == idProceso) {
            b.idProceso = -1;
            liberado = true;
            cout << "-> Memoria del Bloque " << b.idBloque << " liberada para el Proceso ID " << idProceso << ".\n";
        }
        apilarBloque(pilaAux, b);
    }

    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }

    return liberado;
}

void mostrarMemoria(NodoPila* tope) {
    cout << "\n=== ESTADO DE LA MEMORIA (PILA) ===\n";
    NodoPila* pilaAux = NULL;
    
    while (tope != NULL) {
        BloqueMemoria b = desapilarBloque(tope);
        cout << "| Bloque " << b.idBloque << " (" << b.tamanoMB << " MB) -> ";
        if (b.idProceso == -1) cout << "LIBRE |\n";
        else cout << "OCUPADO por Proceso ID " << b.idProceso << " |\n";
        apilarBloque(pilaAux, b);
    }

    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }
}
// ===== 5. PERSISTENCIA DE DATOS (ARCHIVOS) =====

void guardarEnArchivo(NodoLista* cabeza) {
    ofstream archivo("procesos.txt");
    if (!archivo.is_open()) {
        cout << "Error al abrir el archivo.\n";
        return;
    }
    NodoLista* actual = cabeza;
    while (actual != NULL) {
        archivo << actual->dato.id << "," 
                << actual->dato.nombre << "," 
                << actual->dato.prioridad << "," 
                << actual->dato.tiempoEjecucion << ","
                << actual->dato.memoria << "\n";
        actual = actual->siguiente;
    }
    archivo.close();
    cout << "-> Estado del sistema guardado en 'procesos.txt'.\n";
}

void cargarDesdeArchivo(NodoLista*& cabeza, int& contadorID) {
    ifstream archivo("procesos.txt");
    if (!archivo.is_open()) return;

    string linea;
    int maxID = 0;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string idStr, nombre, prioStr, tiempoStr, memStr;

        getline(ss, idStr, ',');
        getline(ss, nombre, ',');
        getline(ss, prioStr, ',');
        getline(ss, tiempoStr, ',');
        getline(ss, memStr, ',');

        Proceso p;
        
        // Conversion portable compatible con Dev-C++ antiguo (sin usar stoi)
        stringstream ssId(idStr); ssId >> p.id;
        p.nombre = nombre;
        stringstream ssPrio(prioStr); ssPrio >> p.prioridad;
        stringstream ssTiempo(tiempoStr); ssTiempo >> p.tiempoEjecucion;
        stringstream ssMem(memStr); ssMem >> p.memoria;

        if (p.id > maxID) maxID = p.id;
        registrarProceso(cabeza, p);
    }
    archivo.close();
    contadorID = maxID + 1;
    cout << "-> Datos recuperados exitosamente desde 'procesos.txt'.\n";
}
// ===== 6. SUBMENÚS Y MENÚ PRINCIPAL =====

void subMenuProcesos(NodoLista*& listaProcesos, NodoPila*& pilaMemoria, int& contadorID) {
    int op;
    do {
        cout << "\n--- 1. GESTOR DE PROCESOS (LISTA ENLAZADA) ---\n";
        cout << "1. Registrar nuevo proceso\n";
        cout << "2. Eliminar proceso por ID\n";
        cout << "3. Buscar proceso por ID\n";
        cout << "4. Buscar proceso por Nombre\n";
        cout << "5. Modificar prioridad de proceso\n";
        cout << "6. Ver todos los procesos\n";
        cout << "0. Volver al Menú Principal\n";
        cout << "Seleccione una opción: ";
        cin >> op;

        switch (op) {
            case 1: {
                Proceso p;
                p.id = contadorID++;
                cout << "Nombre del proceso: "; cin >> p.nombre;
                cout << "Prioridad (1-Alta, 2-Media, 3-Baja): "; cin >> p.prioridad;
                cout << "Tiempo de ejecucion (seg): "; cin >> p.tiempoEjecucion;
                cout << "Memoria requerida (MB): "; cin >> p.memoria;
                registrarProceso(listaProcesos, p);
                break;
            }
            case 2: {
                int id;
                cout << "ID a eliminar: "; cin >> id;
                if (eliminarProceso(listaProcesos, id)) {
                    liberarMemoria(pilaMemoria, id);
                    cout << "Proceso eliminado de la lista y memoria liberada.\n";
                } else cout << "Proceso no encontrado.\n";
                break;
            }
            case 3: {
                int id;
                cout << "ID a buscar: "; cin >> id;
                NodoLista* n = buscarProcesoPorId(listaProcesos, id);
                if (n != NULL) {
                    cout << "Encontrado: " << n->dato.nombre << " | Prioridad: " << n->dato.prioridad << "\n";
                } else cout << "Proceso no encontrado.\n";
                break;
            }
            case 4: {
                string nombre;
                cout << "Nombre a buscar: "; cin >> nombre;
                buscarProcesoPorNombre(listaProcesos, nombre);
                break;
            }
            case 5: {
                int id, prio;
                cout << "ID del proceso: "; cin >> id;
                cout << "Nueva Prioridad: "; cin >> prio;
                modificarPrioridad(listaProcesos, id, prio);
                break;
            }
            case 6:
                mostrarProcesos(listaProcesos);
                break;
            case 0:
                cout << "Regresando...\n";
                break;
            default:
                cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

void subMenuCPU(NodoCola*& colaCPU, NodoLista* listaProcesos) {
    int op;
    do {
        cout << "\n--- 2. PLANIFICADOR DE CPU (COLA DE PRIORIDAD) ---\n";
        cout << "1. Encolar proceso por prioridad\n";
        cout << "2. Ejecutar proceso (Desencolar)\n";
        cout << "3. Ver cola de CPU\n";
        cout << "0. Volver al Menú Principal\n";
        cout << "Seleccione una opción: ";
        cin >> op;

        switch (op) {
            case 1: {
                int id;
                cout << "ID a encolar: "; cin >> id;
                NodoLista* n = buscarProcesoPorId(listaProcesos, id);
                if (n != NULL) encolarPorPrioridad(colaCPU, n->dato);
                else cout << "Proceso no encontrado en la lista.\n";
                break;
            }
            case 2:
                ejecutarProcesoCPU(colaCPU);
                break;
            case 3:
                mostrarColaCPU(colaCPU);
                break;
            case 0:
                cout << "Regresando...\n";
                break;
            default:
                cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

void subMenuMemoria(NodoPila*& pilaMemoria, NodoLista* listaProcesos) {
    int op;
    do {
        cout << "\n--- 3. GESTOR DE MEMORIA (PILA DINÁMICA) ---\n";
        cout << "1. Asignar memoria a proceso (Push)\n";
        cout << "2. Liberar memoria de proceso (Pop)\n";
        cout << "3. Ver estado actual de memoria\n";
        cout << "0. Volver al Menú Principal\n";
        cout << "Seleccione una opción: ";
        cin >> op;

        switch (op) {
            case 1: {
                int id;
                cout << "ID del proceso a asignar memoria: "; cin >> id;
                NodoLista* n = buscarProcesoPorId(listaProcesos, id);
                if (n != NULL) asignarMemoria(pilaMemoria, id, n->dato.memoria);
                else cout << "Proceso no encontrado.\n";
                break;
            }
            case 2: {
                int id;
                cout << "ID del proceso a liberar memoria: "; cin >> id;
                if (!liberarMemoria(pilaMemoria, id)) {
                    cout << "El proceso no tenía bloques asignados.\n";
                }
                break;
            }
            case 3:
                mostrarMemoria(pilaMemoria);
                break;
            case 0:
                cout << "Regresando...\n";
                break;
            default:
                cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

int main() {
    NodoLista* listaProcesos = NULL;
    NodoCola* colaCPU = NULL;
    NodoPila* pilaMemoria = NULL;
    int contadorID = 1;

    inicializarMemoria(pilaMemoria);
    cargarDesdeArchivo(listaProcesos, contadorID);

    int opcionPrincipal;
    do {
        cout << "\n=========================================\n";
        cout << "     SISTEMA DE GESTION DE PROCESOS\n";
        cout << "=========================================\n";
        cout << "1. Gestor de Procesos\n";
        cout << "2. Planificador de CPU\n";
        cout << "3. Gestor de Memoria\n";
        cout << "4. Guardar Estado y Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> opcionPrincipal;

        switch (opcionPrincipal) {
            case 1:
                subMenuProcesos(listaProcesos, pilaMemoria, contadorID);
                break;
            case 2:
                subMenuCPU(colaCPU, listaProcesos);
                break;
            case 3:
                subMenuMemoria(pilaMemoria, listaProcesos);
                break;
            case 4:
                guardarEnArchivo(listaProcesos);
                cout << "Guardando datos y saliendo del sistema...\n";
                break;
            default:
                cout << "Opción inválida.\n";
        }
    } while (opcionPrincipal != 4);

    return 0;
}


