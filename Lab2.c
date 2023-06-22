#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

// Función para enviar una respuesta HTTP al cliente
void send_response(int client_socket, const char *status, const char *content_type, const char *content) {
    char response[BUFFER_SIZE];
    sprintf(response, "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n%s",
            status, content_type, strlen(content), content);
    write(client_socket, response, strlen(response));
}


// Función para manejar la solicitud del cliente
void handle_request(int client_socket, const char *document_root) {
    char request[BUFFER_SIZE];
    read(client_socket, request, sizeof(request));

    // Obtener la ruta solicitada por el cliente
    char *token = strtok(request, " ");
    token = strtok(NULL, " ");
    char *path = token;

    /*// Comprobar si la ruta termina en '/'
    int path_length = strlen(path);
    if (path_length > 1 && path[path_length - 1] != '/') {
        // Redirigir al cliente a la ruta con '/'
        char redirect_url[BUFFER_SIZE];
        sprintf(redirect_url, "http://localhost:%d%s/", DEFAULT_PORT, path);
        send_response(client_socket, "301 Moved Permanently", "text/plain", redirect_url);
        return;
    }*/


    // Comprobar si la ruta es '/'
    if (strcmp(path, "/") == 0) {
        // El cliente consulta por el archivo /index.html
        char file_path[BUFFER_SIZE];
        sprintf(file_path, "%sindex.html", document_root);
        if (access(file_path, F_OK) == 0) {
            // El archivo index.html existe, enviar su contenido al cliente
            FILE *file = fopen(file_path, "rb");
            char file_content[BUFFER_SIZE];
            fread(file_content, sizeof(char), sizeof(file_content), file);
            fclose(file);
            send_response(client_socket, "200 OK", "text/html", file_content);
            return;
        }
    }
        // Comprobar si la ruta contiene '/index.html'
    if (strstr(path, "/index.html") != NULL) {
        // El cliente solicita directamente el archivo /index.html
        char file_path[BUFFER_SIZE];
        sprintf(file_path, "%s%s", document_root, path);
        if (access(file_path, F_OK) == 0) {
            // El archivo index.html existe, enviar su contenido al cliente
            FILE *file = fopen(file_path, "rb");
            char file_content[BUFFER_SIZE];
            fread(file_content, sizeof(char), sizeof(file_content), file);
            fclose(file);
            send_response(client_socket, "200 OK", "text/html", file_content);
            return;
        }
    }
    // Comprobar si la extensión del archivo es PNG
if (strstr(path, ".png") != NULL) {
    char file_path[BUFFER_SIZE];
    sprintf(file_path, "%s%s", document_root, path);
    if (access(file_path, F_OK) == 0) {
        // El archivo PNG existe, enviar su contenido al cliente
        int file_descriptor = open(file_path, O_RDONLY);
        struct stat file_info;
        fstat(file_descriptor, &file_info);
        char file_content[file_info.st_size];
        read(file_descriptor, file_content, sizeof(file_content));
        close(file_descriptor);
        send_response(client_socket, "200 OK", "image/png", file_content);
        return;
    }
}

// Comprobar si la extensión del archivo es JPEG
if (strstr(path, ".jpeg") != NULL || strstr(path, ".jpg") != NULL) {
    char file_path[BUFFER_SIZE];
    sprintf(file_path, "%s%s", document_root, path);
    if (access(file_path, F_OK) == 0) {
        // El archivo JPEG existe, enviar su contenido al cliente
        int file_descriptor = open(file_path, O_RDONLY);
        struct stat file_info;
        fstat(file_descriptor, &file_info);
        char file_content[file_info.st_size];
        read(file_descriptor, file_content, sizeof(file_content));
        close(file_descriptor);
        send_response(client_socket, "200 OK", "image/jpeg", file_content);
        return;
    }
}

    // Comprobar si la ruta termina en '/'
    char file_path[BUFFER_SIZE];
    sprintf(file_path, "%s%sindex.html", document_root, path);
    if (access(file_path, F_OK) == 0) {
        // El archivo index.html existe, enviar su contenido al cliente
        FILE *file = fopen(file_path, "rb");
        char file_content[BUFFER_SIZE];
        fread(file_content, sizeof(char), sizeof(file_content), file);
        fclose(file);
        send_response(client_socket, "200 OK", "text/html", file_content);
        return;
    }
    

    // La ruta no coincide con ninguna de las opciones anteriores, enviar error 404
    send_response(client_socket, "404 Not Found", "text/plain", "File not found");
}

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;

    int port;
    if (argc > 1) {
        port = atoi(argv[1]);
    } else {
        port = DEFAULT_PORT;
    }

    // Crear el socket del servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        exit(1);
    }

    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Vincular el socket a la dirección del servidor
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        exit(1);
    }

    // Escuchar conexiones entrantes
    if (listen(server_socket, 10) == -1) {
        perror("Error listening for connections");
        exit(1);
    }

    printf("Server running on port %d\n", port);

    while (1) {
        // Aceptar una conexión entrante
        client_addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_socket == -1) {
            perror("Error accepting connection");
            exit(1);
        }

        // Manejar la solicitud del cliente
        handle_request(client_socket, "Files");

        // Cerrar el socket del cliente
        close(client_socket);
    }

    // Cerrar el socket del servidor
    close(server_socket);

    return 0;
}
