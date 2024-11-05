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




6. Dado el siguiente programa, ¿cuál sería la salida desplegada en las líneas comentadas como ```LINE X``` y ```LINE Y```?

```C
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#define SIZE 5
int nums[SIZE] = {0,1,2,3,4};

int main() {
    int i;
    pid t pid;
    pid = fork();
    if (pid == 0) {
        for (i = 0; i < SIZE; i++) {
            nums[i] *= -i;
            printf("CHILD: %d ",nums[i]); /* LINE X */
        }
    }
    else if (pid > 0) {
        wait(NULL);
        for (i = 0; i < SIZE; i++)
            printf("PARENT: %d ",nums[i]); /* LINE Y */
    }
    return 0;
}
```

## Problemas de programación ##

1. Escriba un programa que abra un archivo (con la llamada ```open()```) y entonces llame a ```fork()```. Nota: El siguiente [enlace](https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/) puede ser de utilidad para entender la llamada open().
   * ¿Pueden el padre y el hijo acceder al file descriptor retornado por ```open()```?
   * ¿Qué pasa si ellos empiezan a escribir el archivo de manera concurrente, es decir, a la misma vez?

2. Escriba un programa usando fork(). El proceso hijo imprimirá ```"Hello"```; el proceso padre imprimirá ```"goodbye"```. Usted deberá asegurar que el proceso hijo imprima siempre en primer lugar.

3. Escriba un programa que llame ```fork()``` y entonces llame alguna forma de exec() para correr el programa ```/bin/ls```. Intente probar todas las variaciones de la familia de funciones ```exec()``` incluyendo (en linux) ```execl()```, ```execle()```, ```execlp()```, ```execv()```, ```execvp()``` y ```execvpe()```. ¿Por qué piensa usted que existen tantas variaciones para la misma llamada básica?
   
4. Escriba ahora un programa que use ```wait()``` para esperar que el proceso hijo finalice su ejecución. ¿Cuál es el valor de retorno de la función ```wait()```?, ¿Qué pasa si usted usa la función ```wait``` en el hijo?

5. Haga un programa como el del ejercicio anterior con una breve modificación la cual consiste en usar ```waitpid()``` en lugar de ```wait()```, ¿cuándo podría ser ```waitpid()``` útil?

6. Escriba un programa que cree dos hijos y conecte la salida estándar de un hijo a la entrada estándar del otro usando la llamada a sistema ```pipe()```.

7. Escriba un programa en C llamado **time.c** que determine la cantidad de tiempo necesaria para correr un comando desde la línea de comandos. Este programa será ejecutado como "```time <command>```" y mostrará la cantidad de tiempo gastada para ejecutar el comando especificado. Para resolver el problema haga uso de ```fork()``` y ```exec()```, así como de la función ```gettimeofday()``` para determinar el tiempo transcurrido. 
   
   La estrategia general es hacer un fork para crear un proceso hijo el cual ejecutara el comando especificado. Sin embargo, antes de que el proceso hijo ejecute el comando espeficado, debera almacenar el tiempo actual (**starting time**). El padre invocará el wait para esperar por la culminación del proceso hijo. Luego, una vez que el proceso hijo culmine, el padre almacenara el tiempo actual en este punto (**ending time**). La diferencia entre los tiempos **inicial** y **final** (**starting** y **endind**) representará el tiempo gastado para ejecutar el comando. Por ejemplo la salida en pantalla de abajo muestra la cantidad de tiempo para correr el comando ```ls```:

```
./time ls
time.c
time

Elapsed time: 0.25422
```

