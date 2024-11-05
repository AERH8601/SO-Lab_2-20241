# Ejercicios de programación #

A continuación se colocan unos ejercicios de análisis y programación relacionados con el API de procesos con el objetivo de evaluar la comprensión y uso de dicho API en la solución de problemas de programación en C.

## Análisis de código ##

1. Dado el siguiente programa, explique cual es la salida de la línea comentada como ```LINE A```

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int value = 5;
int main() {
    pid t pid;
    pid = fork();
    if (pid == 0) { /* child process */
        value += 15;
        return 0;
    }
    else if (pid > 0) { /* parent process */
        wait(NULL);
        printf("PARENT: value = %d",value); /* LINE A */
        return 0;
    }
}
```
R/ Debido a que el proceso hijo y el proceso padre tienen espacios de memoria independientes (a excepción de los recursos compartidos explícitamente), los cambios realizados por el hijo no afectan al padre, y el valor de value en el padre sigue siendo 5 cuando se ejecuta la línea ```LINE A```.

=================================================================


2. Incluyendo el proceso padre inicial ¿cuántos procesos son creados por el programa mostrado a continuación?

```C
#include <stdio.h>
#include <unistd.h>
int main() {
    /* fork a child process */
    fork();
    /* fork another child process */
    fork();
    /* and fork another */
    fork();
    return 0;
}
```
R/ 
- Comienza con 1 proceso (el proceso padre original).
- Primera llamada a fork(): 1 proceso padre + 1 proceso hijo = 2 procesos.
- Segunda llamada a fork(): 2 procesos originales cada uno crea un hijo = 4 procesos.
- Tercera llamada a fork(): 4 procesos originales cada uno crea un hijo = 8 procesos.

El número total de procesos al finalizar el programa es 8. Esto incluye el proceso padre original y los procesos hijos creados por cada llamada a ```fork()```.

================================================

3. Incluyendo el proceso padre inicial ¿cuántos procesos  son creados por el programa mostrado a continuación?

```C
#include <stdio.h>
#include <unistd.h>

int main() {
    int i;
    for (i = 0; i < 4; i++) // El bucle se ejecuta 4 veces.
        fork();             // Llama a fork() en cada iteración del bucle.
    return 0;
}
```
R/  
- Proceso Padre Inicial: 1 proceso.
- Primera Iteración (fork() en i = 0): 2 procesos en total.
- Segunda Iteración (fork() en i = 1): 4 procesos en total.
- Tercera Iteración (fork() en i = 2): 8 procesos en total.
- Cuarta Iteración (fork() en i = 3): 16 procesos en total.

El número total de procesos al finalizar el programa es 16. Esto incluye el proceso padre original y todos los procesos hijos creados durante las iteraciones del bucle ```for```.

==========================================================

4. En el siguiente código ¿la linea de codigo marcada como ```printf("LINE J")```  imprimirá el mensaje?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    pid t pid;      // Declara una variable `pid` de tipo `pid_t`.

    /* fork a child process */
    pid = fork();       // Crea un proceso hijo.

    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;       // Si `fork()` falla, imprime un mensaje de error y sale del programa.
    }
    else if (pid == 0) { /* child process */
                // Proceso hijo
        execlp("/bin/ls","ls",NULL);    // Reemplaza el proceso hijo con el comando `ls`.
        printf("LINE J");       // Línea en cuestión
    }
    else { /* parent process */
         // Proceso padre
        /* parent will wait for the child to complete */
        wait(NULL);     // El padre espera que el proceso hijo termine.
        printf("Child Complete");   // Imprime que el proceso hijo ha terminado.
    }
    return 0;
}
```
R/ 
- La llamada a execlp() reemplaza el proceso hijo por el programa /bin/ls.
- Si execlp() tiene éxito, el proceso hijo ya no ejecutará las líneas de código restantes en la función principal, incluyendo printf("LINE J");.
- LINE J no se imprimirá a menos que execlp() falle.
Por lo tanto, en condiciones normales, printf("LINE J"); no imprimirá el mensaje.

