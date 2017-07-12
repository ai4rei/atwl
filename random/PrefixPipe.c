// -----------------------------------------------------------------
// Prefix (pipe)
// Prefixes every line from input with a defined prefix to output.
//
// -----------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    char buffer[256];

    if(argc<2)
    {
        fprintf(stderr, "Usage: other-app | prefix <prefix string>\n");
        return EXIT_FAILURE;
    }

    while(fgets(buffer, sizeof(buffer), stdin))
    {
        if(fputs(argv[1], stdout)==EOF || fputs(buffer, stdout)==EOF)
        {
            fprintf(stderr, "Error: Broken output.\n");

            return EXIT_FAILURE;
        }
    }

    if(!feof(stdin))
    {
        fprintf(stderr, "Error: Broken input.\n");

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
