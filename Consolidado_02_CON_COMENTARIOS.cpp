#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

// ============================================================================
// ===== 1. ESTRUCTURAS DE DATOS BASE (REGISTROS) =============================
// ============================================================================

// Registro que representa las propiedades de un proceso del sistema
struct Proceso {
    int id;                // Identificador único (1, 2, 3...)
    string nombre;         // Nombre del proceso
    int prioridad;         // Prioridad (1: Alta, 2: Media, 3: Baja)
    int tiempoEjecucion;   // Duración en segundos
    int memoria;           // Requerimiento de memoria en MB
};

// Nodo para Lista Enlazada Simplemente Ligada (Gestor de Procesos)
struct NodoLista {
    Proceso dato;
    NodoLista* siguiente;
};

// Nodo para Cola de Prioridad (Planificador CPU)
struct NodoCola {
    Proceso dato;
    NodoCola* siguiente;
};

// Registro de Bloque de RAM y Nodo para Pila Dinámica (Gestor de Memoria)
struct BloqueMemoria {
    int idBloque;   // ID del bloque físico (1 a 5)
    int tamanoMB;   // Tamaño del bloque en MB
    int idProceso;  // ID del proceso asignado (-1 indica que el bloque está LIBRE)
};

struct NodoPila {
    BloqueMemoria dato;
    NodoPila* siguiente;
};

// ============================================================================
// ===== 2. GESTOR DE PROCESOS (LISTA ENLAZADA) ===============================
// ============================================================================

// Agrega un nuevo proceso al final de la Lista Enlazada
void registrarProceso(NodoLista*& cabeza, Proceso p) {
    NodoLista* nuevo = new NodoLista(); // Asignación de memoria dinámica
    nuevo->dato = p;
    nuevo->siguiente = NULL;

    if (cabeza == NULL) {
        cabeza = nuevo; // Caso 1: La lista está vacía, se define la cabeza
    } else {
        NodoLista* actual = cabeza;
        while (actual->siguiente != NULL) { // Recorrido hasta el último nodo
            actual = actual->siguiente;
        }
        actual->siguiente = nuevo; // Conexión del nuevo nodo al final
    }
    cout << "-> Proceso [" << p.nombre << "] (ID: " << p.id << ") registrado en la lista.\n";
}

// Imprime todos los procesos registrados en la lista
void mostrarProcesos(NodoLista* cabeza) {
    if (cabeza == NULL) {
        cout << "\nNo hay procesos registrados en el sistema.\n";
        return;
    }
    cout << "\n=== LISTA DE PROCESOS REGISTRADOS ===\n";
    NodoLista* actual = cabeza;
    while (actual != NULL) { // Iteración hasta el fin de la lista (NULL)
        cout << "ID: " << actual->dato.id 
             << " | Nombre: " << actual->dato.nombre 
             << " | Prioridad: " << actual->dato.prioridad 
             << " | Tiempo: " << actual->dato.tiempoEjecucion << "s"
             << " | Memoria: " << actual->dato.memoria << " MB\n";
        actual = actual->siguiente;
    }
}

// Busca un proceso por ID y retorna la dirección del nodo (o NULL si no existe)
NodoLista* buscarProcesoPorId(NodoLista* cabeza, int id) {
    NodoLista* actual = cabeza;
    while (actual != NULL) {
        if (actual->dato.id == id) return actual; // Retorna el puntero al nodo
        actual = actual->siguiente;
    }
    return NULL;
}

// Busca e imprime todas las coincidencias asociadas a un nombre
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

// Actualiza el campo prioridad del proceso seleccionado por ID
void modificarPrioridad(NodoLista* cabeza, int id, int nuevaPrioridad) {
    NodoLista* nodo = buscarProcesoPorId(cabeza, id);
    if (nodo != NULL) {
        nodo->dato.prioridad = nuevaPrioridad;
        cout << "-> Prioridad del proceso ID " << id << " actualizada a " << nuevaPrioridad << ".\n";
    } else {
        cout << "-> Proceso no encontrado.\n";
    }
}