==============================================

5. Asumiendo que se tienen como pids para el padre y para el hijo los valores de 2600 y 2603 respectivamente. Identifique los valores del pid en las líneas comentadas como ```A```, ```B```, ```C``` y ```D```

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    pid t pid, pid1;

    /* fork a child process */
    pid = fork();       // Se crea un proceso hijo. `pid` tendrá valores diferentes en padre e hijo.

    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { /* child process */
            // Este bloque es ejecutado por el proceso hijo.
        pid1 = getpid();         // `pid1` se asigna al PID del proceso hijo.
        printf("child: pid = %d",pid); /* A */
        printf("child: pid1 = %d",pid1); /* B */
    }
    else { /* parent process */
        // Este bloque es ejecutado por el proceso padre.
        pid1 = getpid();        // `pid1` se asigna al PID del proceso padre.
        printf("parent: pid = %d",pid); /* C */
        printf("parent: pid1 = %d",pid1); /* D */
        wait(NULL);
    }
    return 0;
}
```

R/
**Resumen de los Valores:**
Línea A (child: pid = %d): 0
Línea B (child: pid1 = %d): 2603
Línea C (parent: pid = %d): 2603
Línea D (parent: pid1 = %d): 2600

**Explicación:**
- En el proceso hijo, ```pid``` es ```0```, ya que fork() devuelve 0 al proceso hijo.
- En el proceso hijo, ```pid1``` es 2603, que es el PID del hijo.
- En el proceso padre, ```pid``` es 2603, que es el PID del proceso hijo retornado por ```fork()```.
- En el proceso padre, ```pid1``` es 2600, que es el PID del proceso padre.

===========================================

6. Dado el siguiente programa, ¿cuál sería la salida desplegada en las líneas comentadas como ```LINE X``` y ```LINE Y```?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#define SIZE 5
int nums[SIZE] = {0,1,2,3,4}; // Array global compartido entre el proceso padre e hijo (con copias separadas).

int main() {
    int i;
    pid t pid;

    pid = fork();    // Crea un proceso hijo.

    if (pid == 0) { // Código ejecutado por el proceso hijo.
        for (i = 0; i < SIZE; i++) {
            nums[i] *= -i;  // Modifica el array `nums` en el proceso hijo.
            printf("CHILD: %d ",nums[i]); /* LINE X */
        }
    }
    else if (pid > 0) {     // Código ejecutado por el proceso padre.
        wait(NULL);     // El proceso padre espera a que el hijo termine antes de continuar.
        for (i = 0; i < SIZE; i++)
            printf("PARENT: %d ",nums[i]); /* LINE Y */
    }
    return 0;
}
```
R/
- Cuando se llama a fork(), se crea un proceso hijo que tiene una copia independiente de todas las variables del proceso padre. Esto significa que cualquier cambio realizado por el proceso hijo no afecta las variables en el proceso padre, y viceversa.
- En este programa, el proceso hijo modifica su copia del array nums, mientras que el proceso padre conserva los valores originales. Por eso, la salida en LINE Y muestra los valores originales del array.

================================================== 

## Problemas de programación ##

1. Escriba un programa que abra un archivo (con la llamada ```open()```) y entonces llame a ```fork()```. Nota: El siguiente [enlace](https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/) puede ser de utilidad para entender la llamada open().

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> // Para la llamada a open()

