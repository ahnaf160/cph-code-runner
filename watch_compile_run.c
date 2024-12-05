#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR "\\"
#else
#include <sys/inotify.h>
#define PATH_SEPARATOR "/"
#endif

// ANSI color codes for a hacker green theme
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define BOLD "\033[1m"
#define ITALIC "\033[3m"
#define UNDERLINE "\033[4m"
#define CLEAR_SCREEN "\033[2J\033[H"

// Fancy Banner
void print_banner()
{
    printf(CLEAR_SCREEN);
    printf(GREEN BOLD);
    printf("***********************************************\n");
    printf("*                                             *\n");
    printf("*   ULTRA-FAST COMPILER & RUNNER TOOL         *\n");
    printf("*           " ITALIC "Built by Ahnaf106" RESET GREEN BOLD "               *\n");
    printf("*                                             *\n");
    printf("***********************************************\n" RESET);
}

// Compile and run the program
void compile_and_run(const char *directory, const char *file_name)
{
    char input_path[256], output_path[256], exec_path[256], compile_cmd[512], run_cmd[512];

    // Construct paths for input.txt, output.txt, and the compiled executable
    snprintf(input_path, sizeof(input_path), "%s%sinput.txt", directory, PATH_SEPARATOR);
    snprintf(output_path, sizeof(output_path), "%s%soutput.txt", directory, PATH_SEPARATOR);
    snprintf(exec_path, sizeof(exec_path), "%s%sa.out", directory, PATH_SEPARATOR);

    // Determine compiler based on file extension
    const char *ext = strrchr(file_name, '.');
    const char *compiler = (ext && strcmp(ext, ".cpp") == 0) ? "g++" : "gcc";

    // Optimized Compilation flags
    const char *flags = "-std=c++17 -O3 -march=native -mtune=native -funroll-loops -DNDEBUG";

    // Compile the source file
    snprintf(compile_cmd, sizeof(compile_cmd), "%s %s %s -o %s", compiler, flags, file_name, exec_path);
    printf(GREEN BOLD "Compiling %s...\n" RESET, file_name);
    if (system(compile_cmd) != 0)
    {
        fprintf(stderr, GREEN BOLD "Compilation failed! Please check your code.\n" RESET);
        return;
    }

    // Run the executable with input redirection
    snprintf(run_cmd, sizeof(run_cmd), "%s < %s > %s", exec_path, input_path, output_path);
    printf(GREEN BOLD "Running the program with input from %s...\n" RESET, input_path);
    if (system(run_cmd) != 0)
    {
        fprintf(stderr, GREEN BOLD "Execution failed! Check your input file or code logic.\n" RESET);
    }
    else
    {
        printf(GREEN BOLD "Execution complete! Output saved to %s\n" RESET, output_path);
    }
}

// Check if the file was modified
#ifdef _WIN32
int file_modified(const char *file_name, time_t *last_modified_time)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) != 0)
    {
        perror("stat");
        return 0;
    }

    if (*last_modified_time != file_stat.st_mtime)
    {
        *last_modified_time = file_stat.st_mtime;
        return 1;
    }

    return 0;
}
#else
void watch_file(const char *file_name, const char *directory)
{
    int inotify_fd = inotify_init();
    if (inotify_fd < 0)
    {
        perror("inotify_init");
        return;
    }

    int wd = inotify_add_watch(inotify_fd, file_name, IN_MODIFY);
    if (wd < 0)
    {
        perror("inotify_add_watch");
        close(inotify_fd);
        return;
    }

    char buffer[1024];
    while (1)
    {
        int length = read(inotify_fd, buffer, sizeof(buffer));
        if (length < 0)
        {
            perror("read");
            break;
        }

        printf(GREEN BOLD "File modified! Re-compiling and running...\n" RESET);
        compile_and_run(directory, file_name);
    }

    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);
}
#endif

int main()
{
    char directory[256], file_name[256];
    time_t last_modified_time = 0;

    print_banner();

    printf(GREEN BOLD "Enter the directory containing input.txt and output.txt: " RESET);
    scanf("%255s", directory);

    printf(GREEN BOLD "Enter the source file name (.c or .cpp): " RESET);
    scanf("%255s", file_name);

#ifdef _WIN32
    printf(GREEN BOLD "Watching for file modifications...\n" RESET);
    while (1)
    {
        if (file_modified(file_name, &last_modified_time))
        {
            printf(GREEN BOLD "File modified! Re-compiling and running...\n" RESET);
            compile_and_run(directory, file_name);
        }
        Sleep(1000); // Check for modifications every second
    }
#else
    watch_file(file_name, directory);
#endif

    return 0;
}
