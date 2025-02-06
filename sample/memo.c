#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* memos[100];

int main() {
    printf(
        "\nplease enter command\n"
        "  create <memo id> <content>\n"
        "  read <memo id>\n"
        "  delete <memo id>\n"
    );
    while (1) {
        printf("> ");
        char command[10];
        int memo_id;

        int scan_result = scanf("%s %d", command, &memo_id);
        if (scan_result == EOF || scan_result < 2) {
            exit(0);
        } if (strcmp(command, "create") == 0) {
            char content[200];
            scanf("%s", content);
            // Allocate heap-memory of 100 bytes and fill in.
            memos[memo_id] = malloc(100);
            for (int i = 0; (memos[memo_id][i] = content[i]) != '\0'; i++);
            printf("created memo %d\n", memo_id);
        } else if (strcmp(command, "read") == 0) {
            printf("read memo %d: '%s'\n", memo_id, memos[memo_id]);
        } else if (strcmp(command, "delete") == 0) {
            // De-allocate heap-memory not to leak the resource.
            free(memos[memo_id]);
            printf("deleted memo %d\n", memo_id);
        } else {
            exit(1);
        }
    }
}