```C
int main() {
    int fd; // File descriptor para el archivo.
    pid_t pid;

    // Abrir el archivo en modo lectura-escritura (crear si no existe).
    fd = open("example.txt", O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Crear el proceso hijo.
    pid = fork();
    if (pid < 0) {
        perror("Fork falló");
        return 1;
    }
    else if (pid == 0) { // Código del proceso hijo.
        write(fd, "Esto es del proceso hijo\n", 25);
    }
    else { // Código del proceso padre.
        write(fd, "Esto es del proceso padre\n", 26);
        wait(NULL); // Esperar al proceso hijo.
    }

    close(fd); // Cerrar el archivo.
    return 0;
}
```

   * ¿Pueden el padre y el hijo acceder al file descriptor retornado por ```open()```?
   R/
   Sí, tanto el proceso padre como el proceso hijo pueden acceder al file descriptor que se retornó por la llamada a ```open()```.

Cuando se llama a ```fork()```, el proceso hijo hereda una copia del espacio de direcciones del proceso padre, incluyendo los descriptores de archivo abiertos. Esto significa que el proceso hijo y el proceso padre comparten el mismo file descriptor. Ambos tienen una referencia al mismo archivo abierto, lo que les permite leer o escribir en él.

   * ¿Qué pasa si ellos empiezan a escribir el archivo de manera concurrente, es decir, a la misma vez?
   R/
Si el proceso padre y el proceso hijo empiezan a escribir en el archivo concurrentemente, puede ocurrir condiciones de carrera y el resultado en el archivo puede ser indeterminado o inconsistente.

- Tanto el proceso padre como el hijo están compartiendo el mismo file descriptor, lo cual significa que ambos procesos comparten la posición del puntero de archivo. Si escriben simultáneamente sin ninguna forma de sincronización, el puntero de archivo puede cambiar impredeciblemente, lo que causa que los datos se mezclen.
- Por ejemplo, si el proceso padre comienza a escribir algo y luego, mientras está escribiendo, el proceso hijo también escribe, los dos flujos de datos se entremezclarán. La salida del archivo puede terminar con fragmentos de ambos mensajes mezclados, o uno de los mensajes puede sobrescribir partes del otro.  

Conclusión, aunque ambos procesos pueden acceder al file descriptor compartido, escribir en el archivo concurrentemente sin control puede resultar en una salida inconsistente debido a condiciones de carrera.

=============================

2. Escriba un programa usando fork(). El proceso hijo imprimirá ```"Hello"```; el proceso padre imprimirá ```"goodbye"```. Usted deberá asegurar que el proceso hijo imprima siempre en primer lugar.

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // Para la llamada a wait()

int main() {
    pid_t pid;

    // Crear un proceso hijo
    pid = fork();

    if (pid < 0) { // Si la llamada a fork() falla
        perror("Fork falló");
        return 1;
    }
    else if (pid == 0) { // Código ejecutado por el proceso hijo
        printf("Hello\n");
    }
    else { // Código ejecutado por el proceso padre
        wait(NULL); // El padre espera a que el hijo termine
        printf("Goodbye\n");
    }

    return 0;
}
```

- La función ```wait(NULL)``` en el proceso padre garantiza que el proceso hijo termine primero.
- Esto asegura que "Hello" siempre se imprima antes de "Goodbye".

=============================================================

3. Escriba un programa que llame ```fork()``` y entonces llame alguna forma de exec() para correr el programa ```/bin/ls```. Intente probar todas las variaciones de la familia de funciones ```exec()``` incluyendo (en linux) ```execl()```, ```execle()```, ```execlp()```, ```execv()```, ```execvp()``` y ```execvpe()```. ¿Por qué piensa usted que existen tantas variaciones para la misma llamada básica?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // Para exit()

int main() {
    pid_t pid;
    pid = fork();  // Crear un proceso hijo

    if (pid < 0) {
        perror("Fork falló");
        return 1;
    }
    else if (pid == 0) {  // Proceso hijo
        // Variación con execl()
        // execl("/bin/ls", "ls", "-l", (char *)NULL);

        // Variación con execle()
        // char *env[] = {"PATH=/bin", NULL};
        // execle("/bin/ls", "ls", "-l", (char *)NULL, env);

        // Variación con execlp()
        // execlp("ls", "ls", "-l", (char *)NULL);

        // Variación con execv()
        // char *args[] = {"/bin/ls", "-l", NULL};
        // execv("/bin/ls", args);

        // Variación con execvp()
        // char *args[] = {"ls", "-l", NULL};
        // execvp("ls", args);

        // Variación con execvpe()
        // char *args[] = {"ls", "-l", NULL};
        // char *env[] = {"PATH=/bin", NULL};
        // execvpe("ls", args, env);

        // En caso de fallo, imprimir mensaje y salir
        perror("Exec falló");
        exit(1);
    }
    else {  // Proceso padre
        wait(NULL);  // Esperar al proceso hijo
        printf("Proceso hijo completado.\n");
    }

    return 0;
}
```

