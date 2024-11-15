#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT 1024 // Tamaño máximo de entrada para una línea de comando
#define MAX_ARGS 64    // Máximo número de argumentos en un comando
#define MAX_PATH 10    // Máximo número de rutas en el path de búsqueda

// Mensaje de error estándar para todos los tipos de error en el shell
char error_message[30] = "An error has occurred\n";

// Arreglo de cadenas para almacenar el path de búsqueda
char *path[MAX_PATH];
int path_count = 0; // Contador de directorios en el path de búsqueda

// Función para inicializar el path de búsqueda predeterminado
void initialize_path()
{
    path[0] = strdup("./");        // Directorio actual
    path[1] = strdup("/usr/bin/"); // /usr/bin
    path[2] = strdup("/bin/");     // /bin
    path_count = 3;                // Número de directorios en el path inicial
}

// Libera la memoria reservada para el path de búsqueda
void free_path()
{
    for (int i = 0; i < path_count; i++)
    {
        free(path[i]);
        path[i] = NULL;
    }
    path_count = 0;
}

// Comando integrado "exit": finaliza el shell si no tiene argumentos adicionales
void run_exit(int num_args)
{
    if (num_args != 1) // Si hay argumentos adicionales, muestra un error
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return; // Retorna sin cerrar el shell
    }
    exit(0);
}

// Comando integrado "cd": cambia el directorio actual
void run_cd(char **args, int num_args)
{
    if (num_args != 2) // Verifica que solo haya un argumento adicional (el directorio)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    if (chdir(args[1]) != 0) // Intenta cambiar al directorio especificado
    {
        write(STDERR_FILENO, error_message, strlen(error_message)); // Error si chdir falla
    }
}

// Comando integrado "path": establece el path de búsqueda del shell
void set_path(char **args, int num_args)
{
    // Primero, libera la configuración anterior del path
    free_path();

    // Luego, establece el nuevo path con los argumentos proporcionados
    for (int i = 1; i < num_args; i++)
    {
        if (path_count < MAX_PATH)
        {
            path[path_count] = strdup(args[i]);
            path_count++;
        }
    }

    // Si no se especificaron nuevos directorios, path_count se queda en 0,
    // lo que significa que el shell no debería ejecutar comandos externos.
}

// Ejecuta un comando externo en un proceso hijo
int run_external_command(char **args)
{
    char command_path[256];

    // Verificación inicial de que el path no está vacío
    if (path_count == 0)
    {
        // Imprime el mensaje de error estándar y sale sin intentar ejecutar el comando
        write(STDERR_FILENO, error_message, strlen(error_message));
        return -1;
    }

    for (int i = 0; i < path_count; i++)
    {
        // Construye el path completo del comando usando el directorio actual del path
        snprintf(command_path, sizeof(command_path), "%s/%s", path[i], args[0]);

        // Verifica si el comando es ejecutable en este path
        if (access(command_path, X_OK) == 0)
        {
            execv(command_path, args); // Ejecuta el comando si es accesible
            perror("execv");           // Si execv falla, muestra el error del sistema
            return -1;
        }
    }

    // Si no se encuentra el comando en ningún directorio del path
    write(STDERR_FILENO, error_message, strlen(error_message));
    return -1;
}

// Parsea la entrada en un arreglo de argumentos (tokens)
int parse_input(char *input, char **args)
{
    int num_args = 0;
    char *token = strtok(input, " \t\n"); // Divide la entrada usando espacios y tabulaciones
    while (token != NULL && num_args < MAX_ARGS)
    {
        args[num_args++] = token;
        token = strtok(NULL, " \t\n"); // Continua el parsing de la línea
    }
    args[num_args] = NULL; // Finaliza el arreglo de argumentos con NULL
    return num_args;       // Retorna el número de argumentos
}

// Revisa si el formato de la redirección es correcto
int handle_redirection(char **args, int num_args)
{
    for (int i = 0; i < num_args; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            // Verifica si hay un archivo especificado después del operador '>'
            if (i == num_args - 1)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }
            // Redirección está correctamente especificada
            // Implementa la lógica de redirección aquí si es necesario
        }
    }
    return 0;
}

// Loop principal del shell: muestra el prompt, lee entrada, y ejecuta comandos
void shell_loop(FILE *input_stream)
{
    char input[MAX_INPUT]; // Buffer para la línea de entrada
    char *args[MAX_ARGS];  // Arreglo para almacenar los argumentos
    int num_args;          // Número de argumentos en la línea de entrada

    while (1)
    {
        // Muestra el prompt solo en modo interactivo
        if (input_stream == stdin)
        {
            printf("wish> ");
            fflush(stdout);
        }

        // Lee una línea de entrada (de stdin o de archivo batch)
        if (fgets(input, sizeof(input), input_stream) == NULL)
        {
            break; // EOF
        }

        // Divide la línea en argumentos y cuenta los argumentos
        num_args = parse_input(input, args);
        if (num_args == 0)
            continue; // Si la entrada está vacía, salta al siguiente ciclo

        // Verifica redirección incorrecta antes de ejecutar el comando
        if (handle_redirection(args, num_args) == -1)
        {
            continue; // Salta a la siguiente línea de entrada sin ejecutar el comando
        }

        // Verifica y ejecuta los comandos integrados
        if (strcmp(args[0], "exit") == 0)
        {
            run_exit(num_args);
        }

        else if (strcmp(args[0], "cd") == 0)
        {
            run_cd(args, num_args);
        }
        else if (strcmp(args[0], "path") == 0)
        {
            set_path(args, num_args);
        }
        else
        {
            // Para comandos externos, crea un proceso hijo con fork()
            pid_t pid = fork();
            if (pid == 0)
            {
                // En el proceso hijo: intenta ejecutar el comando externo
                run_external_command(args);
                exit(1); // Termina el hijo si el comando falla
            }
            else if (pid > 0)
            {
                // En el proceso padre: espera a que el hijo termine
                wait(NULL);
            }
            else
            {
                // Error en fork()
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
    }
}

// Función principal del programa: verifica los argumentos y entra en el modo correcto
int main(int argc, char *argv[])
{
    if (argc > 2) // Solo se permite 0 o 1 argumento (batch file opcional)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    initialize_path(); // Configura el path de búsqueda inicial

    if (argc == 1)
    {
        shell_loop(stdin); // Modo interactivo si no hay argumentos
    }
    else if (argc == 2)
    {
        // Modo batch: intenta abrir el archivo especificado
        FILE *batch_file = fopen(argv[1], "r");
        if (batch_file == NULL)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        shell_loop(batch_file); // Modo batch con archivo de entrada
        fclose(batch_file);
    }

    free_path(); // Libera la memoria del path de búsqueda
    return 0;
}