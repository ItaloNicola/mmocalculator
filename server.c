#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include "unicode.h"

// TODO: handle multiple consecutive operators, like "---"
static void solve(char *expression, char *solution) {
    char text[256], *problem, operator[8];
    double first_operand, second_operand, result;

    strcpy(text, expression);

    // solve problem // TODO: handle parenthesis
    {
        // operator precedence table
        char *precedence[4][4] = {
            {"(", ")", "€", "€"},
            {"√", "^", "€", "€"},
            {"*", "÷", "/", "%"},
            {"+", "-", "€", "€"},
        };

        int precedence_tier = 0, operator_found = 0;

        while (precedence_tier < 4) {
            char first_part[256];
            char last_char_reached = 0;

            problem = text;

            while (!last_char_reached) {

                // save first part
                strncpy(first_part, text, problem - text);
                first_part[problem - text] = '\0';

                // get first operand
                result = strtof(problem, &problem);

                // save pointer to operator
                char *operator_ptr = problem;

                // get operator
                {
                    ssize_t start = 0, end = 1;
                    utf8slice(problem, &start, &end);

                    // handle trailing operators
                    if (end == -1) break;

                    strncpy(operator, problem, end);
                    operator[end] = '\0';
                    problem = problem + end;
                }

                for (int i = 0; i < 4; i++) {
                    if (!strcmp(precedence[precedence_tier][i], operator)) operator_found = 1;
                }

                // if operator precedence is not enough, search for the next one
                if (!operator_found) continue;

                // if operator is right-only, update first part
                if (!strcmp("√", operator)) {
                    strncpy(first_part, text, operator_ptr - text);
                    first_part[operator_ptr - text] = '\0';
                }

                // get second operand
                second_operand = strtof(problem, &problem);

                // last operand reached, set flag to end calculation
                if (problem == text + strlen(text)) {
                    last_char_reached = 1;
                }

                // solve operation
                {
                    // plus
                    if (!strcmp(operator, "+")) {
                        result += second_operand;
                    }

                    // minus
                    if (!strcmp(operator, "-")) {
                        result -= second_operand;
                    }

                    // times
                    if (!strcmp(operator, "*")) {
                        result *= second_operand;
                    }

                    // division
                    if (!strcmp(operator, "÷")) {
                        result /= second_operand;
                    }

                    // power
                    if (!strcmp(operator, "^")) {
                        result = pow(result, second_operand);
                    }

                    // square root
                    if (!strcmp(operator, "√")) {
                        result = sqrt(second_operand);
                    }

                    // module
                    if (!strcmp(operator, "%")) {
                        result = (int) result % (int) second_operand;
                    }
                }

                // dirty hack to hide float precision problems
                if (fabs(result - round(result)) < 0.000001) {
                    result = round(result);
                }

                // save second part
                char second_part[256];
                strcpy(second_part, problem);

                // update problem
                sprintf(text, "%s%f%s", first_part, result, second_part);

                // get position to continue calculating from
                char str_result[256];
                sprintf(str_result, "%f", result);
                problem = strstr(text, str_result);

                // reset operator found flag
                operator_found = 0;
            }

            // try next precedence tier
            precedence_tier++;
        }
    }

    // format output
    sprintf(solution, "%g", result);
}

void calculate_new_expression(char *new_expression, char *current_expression,  char *new_char) {

    if (!strcmp(new_char, "=")) {
        solve(current_expression, new_expression);
        strcpy(current_expression, new_expression);
        return;
    }

    if (!strcmp(new_char, "C")) {
        current_expression[0] = '\0';
        new_expression[0] = '\0';
        return;
    }

    if (!strcmp(new_char, "←")) {
        int last_char_index = utf8len(current_expression) - 1;
        current_expression[last_char_index] = '\0';
        strcpy(new_expression, current_expression);
        return;
    }

    strcat(current_expression, new_char);
    strcpy(new_expression, current_expression);
}

int main(int argc, char **argv) {

    // ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    // handle bad usage
    if (argc < 2) {
        fprintf(stderr, "ERROR: No port provided!\n");
        exit(1);
    }

    // load port number from input
    int port_number = strtol(argv[1], NULL, 10);

    // create TCP socket
    int socket_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
    if (socket_fd < 0) {
        fprintf(stderr, "ERROR: Could not open socket.\n");
        exit(1);
    }

    // server address initialization
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number);

    // bind socket file descriptor to socket
    if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "ERROR: Could not bind file descriptor to socket.\n");
        exit(1);
    }

    // listen to socket input
    listen(socket_fd, 5);

    // dirty hack so I don't have to use linked-lists
    // TODO: stop using this, it is dumb and slow
    char sockets[500];
    memset(sockets, 0, 500);

    // declare new socket file descriptor number and delimiter
    int new_socket_fd, last_fd = 0;

    // initialize current expression as nothing
    char current_expression[256];
    memset(current_expression, 0, 256);

    // main loop
    for (;;) {

        // avoid burning CPU
        usleep(10000);

        // accept incoming connection (NON-BLOCKING)
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        new_socket_fd = accept(socket_fd, (struct sockaddr *) &client_address, &client_address_len);
        if (new_socket_fd < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "ERROR: Could not accept incoming connection.\n");
        }

        // if received new connection
        if (new_socket_fd > 0) {

            // set file descriptor to non-blocking mode
            fcntl(new_socket_fd, F_SETFL, O_NONBLOCK);

            // update socket counter
            sockets[last_fd++] = new_socket_fd;
            fprintf(stdout, "Connection received.\n");

            // tell client what the current expression is
            int n = write(new_socket_fd, current_expression, 255);
            if (n < 0) {
                fprintf(stderr, "ERROR writing to socket.\n");
            }
        }

        // loop through all sockets
        char buffer[256];
        for (int i = 0; i < last_fd; i++) {
            if (sockets[i] == 0) continue;

            // read next information on socket if any (NON-BLOCKING)
            memset(buffer, 0, 256);
            int n = read(sockets[i], buffer, 255);
            if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                printf("ERROR reading from socket %d\n", i);
            }
            // TODO: this is just bad error handling, think of something else
            else if (n == 0 || n == -1) {
                continue;
            }

            // if a new message was received
            // kinda log received message
            // TODO: implement proper logging
            printf("Client %d: %s\n", i, buffer);

            // calculate new expression
            char new_expression[256];
            memset(new_expression, 0, 256);
            calculate_new_expression(new_expression, current_expression, buffer);
            fprintf(stdout, "%s\n", new_expression);

            // update all users
            for (int j = 0; j < last_fd; j++) {
                if (sockets[j] == 0) continue;

                int n = write(sockets[j], new_expression, 255);
                if (n < 0) {
                    fprintf(stderr, "ERROR writing to socket %d.\n", j);
                    sockets[j] = 0;
                }
            }
        }
    }

    return 0;
}
