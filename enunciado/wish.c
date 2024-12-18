#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/stat.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_INPUT 1024 // Tamaño máximo de entrada para una línea de comando
#define MAX_ARGS 64    // Máximo número de argumentos en un comando
#define MAX_PATH 10    // Máximo número de rutas en el path de búsqueda
#define MAX_TEMP_FILES 128

// Mensaje de error estándar para todos los tipos de error en el shell
char error_message[30] = "An error has occurred\n";

// Arreglo de cadenas para almacenar el path de búsqueda
char *path[MAX_PATH];
int path_count = 0; // Contador de directorios en el path de búsqueda
char temp_file_list[MAX_TEMP_FILES][256];
int temp_file_count = 0;

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

// Agrega un archivo temporal a la lista global
void add_temp_file(const char *filename)
{
    if (temp_file_count < MAX_TEMP_FILES)
    {
        strncpy(temp_file_list[temp_file_count], filename, sizeof(temp_file_list[temp_file_count]) - 1);
        temp_file_list[temp_file_count][sizeof(temp_file_list[temp_file_count]) - 1] = '\0';
        temp_file_count++;
    }
}

// Elimina todos los archivos temporales al final del programa
void cleanup_temp_files()
{
    for (int i = 0; i < temp_file_count; i++)
    {
        unlink(temp_file_list[i]); // Elimina el archivo
    }
    temp_file_count = 0; // Reinicia el contador
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
// Función para manejar redirección
int handle_redirection(char **args, int *num_args)
{
    int redirect_index = -1;

    // Buscar el operador '>'
    for (int i = 0; i < *num_args; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            if (i == 0 || redirect_index != -1)
            {
                // Ya hay un operador '>', error por múltiples operadores
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }
            redirect_index = i;
        }
    }

    // Si no hay redirección, retornar 0
    if (redirect_index == -1)
    {
        return 0;
    }

    // Validar que hay exactamente un archivo después del operador '>'
    if (redirect_index + 1 >= *num_args || redirect_index + 2 < *num_args)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return -1;
    }

    // Redirección válida: ajustar los argumentos y redirigir stdout
    char *output_file = args[redirect_index + 1];
    args[redirect_index] = NULL; // Finaliza los argumentos en el operador '>'
    *num_args = redirect_index;  // Ajusta el número de argumentos

    // Redirigir stdout al archivo especificado
    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd == -1)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return -1;
    }

    close(fd);
    return 1; // Indica que la redirección fue exitosa
}

// Manejo de comandos paralelos con redirección de salida única
void execute_parallel_commands(char *input)
{
    char *subcommands[MAX_ARGS];
    int num_subcommands = 0;

    char *token = strtok(input, "&");
    while (token != NULL && num_subcommands < MAX_ARGS)
    {
        if (strlen(token) > 0 && strspn(token, " \t\n") != strlen(token))
        {
            subcommands[num_subcommands++] = token;
        }
        token = strtok(NULL, "&");
    }

    pid_t pids[MAX_ARGS];
    char temp_files[MAX_ARGS][256];

    for (int i = 0; i < num_subcommands; i++)
    {
        char *args[MAX_ARGS];
        int num_args = parse_input(subcommands[i], args);

        if (num_args == 0)
            continue;

        snprintf(temp_files[i], sizeof(temp_files[i]), "/tmp/output20%d", i + 1);
        int fd = open(temp_files[i], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        if (fd == -1)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        // Registrar el archivo temporal
        add_temp_file(temp_files[i]);

        pid_t pid = fork();
        if (pid == 0)
        {
            // Proceso hijo

            // Ejecuta el subcomando
            if (strcmp(args[0], "cd") == 0)
            {
                run_cd(args, num_args);
            }
            else if (strcmp(args[0], "path") == 0)
            {
                set_path(args, num_args);
            }
            else if (strcmp(args[0], "exit") == 0)
            {
                run_exit(num_args);
            }
            else
            {
                run_external_command(args);
            }

            exit(0); // Asegúrate de que el proceso hijo termine después de ejecutar el comando
        }
        else if (pid > 0)
        {
            // Proceso padre
            pids[i] = pid;
            close(fd); // Importante: Cierra el descriptor de archivo en el padre
        }
        else
        {
            // Error al hacer fork
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < num_subcommands; i++)
    {
        if (pids[i] > 0)
        {
            waitpid(pids[i], NULL, 0);
        }
    }

    // Leer y combinar la salida de los archivos temporales SOLO UNA VEZ
    for (int i = 0; i < num_subcommands; i++)
    {
        int fd = open(temp_files[i], O_RDONLY);
        if (fd != -1)
        {
            char buffer[1024];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
            {
                // Escribe en stdout desde los archivos temporales (solo una vez)
                write(STDOUT_FILENO, buffer, bytes_read);
            }
            close(fd);
        }
        else
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

// Loop principal del shell: muestra el prompt, lee entrada, y ejecuta comandos
void shell_loop(FILE *input_stream)
{
    char input[MAX_INPUT]; // Buffer para la línea de entrada
    char *args[MAX_ARGS];  // Arreglo para almacenar los argumentoss
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

        // Manejo de comandos paralelos
        if (strchr(input, '&') != NULL)
        {
            execute_parallel_commands(input);
            continue;
        }

        // Divide la línea en argumentos y cuenta los argumentos
        num_args = parse_input(input, args);
        if (num_args == 0)
            continue; // Si la entrada está vacía, salta al siguiente ciclo

        // Verifica si el primer argumento es '&' (error)
        if (num_args == 1 && strcmp(args[0], "&") == 0)
        {
            continue;
        }

        // Manejo de redirección
        int redirection_result = handle_redirection(args, &num_args);
        if (redirection_result == -1)
        {
            continue; // Si hay un error en la redirección, pasa al siguiente comando
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
            pid_t pid = fork();
            if (pid == 0)
            {
                run_external_command(args);
                exit(1);
            }
            else if (pid > 0)
            {
                wait(NULL);
            }
            else
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }

        // Restaura el stdout original después de la redirección
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

    // Limpia los archivos temporales
    cleanup_temp_files();

    free_path();
    return 0;
}