La familia exec() tiene varias variantes para cubrir diferentes casos de uso y hacer más flexible la sustitución del proceso:

1. Diferentes Formas de Pasar Argumentos:

    - Algunas variantes permiten pasar los argumentos como una lista (execl(), execlp()) y otras permiten pasarlos como un array/vector (execv(), execvp()). Esto permite al programador elegir la opción que sea más conveniente según cómo se preparen los argumentos en el programa.
    - Por ejemplo, si ya tienes los argumentos almacenados en un array, execv() o execvp() son más convenientes que execl().
2. Buscar en el PATH o Usar una Ruta Absoluta:

    - Las variantes con la letra p (execlp(), execvp(), execvpe()) buscan el ejecutable en los directorios especificados en la variable de entorno PATH. Esto es útil si no se tiene la ruta absoluta del programa y se quiere buscar automáticamente en el sistema.
    - Las otras variantes (execl(), execv()) requieren la ruta absoluta al ejecutable.
Especificar un Entorno:

3. Las variantes con la letra e (execle(), execvpe()) permiten especificar un entorno personalizado. Esto es útil cuando se necesita modificar las variables de entorno para el proceso que se va a ejecutar, en lugar de heredar el entorno del proceso padre.
Compatibilidad y Flexibilidad:

    - Estas variaciones también ayudan a mantener la compatibilidad con diferentes formas de escribir programas y el manejo de procesos. Dependiendo de la situación, el programador puede preferir una versión sobre otra para cumplir con las necesidades específicas del programa.
    - También es una cuestión de comodidad y simplicidad: algunas versiones son más simples de usar en ciertos contextos (como cuando los argumentos ya están en un array) y otras son más explícitas.

==========================================================

   
4. Escriba ahora un programa que use ```wait()``` para esperar que el proceso hijo finalice su ejecución. ¿Cuál es el valor de retorno de la función ```wait()```?, ¿Qué pasa si usted usa la función ```wait``` en el hijo?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // Para la llamada a wait()

int main() {
    pid_t pid;
    int status;

    pid = fork();  // Crear un proceso hijo

    if (pid < 0) {
        perror("Fork falló");
        return 1;
    }
    else if (pid == 0) {  // Proceso hijo
        printf("Proceso hijo en ejecución.\n");
        sleep(2);  // Simula que el proceso hijo tarda un poco en ejecutar algo.
        printf("Proceso hijo finalizado.\n");
    }
    else {  // Proceso padre
        wait(&status);  // El proceso padre espera que el proceso hijo termine
        printf("Proceso padre: El proceso hijo ha terminado.\n");
    }

    return 0;
}
```

- Si se usa wait() en el proceso hijo, no habrá ningún efecto porque el proceso hijo no tiene procesos hijos propios.

- wait() espera la terminación de un proceso hijo del proceso que la llama. Como el proceso hijo no ha creado ningún otro proceso, llamar a wait() en el hijo simplemente fallará y devolverá -1, indicando que no hay hijos que esperar.

- Además, generalmente cuando wait() falla, se establece la variable errno para proporcionar más información sobre el error, que en este caso sería que no hay procesos hijos que esperar (ECHILD).

En resumen, la función wait() está diseñada para ser usada por procesos que tienen procesos hijos, y su propósito es sincronizar la ejecución para que el padre espere a que termine el hijo. En el proceso hijo, wait() no tiene ningún hijo que esperar y fallará.

=========================================

5. Haga un programa como el del ejercicio anterior con una breve modificación la cual consiste en usar ```waitpid()``` en lugar de ```wait()```, ¿cuándo podría ser ```waitpid()``` útil?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> // Para la llamada a waitpid()

int main() {
    pid_t pid;
    int status;

    pid = fork();  // Crear un proceso hijo

    if (pid < 0) {
        perror("Fork falló");
        return 1;
    }
    else if (pid == 0) {  // Proceso hijo
        printf("Proceso hijo en ejecución.\n");
        sleep(2);  // Simula que el proceso hijo tarda un poco en ejecutar algo.
        printf("Proceso hijo finalizado.\n");
    }
    else {  // Proceso padre
        // El proceso padre espera específicamente por el proceso hijo usando waitpid()
        waitpid(pid, &status, 0);
        printf("Proceso padre: El proceso hijo con PID %d ha terminado.\n", pid);
    }

    return 0;
}
```