// Elimina un nodo de la Lista Enlazada por ID y libera su memoria
bool eliminarProceso(NodoLista*& cabeza, int id) {
    if (cabeza == NULL) return false;

    // Caso A: El elemento a eliminar es la cabeza de la lista
    if (cabeza->dato.id == id) {
        NodoLista* temp = cabeza;
        cabeza = cabeza->siguiente;
        delete temp; // Liberación de memoria dinámica
        return true;
    }

    // Caso B: El elemento está en el cuerpo o final de la lista
    NodoLista* actual = cabeza;
    while (actual->siguiente != NULL && actual->siguiente->dato.id != id) {
        actual = actual->siguiente;
    }

    if (actual->siguiente != NULL) {
        NodoLista* temp = actual->siguiente;
        actual->siguiente = actual->siguiente->siguiente; // Reconexión de punteros
        delete temp; // Liberación de memoria dinámica
        return true;
    }
    return false;
}

// ============================================================================
// ===== 3. PLANIFICADOR DE CPU (COLA DE PRIORIDAD) ===========================
// ============================================================================

// Inserta de forma ordenada en la cola (Menor valor numérico = Mayor prioridad)
void encolarPorPrioridad(NodoCola*& frente, Proceso p) {
    NodoCola* nuevo = new NodoCola();
    nuevo->dato = p;
    nuevo->siguiente = NULL;

    // Caso 1: La cola está vacía o el proceso entrante tiene mayor prioridad que el primero
    if (frente == NULL || p.prioridad < frente->dato.prioridad) {
        nuevo->siguiente = frente;
        frente = nuevo;
    } else {
        // Caso 2: Buscar la posición correcta dentro de la cola
        NodoCola* actual = frente;
        while (actual->siguiente != NULL && actual->siguiente->dato.prioridad <= p.prioridad) {
            actual = actual->siguiente;
        }
        nuevo->siguiente = actual->siguiente;
        actual->siguiente = nuevo;
    }
    cout << "-> Proceso ID " << p.id << " agregado a la Cola de Prioridad de CPU.\n";
}

// Extrae el elemento del frente de la cola (mayor prioridad) y simula su ejecución
void ejecutarProcesoCPU(NodoCola*& frente) {
    if (frente == NULL) {
        cout << "\nNo hay procesos pendientes en la cola de CPU.\n";
        return;
    }
    NodoCola* aEjecutar = frente; // Tomar elemento del frente
    frente = frente->siguiente;    // Desplazar el frente al siguiente nodo
    cout << "\n>>> [CPU EJECUTANDO] Proceso ID " << aEjecutar->dato.id 
         << " (" << aEjecutar->dato.nombre << ") | Prioridad: " << aEjecutar->dato.prioridad << " <<<\n";
    delete aEjecutar; // Liberar nodo de la cola
}

// Muestra los elementos formados en la cola de la CPU
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

// ============================================================================
// ===== 4. GESTOR DE MEMORIA (PILA DINÁMICA) =================================
// ============================================================================

// PUSH: Coloca un elemento en el tope de la Pila
void apilarBloque(NodoPila*& tope, BloqueMemoria b) {
    NodoPila* nuevo = new NodoPila();
    nuevo->dato = b;
    nuevo->siguiente = tope;
    tope = nuevo; // El tope apunta al nuevo nodo
}

// POP: Retira y retorna el elemento del tope de la Pila
BloqueMemoria desapilarBloque(NodoPila*& tope) {
    if (tope == NULL) {
        BloqueMemoria nulo = {-1, 0, -1};
        return nulo;
    }
    NodoPila* temp = tope;
    BloqueMemoria b = temp->dato;
    tope = tope->siguiente; // El tope desciende al siguiente nodo
    delete temp;
    return b;
}

// Inicializa la memoria RAM creando 5 bloques con capacidades fijas
void inicializarMemoria(NodoPila*& tope) {
    int tamanos[] = {1024, 512, 256, 128, 64};
    for (int i = 0; i < 5; i++) {
        BloqueMemoria b = {i + 1, tamanos[i], -1}; // idProceso = -1 (LIBRE)
        apilarBloque(tope, b);
    }
}