Ejemplo de Caso de Uso de waitpid()
Supongamos que un proceso padre tiene tres hijos (hijo1, hijo2, hijo3). En un escenario donde cada hijo tiene una tarea específica que depende del orden de ejecución, podrías necesitar que el proceso padre espere primero la finalización de hijo2 antes de proceder a hacer algo con los datos generados por él, sin importar el estado de hijo1 y hijo3. En este caso, waitpid(pid_hijo2, &status, 0) garantiza que el padre espere solo por el hijo2.

Por cosiguiente, waitpid() es útil cuando se necesita mayor control sobre qué proceso hijo esperar, cuando se quiere operar en modo no bloqueante, o cuando se quiere evitar procesos zombie. Es una herramienta más versátil en comparación con wait(), y es especialmente importante en aplicaciones que manejan múltiples procesos hijos.

====================================

6. Escriba un programa que cree dos hijos y conecte la salida estándar de un hijo a la entrada estándar del otro usando la llamada a sistema ```pipe()```.

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];  // Array para almacenar los file descriptors del pipe
    pid_t pid1, pid2;

    // Crear el pipe
    if (pipe(pipefd) == -1) {
        perror("Pipe falló");
        exit(1);
    }

    // Crear el primer hijo
    pid1 = fork();
    if (pid1 < 0) {
        perror("Fork falló para el primer hijo");
        exit(1);
    }

    if (pid1 == 0) {  // Código del primer hijo
        // Cerrar la lectura del pipe (no la necesita)
        close(pipefd[0]);

        // Redirigir la salida estándar al pipe (escribir en el pipe)
        dup2(pipefd[1], STDOUT_FILENO);
        
        // Cerrar el extremo de escritura del pipe (ya se duplicó con dup2)
        close(pipefd[1]);

        // Ejecutar un comando que genere salida, como `ls`
        execlp("ls", "ls", NULL);
        perror("Exec falló para el primer hijo");
        exit(1);
    }

    // Crear el segundo hijo
    pid2 = fork();
    if (pid2 < 0) {
        perror("Fork falló para el segundo hijo");
        exit(1);
    }

    if (pid2 == 0) {  // Código del segundo hijo
        // Cerrar la escritura del pipe (no la necesita)
        close(pipefd[1]);

        // Redirigir la entrada estándar al pipe (leer del pipe)
        dup2(pipefd[0], STDIN_FILENO);
        
        // Cerrar el extremo de lectura del pipe (ya se duplicó con dup2)
        close(pipefd[0]);

        // Ejecutar un comando que use la entrada estándar, como `wc -l` (cuenta líneas)
        execlp("wc", "wc", "-l", NULL);
        perror("Exec falló para el segundo hijo");
        exit(1);
    }

    // Código del proceso padre
    // Cerrar ambos extremos del pipe en el padre (no los necesita)
    close(pipefd[0]);
    close(pipefd[1]);

    // Esperar a que ambos hijos terminen
    wait(NULL);
    wait(NULL);

    return 0;
}
```

**¿Cuándo es Útil pipe()?**
- pipe() es extremadamente útil para comunicar procesos entre sí, permitiendo que la salida de un proceso sea la entrada de otro.
- Este tipo de programación se utiliza a menudo en shell scripting y en sistemas Unix para crear pipelines, permitiendo que diferentes herramientas y comandos trabajen juntos de manera eficiente.

===================================

7. Escriba un programa en C llamado **time.c** que determine la cantidad de tiempo necesaria para correr un comando desde la línea de comandos. Este programa será ejecutado como "```time <command>```" y mostrará la cantidad de tiempo gastada para ejecutar el comando especificado. Para resolver el problema haga uso de ```fork()``` y ```exec()```, así como de la función ```gettimeofday()``` para determinar el tiempo transcurrido. 
   
   La estrategia general es hacer un fork para crear un proceso hijo el cual ejecutara el comando especificado. Sin embargo, antes de que el proceso hijo ejecute el comando espeficado, debera almacenar el tiempo actual (**starting time**). El padre invocará el wait para esperar por la culminación del proceso hijo. Luego, una vez que el proceso hijo culmine, el padre almacenara el tiempo actual en este punto (**ending time**). La diferencia entre los tiempos **inicial** y **final** (**starting** y **endind**) representará el tiempo gastado para ejecutar el comando. Por ejemplo la salida en pantalla de abajo muestra la cantidad de tiempo para correr el comando ```ls```:

```
./time ls
time.c
time

Elapsed time: 0.25422
```
```C
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>  // Para gettimeofday()

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
        exit(1);
    }

    struct timeval start, end;

    // Obtener el tiempo actual antes de ejecutar el comando (start time)
    if (gettimeofday(&start, NULL) < 0) {
        perror("Error obteniendo el tiempo inicial");
        exit(1);
    }

    pid_t pid = fork();  // Crear proceso hijo

    if (pid < 0) {
        perror("Fork falló");
        exit(1);
    }
    else if (pid == 0) {  // Código del proceso hijo
        // Ejecutar el comando especificado usando execvp()
        execvp(argv[1], &argv[1]);
        // Si exec falla
        perror("Exec falló");
        exit(1);
    }
    else {  // Código del proceso padre
        // Esperar a que el proceso hijo termine
        wait(NULL);

        // Obtener el tiempo actual después de que el hijo termina (end time)
        if (gettimeofday(&end, NULL) < 0) {
            perror("Error obteniendo el tiempo final");
            exit(1);
        }

        // Calcular el tiempo transcurrido en segundos y microsegundos
        double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

        // Mostrar el tiempo transcurrido
        printf("Elapsed time: %f seconds\n", elapsed_time);
    }

    return 0;
}
```

**Explicación de ```execvp()```**
- ```execvp(argv[1], &argv[1])```:
    - ```argv[1]``` es el comando que se va a ejecutar (por ejemplo, ls).
    - ```&argv[1]``` es el array de argumentos que se pasa a execvp(). Este array incluye el comando y sus posibles argumentos adicionales, y siempre debe terminar con NULL.
**Uso de gettimeofday()**
- ```gettimeofday()```:
    - Esta función se utiliza para obtener la hora actual en segundos y microsegundos.
    - El tiempo inicial (start) se toma antes de crear el proceso hijo, y el tiempo final (end) se toma después de que el proceso hijo haya terminado.
**¿Por Qué Usar ```wait()```?**
- ```wait()```:
    - Es importante usar wait() en el padre para sincronizar la finalización del proceso hijo antes de calcular el tiempo total.
    - Esto asegura que el tiempo que se mide sea el tiempo real de ejecución del proceso hijo, y no el tiempo total del proceso padre más el hijo.