// Busca un bloque apto desapilando temporalmente hacia una Pila Auxiliar
bool asignarMemoria(NodoPila*& tope, int idProceso, int memoriaSolicitada) {
    NodoPila* pilaAux = NULL;
    bool asignado = false;

    // Desapilar y verificar disponibilidad
    while (tope != NULL) {
        BloqueMemoria b = desapilarBloque(tope);
        if (!asignado && b.idProceso == -1 && b.tamanoMB >= memoriaSolicitada) {
            b.idProceso = idProceso; // Asignar ID al bloque
            asignado = true;
            cout << "-> Bloque " << b.idBloque << " (" << b.tamanoMB 
                 << " MB) asignado al Proceso ID " << idProceso << ".\n";
        }
        apilarBloque(pilaAux, b); // Pasar a la pila auxiliar
    }

    // Restaurar el contenido desde la Pila Auxiliar a la Pila Principal
    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }

    if (!asignado) cout << "-> No hay bloque de memoria disponible con suficiente capacidad.\n";
    return asignado;
}

// Marca como LIBRE (-1) el bloque asignado a un ID
bool liberarMemoria(NodoPila*& tope, int idProceso) {
    NodoPila* pilaAux = NULL;
    bool liberado = false;

    while (tope != NULL) {
        BloqueMemoria b = desapilarBloque(tope);
        if (b.idProceso == idProceso) {
            b.idProceso = -1; // Liberar bloque
            liberado = true;
            cout << "-> Memoria del Bloque " << b.idBloque << " liberada para el Proceso ID " << idProceso << ".\n";
        }
        apilarBloque(pilaAux, b);
    }

    // Restaurar Pila Principal
    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }

    return liberado;
}

// Recorre desapilando temporalmente para visualizar la estructura de la RAM
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

    // Restaurar Pila Principal
    while (pilaAux != NULL) {
        apilarBloque(tope, desapilarBloque(pilaAux));
    }
}

// ============================================================================
// ===== 5. PERSISTENCIA DE DATOS (ARCHIVOS) ==================================
// ============================================================================

// Guarda el estado de la Lista Enlazada en un archivo 'procesos.txt'
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

// Carga la información desde 'procesos.txt' al iniciar la ejecución
void cargarDesdeArchivo(NodoLista*& cabeza, int& contadorID) {
    ifstream archivo("procesos.txt");
    if (!archivo.is_open()) return;

    string linea;
    int maxID = 0;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string idStr, nombre, prioStr, tiempoStr, memStr;

        // Lectura de valores delimitados por coma (CSV)
        getline(ss, idStr, ',');
        getline(ss, nombre, ',');
        getline(ss, prioStr, ',');
        getline(ss, tiempoStr, ',');
        getline(ss, memStr, ',');

        Proceso p;
        stringstream ssId(idStr); ssId >> p.id;
        p.nombre = nombre;
        stringstream ssPrio(prioStr); ssPrio >> p.prioridad;
        stringstream ssTiempo(tiempoStr); ssTiempo >> p.tiempoEjecucion;
        stringstream ssMem(memStr); ssMem >> p.memoria;

        if (p.id > maxID) maxID = p.id;
        registrarProceso(cabeza, p);
    }
    archivo.close();
    contadorID = maxID + 1; // Siguiente ID disponible
    cout << "-> Datos recuperados exitosamente desde 'procesos.txt'.\n";
}

// ============================================================================
// ===== 6. INTERFAZ Y CONTROLADORES DE MENÚ ==================================
// ============================================================================

// Submenú 1: Operaciones del Gestor de Procesos
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
                    liberarMemoria(pilaMemoria, id); // Al eliminar proceso, desasigna su bloque RAM
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

// Submenú 2: Operaciones del Planificador de CPU
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

// Submenú 3: Operaciones del Gestor de Memoria RAM
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

// Función principal
int main() {
    // Declaración e inicialización de punteros de cabecera
    NodoLista* listaProcesos = NULL;
    NodoCola* colaCPU = NULL;
    NodoPila* pilaMemoria = NULL;
    int contadorID = 1;

    // Inicialización del sistema
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

