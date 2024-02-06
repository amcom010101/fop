#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 2000
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000

#define debug(x) printf("%s", x);


void print_command(int argc, char * const argv[]);

int run_init(int argc, char * const argv[]);
int create_configs(char *username, char *email);

char * find_neogit (char * filepath);

int run_add(int argc, char * const argv[]);
int add_to_staging(char *filepath);
int compareFile(FILE * fPtr1, FILE * fPtr2);
int copy_file(FILE * fptr1, FILE * fptr2);
bool WildCmp (char * pattern, char * string);
bool is_stagged(char *filepath);
void add_n (char * cwd, int depth);
int get_last_stage_id (void);

int run_reset(int argc, char * const argv[]);
int remove_from_staging(char *filepath);

int run_commit(int argc, char * const argv[]);
int inc_last_commit_ID(void);
bool check_file_directory_exists(char *filepath);
int commit_staged_file(int commit_ID, char *filepath);
int track_file(char *filepath);
bool is_tracked(char *filepath);
int create_commit_file(int commit_ID, char *message, struct tm * ptr);
int find_file_last_commit(char* filepath);

int run_checkout(int argc, char *const argv[]);
int find_file_last_change_before_commit(char *filepath, int commit_ID);
int checkout_file(char *filepath, int commit_ID);

char neogit_dir[MAX_FILENAME_LENGTH];

void print_command(int argc, char * const argv[]) {
    for (int i = 0; i < argc; i++) {
        fprintf(stdout, "%s ", argv[i]);
    }
    fprintf(stdout, "\n");
    printf("%d\n", argc);
}

/*
 * Function to compare two files.
 * Returns 0 if both files are equivalent, otherwise returns -1
 */
int compareFile(FILE * fPtr1, FILE * fPtr2) {
    char ch1, ch2;

    do
    {
        // Input character from both files
        ch1 = fgetc(fPtr1);
        ch2 = fgetc(fPtr2);

        // If characters are not same then return -1
        if (ch1 != ch2)
            return -1;

    } while (ch1 != EOF && ch2 != EOF);


    /* If both files have reached end */
    if (ch1 == EOF && ch2 == EOF)
        return 0;
    else
        return -1;
    
}

int copy_file(FILE * fptr1, FILE * fptr2) {
    char filename[100], c;
  
    printf("Enter the filename to open for reading \n");
    scanf("%s", filename);
  
    // Open one file for reading
    fptr1 = fopen(filename, "r");
    if (fptr1 == NULL)
    {
        printf("Cannot open file %s \n", filename);
        exit(0);
    }
  
    printf("Enter the filename to open for writing \n");
    scanf("%s", filename);
  
    // Open another file for writing
    fptr2 = fopen(filename, "w");
    if (fptr2 == NULL)
    {
        printf("Cannot open file %s \n", filename);
        exit(0);
    }
  
    // Read contents from file
    c = fgetc(fptr1);
    while (c != EOF)
    {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }
  
    printf("\nContents copied to %s", filename);
  
    fclose(fptr1);
    fclose(fptr2);
    return 0;
}

int run_init(int argc, char * const argv[]) {
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;
    
    char tmp_cwd[1024];
    bool exists = false;
    struct dirent *entry;
    
    do {
        // find .neogit
        DIR *dir = opendir(".");

        if (dir == NULL) {
            perror("Error opening current directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".neogit") == 0)){
                exists = true;
            }
        }
        closedir(dir);

        // update current working directory
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return 1;

        // change cwd to parent
        if (strcmp(tmp_cwd, "/") != 0) {
            if (chdir("..") != 0) return 1;
        }
        

    } while (strcmp(tmp_cwd, "/") != 0);

    // return to the initial cwd
    if (chdir(cwd) != 0) return 1;
      
    if (!exists) {
       
        
        if (mkdir(".neogit", 0755) != 0) return 1;
        
        //initializing the .neogit dir
        // to do: give the global username and email as parameters
        return create_configs("", "");
    } else {
        perror("neogit repository has already been initialized");
    }
    return 0;
}

int create_configs(char *username, char *email) {
    printf("flag\n");
    FILE *file = fopen(".neogit/config", "w");
    if (file == NULL) return 1;

    fprintf(file, "username: %s\n", username);
    fprintf(file, "email: %s\n", email);
    fprintf(file, "last_commit_ID: %d\n", 0);
    fprintf(file, "current_commit_ID: %d\n", 0);
    fprintf(file, "branch: %s", "master");

    fclose(file);
    
    // create commits folder
    if (mkdir(".neogit/commits", 0755) != 0) return 1;

    // create committed files folder
    if (mkdir(".neogit/committedfiles", 0755) != 0) return 1;
    
    // create stagged files folder
    if (mkdir(".neogit/staggedfiles", 0755) != 0) return 1;
    
    // create shortcuts folder
    if (mkdir(".neogit/shortcuts", 0755) != 0) return 1;

    file = fopen(".neogit/staging", "w");
    fclose(file);

    file = fopen(".neogit/tracks", "w");
    fclose(file);
    
    file = fopen(".neogit/alias", "w");
    // to do: write the global alias part to it
    fclose(file);

    return 0;
}

char * find_neogit (char * filepath) {
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) return NULL;
    char tmp_cwd[1024];
    
    struct dirent *entry;
    do {
        // find .neogit
        DIR *dir = opendir(".");
        if (dir == NULL) {
            perror("Error opening current directory");
            return NULL;
        }
        // update current working directory
        if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) return NULL;

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".neogit") == 0){
                
                // we are in the dir where .neogit exists
                // compare the tmp_cwd and the cwd
                // the filepath is the difference btwn those two + the name of the file
                strcat(tmp_cwd, "/.neogit");
                return tmp_cwd;
            }
        }
        closedir(dir);

        

        // change cwd to parent
        if (strcmp(tmp_cwd, "/") != 0) {
            if (chdir("..") != 0) return NULL;
            
        }

    } while (strcmp(tmp_cwd, "/") != 0);
    
    // return to the initial cwd
    if (chdir(cwd) != 0) return 1;
    return NULL;
    
}

bool WildCmp (char * pattern, char * string) {
    
    if(*pattern == '\0' && *string == '\0') return true;
    if(*pattern == *string) return WildCmp(pattern+1, string+1);
    else if (*pattern == '*') return WildCmp(pattern+1, string) || WildCmp(pattern, string+1);
    
    return false;
}

void add_n (char * cwd, int depth) {
    
    struct dirent *entry;
    DIR *dir = opendir(".");
    
    for (int i = 0; i < depth; i++) {
        
        if (dir == NULL) {
            perror("Error opening current directory");
            return;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                char dirPath[1024];
                strcpy(dirPath, cwd);
                strcat(dirPath, entry->d_name);
                chdir(dirPath);
                add_n(dirPath, --depth);
            }
            
            else if (entry->d_type == DT_REG){
                printf("%s\n", entry->d_name);
                if(is_stagged(entry->d_name)){
                    printf("Stagged: YES\n");
                }
                else printf("Stagged: NO\n");
            }
            
        }
        closedir(dir);

    }
    
}

bool is_stagged(char *filepath) {
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    FILE *file = fopen(staging_file, "r");
    if (file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = (int)strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (strcmp(line+2, filepath) == 0) return true;

    }
    fclose(file);

    return false;
}

int run_add(int argc, char *const argv[]) {
    
    if (argc < 3) {
        perror("please specify a file");
        return 1;
    }
    
    char * filepath; //for handling non-root dir
    
    if (argc == 3) {
        char * pointer_to_wildcard = strchr(argv[2], '*');
        //handle command in non-root directories
        //if not in the dir that .neogit exists go there and change the filepath accordingly
        filepath = find_neogit(argv[2]);
        //handle wildcards
        if(!pointer_to_wildcard) //does not have wildcard
        {
            if(filepath) return add_to_staging(filepath);
            else return add_to_staging(argv[2]);
        }
        else{
            struct dirent *entry;
            DIR *dir = opendir(".");
            
            
            if (dir == NULL) {
                perror("Error opening current directory");
                return 1;
            }
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG){
                    
                    if(WildCmp(argv[2], entry->d_name)) return add_to_staging(entry->d_name);
                }
            }
            closedir(dir);
        }
    }
    else{
        if(strcmp(argv[2], "-f") == 0) //-f option
        {
            
            for (int i = 3; i < argc; i++) {
                
                //handle command in non-root directories
                //if not in the dir that .neogit exists go there and change the filepath accordingly
                filepath = find_neogit(argv[i]);
                if(filepath) add_to_staging(filepath);
                else add_to_staging(argv[i]);
                
            }
            
        }
        else if(strcmp(argv[2], "-n") == 0) //-n option
        {
            int depth;
            sscanf(argv[3], "%d", &depth);
            
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) == NULL) return 1;
            
            //handle command in non-root directories
            //if not in the dir that .neogit exists go there and change the filepath accordingly
            filepath = find_neogit(cwd);
            if(filepath) add_n(filepath, depth);
            else add_n(cwd, depth);
            
        }
        return 0;
    }
    return 1;
}

int add_to_staging(char *filepath) {
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    printf("%s\n", staging_file);
    FILE *file = fopen(staging_file, "r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = (int)strlen(line);

        // remove '\n' from the end so that strcmp returns a vaild value
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(filepath, line) == 0) {
            //we need to check if the content of the file has changed:
            char staggedfilepath[MAX_LINE_LENGTH];
            strcpy(staggedfilepath, neogit_dir);
            strcat(staggedfilepath, "/staggedfiles/");
            strcat(staggedfilepath, line);
            file = fopen(staggedfilepath, "r");
            FILE * file2 = fopen(line, "r");
            int resultOfComparison = compareFile(file, file2);
            //a) if not we return doing nothing
            if(resultOfComparison == 0) return 0;
            //b) else we hash the current content of the file and delete the last hash and add the new hash
            else {
                file = fopen(staggedfilepath, "w");
                file2 = fopen(line, "r");
                copy_file(file2, file);
                return 0;
            }
            
        }
    }
    
    file = fopen(staging_file,"a");
    if (file == NULL) {
        printf("no\n");
        return 1;

    }
    
    //adding the new filename to the staging file
    
    fprintf(file, "%d %s", get_last_stage_id()+1,filepath);
    //adding the hash of its content to the front of its name in the staging file
    char staggedfilepath[MAX_LINE_LENGTH];
    strcpy(staggedfilepath, neogit_dir);
    strcat(staggedfilepath, "/staggedfiles/");
    strcat(staggedfilepath, filepath);
    file = fopen(staggedfilepath, "w");
    
    fclose(file);

    return 0;
}

int get_last_stage_id(void) {
    
    //getting the last line of the stagging file
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    FILE * file = fopen(staging_file, "r");
    char line[MAX_LINE_LENGTH];
    //if file is empty the last id is 0
    if (fgets(line, sizeof(line), file) == NULL) return 0;
    //+1 is for the \0
    char buf[MAX_FILENAME_LENGTH+1];
    //now reading that many bytes from the end of the file
    fseek(file, -MAX_FILENAME_LENGTH, SEEK_END);
    ssize_t len = fread(buf, MAX_FILENAME_LENGTH, 1, file);
    
    buf[len] = '\0';
    
    /* and find the last newline character */
    char *last_newline = strrchr(buf, '\n');
    char *last_line = last_newline+1;
    
    return *last_line - '0';
}

int run_reset(int argc, char *const argv[]) {
    
    if (argc < 3) {
        perror("please specify a file");
        return 1;
    }
    
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    
    char tmpstaging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/tmp_staging");
    
    if (strcmp(argv[2], "-undo") == 0){ //-undo option
        int last_id = get_last_stage_id();
        
        FILE *file = fopen(staging_file, "r");
        if (file == NULL) return 1;
        
        FILE *tmp_file = fopen(tmpstaging_file, "w");
        if (tmp_file == NULL) return 1;

        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL) {
            
            if (line[0] - '0' != last_id) fputs(line, tmp_file);
            else{
                char staggedfilepath[MAX_LINE_LENGTH];
                strcpy(staggedfilepath, neogit_dir);
                strcat(staggedfilepath, "/staggedfiles/");
                strcat(staggedfilepath, line);
                remove(staggedfilepath);
            }
        }
        
        
        fclose(file);
        fclose(tmp_file);

        remove(staging_file);
        rename(tmpstaging_file, staging_file);
        return 0;
    }
    //else:
    //handle command in non-root directories
    //if not in the dir that .neogit exists go there and change the filepath accordingly
    char * filepath;
    filepath = find_neogit(argv[2]);
    if(filepath) return remove_from_staging(filepath);
    else return remove_from_staging(argv[2]);
}

int remove_from_staging(char *filepath) {
    
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    
    char tmpstaging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/tmp_staging");
    
    FILE *file = fopen(staging_file, "r");
    if (file == NULL) return 1;
    
    FILE *tmp_file = fopen(tmpstaging_file, "w");
    if (tmp_file == NULL) return 1;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = (int)strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(filepath, line) != 0) fputs(line, tmp_file);
    }
    
    
    char staggedfilepath[MAX_LINE_LENGTH];
    strcpy(staggedfilepath, neogit_dir);
    strcat(staggedfilepath, "/staggedfiles/");
    strcat(staggedfilepath, filepath);
    remove(staggedfilepath);
    
    
    fclose(file);
    fclose(tmp_file);

    remove(staging_file);
    rename(tmpstaging_file, staging_file);
    return 0;
}

int run_commit(int argc, char * const argv[]) {
    if (argc < 4) {
        perror("please use the correct format");
        return 1;
    }
    
    char message[MAX_MESSAGE_LENGTH];
    bool exists = false;
    
    if(strcmp(argv[2], "-s") == 0){ //shortcut
        
        char shortcut_dir[MAX_FILENAME_LENGTH];
        strcpy(shortcut_dir, neogit_dir);
        strcat(shortcut_dir, "/shortcuts");
        
        struct dirent *entry;
        DIR *dir = opendir(shortcut_dir);
        
        if (dir == NULL) {
            perror("Error opening shortcut directory");
            return 1;
        }
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG){
                
                if(strcmp(argv[3], entry->d_name) == 0) {
                    exists = true;
                    FILE * shortcut_file = fopen(argv[3], "r");
                    for (int i = 0; !feof(shortcut_file); i++) {
                        message[i] = fgetc(shortcut_file);
                    }
                    fclose(shortcut_file);
                }
            }
        }
        closedir(dir);
        
        if(!exists) {
            printf("Error: No such shortcut exists.\n");
            return 1;
        }
        
    }
    
    else{ //not shortcut
        int mess_length = (int)strlen(argv[3]);
        if(mess_length > 72) {
            printf("Error: Commit message exceeds the limit.\n");
            return 1;
        }
        if(strchr(argv[3], ' ')){
            if(argv[3][0] != '\"' || argv[3][mess_length-1] != '\"') {
                printf("Error: Commit message with space must be between double quotes.\n");
                return 1;
            }
        }
        strcpy(message, argv[3]);
    }
    
    int commit_ID = inc_last_commit_ID();
    if (commit_ID == -1) return 1;
    
    char staging_file[MAX_FILENAME_LENGTH];
    strcpy(staging_file, neogit_dir);
    strcat(staging_file, "/staging");
    FILE *file = fopen(staging_file, "r");
    FILE * file2;
    FILE * file3;
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    char commit_path[MAX_FILENAME_LENGTH];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = (int)strlen(line);
        
        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (!check_file_directory_exists(line)) {
            
            strcpy(commit_path, neogit_dir);
            strcat(commit_path, "/committedfiles/");
            strcat(commit_path, line);
            if (mkdir(commit_path, 0755) != 0) return 1;
        }
        
        track_file(line);
        
        char stage_path[MAX_FILENAME_LENGTH];
        strcpy(stage_path, neogit_dir);
        strcat(stage_path, "/staggedfiles/");
        strcat(stage_path, line);
        file2 = fopen(stage_path, "r");
        file3 = fopen(line, "r");
        int resultOfCOmparison = compareFile(file2, file3);
        if (resultOfCOmparison == 0) {
            printf("commit %s\n", line);
            commit_staged_file(commit_ID, line);
        }
        else{
            remove(stage_path);
            rmdir(commit_path);
        }
        fclose(file2);
        fclose(file3);
    }
    fclose(file);
    
    // remove stagged files
    file = fopen(staging_file, "w");
    if (file == NULL) return 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        char stagged_file_name[MAX_FILENAME_LENGTH];
        strcpy(stagged_file_name, neogit_dir);
        strcat(stagged_file_name, "/staggedfiles/");
        strcat(stagged_file_name, line);
        remove(stagged_file_name);
    }
    // free staging
    fclose(file);
    
    struct tm* ptr;
    time_t t;
    t = time(NULL);
    ptr = localtime(&t);
    
    create_commit_file(commit_ID, message, ptr);
    fprintf(stdout, "commit successfully with commit ID %d, commit Message: %s, at time: ", commit_ID, message);
    printf("%s\n", asctime(ptr));
    return 0;
}

// returns new commit_ID
int inc_last_commit_ID(void) {
    char config_file[MAX_FILENAME_LENGTH];
    strcpy(config_file, neogit_dir);
    strcat(config_file, "/config");
    FILE *file = fopen(config_file, "r");
    if (file == NULL) return -1;
    
    char tmpconfig_file[MAX_FILENAME_LENGTH];
    strcpy(tmpconfig_file, neogit_dir);
    strcat(tmpconfig_file, "/tmp_config");
    FILE *tmp_file = fopen(tmpconfig_file, "w");
    if (tmp_file == NULL) return -1;

    int last_commit_ID = 0;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);
            last_commit_ID++;
            fprintf(tmp_file, "last_commit_ID: %d\n", last_commit_ID);

        } else fprintf(tmp_file, "%s", line);
    }
    fclose(file);
    fclose(tmp_file);

    remove(config_file);
    rename(tmpconfig_file, config_file);
    return last_commit_ID;
}

bool check_file_directory_exists(char *filepath) {
    
    char commitedfiles[MAX_FILENAME_LENGTH];
    strcpy(commitedfiles, neogit_dir);
    strcat(commitedfiles, "/committedfiles");
    DIR *dir = opendir(commitedfiles);
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, filepath) == 0) return true;
    }
    closedir(dir);

    return false;
}

int commit_staged_file(int commit_ID, char* filepath) {
    FILE *read_file, *write_file;
    char read_path[MAX_FILENAME_LENGTH];
    strcpy(read_path, filepath);
    char write_path[MAX_FILENAME_LENGTH];
    strcpy(write_path, neogit_dir);
    strcat(write_path, "/committedfiles/");
    strcat(write_path, filepath);
    strcat(write_path, "/");
    char tmp[10];
    sprintf(tmp, "%d", commit_ID);
    strcat(write_path, tmp);

    read_file = fopen(read_path, "r");
    if (read_file == NULL) return 1;

    write_file = fopen(write_path, "w");
    if (write_file == NULL) return 1;

    char buffer;
    buffer = fgetc(read_file);
    while(buffer != EOF) {
        fputc(buffer, write_file);
        buffer = fgetc(read_file);
    }
    fclose(read_file);
    fclose(write_file);

    return 0;
}

int track_file(char *filepath) {
    if (is_tracked(filepath)) return 0;
    char track_file[MAX_FILENAME_LENGTH];
    strcpy(track_file, neogit_dir);
    strcat(track_file, "/tracks");
    FILE *file = fopen(track_file, "a");
    if (file == NULL) return 1;
    fprintf(file, "%s\n", filepath);
    return 0;
}

bool is_tracked(char *filepath) {
    
    char track_file[MAX_FILENAME_LENGTH];
    strcpy(track_file, neogit_dir);
    strcat(track_file, "/tracks");
    
    FILE *file = fopen(track_file, "r");
    if (file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = (int)strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (strcmp(line, filepath) == 0) return true;

    }
    fclose(file);

    return false;
}

int create_commit_file(int commit_ID, char *message, struct tm* ptr) {
    char commit_filepath[MAX_FILENAME_LENGTH];
    strcpy(commit_filepath, neogit_dir);
    strcat(commit_filepath, "/commits/");
    char tmp[10];
    sprintf(tmp, "%d", commit_ID);
    strcat(commit_filepath, tmp);

    FILE *file = fopen(commit_filepath, "w");
    if (file == NULL) return 1;
    
    fprintf(file, "%s\n", asctime(ptr));
    fprintf(file, "message: %s\n", message);
    
    //writing the name of the author
    char config[MAX_FILENAME_LENGTH];
    strcpy(config, neogit_dir);
    strcat(config, "/config");
    char line[MAX_LINE_LENGTH];
    char authorname[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        
        if (strncmp(line, "username: ", 10) == 0) {
            sscanf(line, "username: %s\n", authorname);
            break;
        }

    }
    fprintf(file, "%s\n", authorname);
    fprintf(file, "files:\n");
    
    DIR *dir = opendir(".");
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_tracked(entry->d_name)) {
            int file_last_commit_ID = find_file_last_commit(entry->d_name);
            fprintf(file, "%s %d\n", entry->d_name, file_last_commit_ID);
        }
    }
    closedir(dir);
    fclose(file);
    return 0;
}

int find_file_last_commit(char* filepath) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    
    strcpy(filepath_dir, neogit_dir);
    strcat(filepath_dir, "/committedfiles/");
    strcat(filepath_dir, filepath);

    int max = -1;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            max = max > tmp ? max: tmp;
        }
    }
    closedir(dir);

    return max;
}

int run_checkout(int argc, char * const argv[]) {
    if (argc < 3) return 1;
    
    int commit_ID;
    DIR *dir;
    struct dirent *entry;
    
    if (strcmp(argv[2], "HEAD") == 0){ //neogit checkout HEAD
        commit_ID = inc_last_commit_ID();
        
    }
    else{
        commit_ID = atoi(argv[2]); //neogit checkout <commit-id>
        
        if(commit_ID == 0) { //neogit checkout <branch-name>
            //find the last commit_id, the rest is the same
            char branch_dir[MAX_FILENAME_LENGTH];
            strcpy(branch_dir, neogit_dir);
            strcat(branch_dir, "/commits");
            strcat(branch_dir, argv[2]);
            
            dir = opendir(branch_dir);
            while((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) {
                    if(commit_ID < atoi(entry->d_name)){
                        commit_ID = atoi(entry->d_name);
                    }
                }
            }
            closedir(dir);
        }
    }
    
    char committedfiles_dir[MAX_FILENAME_LENGTH];
    strcpy(committedfiles_dir, neogit_dir);
    strcat(committedfiles_dir, "/committedfiles/");
    
    dir = opendir(committedfiles_dir);
    
    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && is_tracked(entry->d_name)) {
            checkout_file(entry->d_name, find_file_last_change_before_commit(entry->d_name, commit_ID));
        }
    }
    closedir(dir);

    return 0;
}

int find_file_last_change_before_commit(char *filepath, int commit_ID) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    
    strcpy(filepath_dir, neogit_dir);
    strcat(filepath_dir, "/committedfiles/");
    strcat(filepath_dir, filepath);

    int max = -1;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            if (tmp > max && tmp <= commit_ID) {
                max = tmp;
            }
        }
    }
    closedir(dir);

    return max;
}

int checkout_file(char *filepath, int commit_ID) {
    char src_file[MAX_FILENAME_LENGTH];
    strcpy(src_file, neogit_dir);
    strcat(src_file, "/committedfiles/");
    strcat(src_file, filepath);
    char tmp[10];
    sprintf(tmp, "/%d", commit_ID);
    strcat(src_file, tmp);

    FILE *read_file = fopen(src_file, "r");
    if (read_file == NULL) return 1;
    FILE *write_file = fopen(filepath, "w");
    if (write_file == NULL) return 1;
    
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), read_file) != NULL) {
        fprintf(write_file, "%s", line);
    }
    
    fclose(read_file);
    fclose(write_file);

    return 0;
}

int run_status(int argc){
    
    if (argc > 2) return 1;
    
    struct dirent *entry;
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            
            if(is_stagged(entry->d_name)) printf("%s+\n", entry->d_name);
            else{
                if(is_tracked(entry->d_name)) printf("%s-M\n", entry->d_name);
                else printf("%s-A\n", entry->d_name);
            }
        }
    }

    closedir(dir);
    return 0;
}

int set_shortcut(int argc, char * const argv[]){
    
    if(argc < 6){
        return 1;
    }
    
    char shortcut_dir[MAX_FILENAME_LENGTH];
    strcpy(shortcut_dir, neogit_dir);
    strcat(shortcut_dir, "/shortcuts");
    
    DIR *dir = opendir(shortcut_dir);
    
    if (dir == NULL) {
        perror("Error opening shortcut directory");
        return 1;
    }
    
    FILE * shortcut_file = fopen(argv[5], "w");
    fprintf(shortcut_file, "%s", argv[3]);
    closedir(dir);
    return 0;
}

int replace_shortcut(int argc, char * const argv[]){
    
    if(argc < 6){
        return 1;
    }
    
    bool exists = false;
    
    char shortcut_dir[MAX_FILENAME_LENGTH];
    strcpy(shortcut_dir, neogit_dir);
    strcat(shortcut_dir, "/shortcuts");
    
    struct dirent *entry;
    DIR *dir = opendir(shortcut_dir);
    
    if (dir == NULL) {
        perror("Error opening shortcut directory");
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG){
            if(strcmp(argv[5], entry->d_name) == 0) {
                exists = true;
                FILE * shortcut_file = fopen(argv[5], "w");
                fprintf(shortcut_file, "%s", argv[3]);
                fclose(shortcut_file);
            }
        }
    }
    closedir(dir);
    
    if(!exists) {
        printf("Error: No such shortcut exists.\n");
        return 1;
    }
    return 0;
}

int remove_shortcut(int argc, char * const argv[]){
    
    if(argc < 4){
        return 1;
    }
    
    bool exists = false;
    
    char shortcut_dir[MAX_FILENAME_LENGTH];
    strcpy(shortcut_dir, neogit_dir);
    strcat(shortcut_dir, "/shortcuts");
    
    struct dirent *entry;
    DIR *dir = opendir(shortcut_dir);
    
    if (dir == NULL) {
        perror("Error opening shortcut directory");
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG){
            if(strcmp(argv[5], entry->d_name) == 0) {
                remove(argv[5]);
            }
        }
    }
    closedir(dir);
    
    if(!exists) {
        printf("Error: No such shortcut exists.\n");
        return 1;
    }
    return 0;
}

int run_log(int argc, char * const argv[]){
    
    
    // to do: branch
    
    char commit_dir[MAX_FILENAME_LENGTH];
    strcpy(commit_dir, neogit_dir);
    strcat(commit_dir, "/commits");
    
    DIR *dir = opendir(commit_dir);
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening commits directory");
        return 1;
    }

    
    FILE * commit_file;
    char line[MAX_LINE_LENGTH];
    char line2[MAX_LINE_LENGTH];
    char line3[MAX_LINE_LENGTH];
    int count = 0;
    int num = -1;
    
    if(argc == 4 && strcmp(argv[2], "-n") == 0){ //neogit log -n [number]
        num = atoi(argv[3]);
    }
    while ((entry = readdir(dir)) != NULL && num != 0) {
        if (entry->d_type == DT_REG) {
            //we're in a commit file of the form:
            // <date & time>
            // message: <message>
            // <authorname>
            // files:
            // <files>
            num--;
            
            commit_file = fopen(entry->d_name, "r");
            fgets(line, sizeof(line), commit_file); //date & time line
            fgets(line2, sizeof(line2), commit_file); //message line
            fgets(line3, sizeof(line), commit_file); // <authorname>
            
            if(argc == 2){ // neogit log
                
                printf("Branch: master\n");
                printf("%s\n", line);
                printf("%s\n", line2);
                printf("Author: %s\n", line3);
                while (fgets(line, sizeof(line), commit_file) != NULL) {
                    count++;
                }
                printf("Commit ID: %s\n", entry->d_name);
                printf("Number of files: %d\n", count-1);
                count = 0;
                
            }
            else if(argc == 4 && strcmp(argv[2], "-search") == 0) {// neogit log -search <word>
                char * point = strstr(line2, argv[3]);
                if(point){ //the word exists in the message
                    
                    printf("Branch: master\n");
                    printf("%s\n", line);
                    printf("%s\n", line2);
                    printf("Author: %s\n", line3);
                    while (fgets(line, sizeof(line), commit_file) != NULL) {
                        count++;
                    }
                    printf("Commit ID: %s\n", entry->d_name);
                    printf("Number of files: %d\n", count-1);
                    count = 0;
                    
                }
            }
            else if(argc == 4 && strcmp(argv[2], "-author") == 0){// neogit log -author <authorname>
                
                if(strcmp(line3, argv[3]) == 0){
                    
                    
                    printf("Branch: master\n");
                    printf("%s\n", line);
                    printf("%s\n", line2);
                    printf("Author: %s\n", line3);
                    while (fgets(line, sizeof(line), commit_file) != NULL) {
                        count++;
                    }
                    printf("Commit ID: %s\n", entry->d_name);
                    printf("Number of files: %d\n", count-1);
                    count = 0;
                }
                
            }
        }
        
    }
        
    closedir(dir);
    
    return 0;
}





int run_branch (int argc, char * const argv[]) {
    
    if(argc > 3){
        return 1;
    }
    
    char commits_dir[MAX_FILENAME_LENGTH];
    strcpy(commits_dir, neogit_dir);
    strcat(commits_dir, "/commits");
    
    if(argc == 3) { //neogit branch <branch-name>
        char branch_dir[MAX_FILENAME_LENGTH];
        strcpy(branch_dir, commits_dir);
        strcat(branch_dir, argv[2]);
        
        struct stat stats;
        stat(branch_dir, &stats);

        // Check for file existence
        if (S_ISDIR(stats.st_mode)){
            printf("Error: A branch with the specified name already exists.\n");
            return 1;
        }

        // create branch folder
        if (mkdir(branch_dir, 0755) != 0) return 1;
        
        char config_file[MAX_FILENAME_LENGTH];
        strcpy(config_file, neogit_dir);
        strcat(config_file, "/config");
        FILE * config = fopen(config_file, "r");
        
        char tmpconfig_file[MAX_FILENAME_LENGTH];
        strcpy(tmpconfig_file, neogit_dir);
        strcat(tmpconfig_file, "/tmp_config");
        FILE * tmp_config = fopen(tmpconfig_file, "w");
        
        char line[MAX_LINE_LENGTH];
        
        while (fgets(line, sizeof(line), config) != NULL) {
            if (strncmp(line, "branch: ", 8) == 0) {

                fprintf(tmp_config, "branch: %s\n", argv[2]);

            } else fprintf(tmp_config, "%s", line);
        }
        fclose(config);
        fclose(tmp_config);
        
        remove(config_file);
        rename(tmpconfig_file, config_file);
        
    }
    else { //neogit branch
        printf("branches:\n");
        printf("master ");
        
        struct dirent *entry;
        DIR *dir = opendir(commits_dir);
        
        if (dir == NULL) {
            perror("Error opening commit directory");
            return 1;
        }
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                
                printf("%s ", entry->d_name);
            }
        }
        closedir(dir);
    }
    
    return 0;
}

int run_config (int argc, char * const argv[]) {
    
    if(strcmp(argv[2], "-global") == 0){
        // to do: create global config file with username and email and alias in it
        // if global config file already exists make a tmp file and write the stuff to it and replace it w the main file
        
    }
    else if(strcmp(argv[2], "user.name") == 0 || strcmp(argv[2], "user.email") == 0){
        char config_file[MAX_FILENAME_LENGTH];
        strcpy(config_file, neogit_dir);
        strcat(config_file, "/config");
        FILE * config = fopen(config_file, "r");
        
        char tmpconfig_file[MAX_FILENAME_LENGTH];
        strcpy(tmpconfig_file, neogit_dir);
        strcat(tmpconfig_file, "/tmp_config");
        FILE * tmp_config = fopen(tmpconfig_file, "w");
        
        char line[MAX_LINE_LENGTH];
        
        if(strcmp(argv[2], "user.name") == 0){ //neogit config user.name ""
            
            while (fgets(line, sizeof(line), config) != NULL) {
                if (strncmp(line, "username: ", 10) == 0) {

                    fprintf(tmp_config, "username: %s\n", argv[3]);

                } else fprintf(tmp_config, "%s", line);
            }
        }
        else if (strcmp(argv[2], "user.email") == 0){ //neogit config user.email ""
            
            while (fgets(line, sizeof(line), config) != NULL) {
                if (strncmp(line, "email: ", 7) == 0) {

                    fprintf(tmp_config, "email: %s\n", argv[3]);

                } else fprintf(tmp_config, "%s", line);
            }
        }
        
        fclose(config);
        fclose(tmp_config);
        
        remove(config_file);
        rename(tmpconfig_file, config_file);
        
    }
    else if (strncmp(argv[2], "alias.", 6) == 0){ //neogit config alias.<alias-name> "command"
        char alias_file[MAX_FILENAME_LENGTH];
        strcpy(alias_file, neogit_dir);
        strcat(alias_file, "/alias");
        FILE * alias = fopen(alias_file, "a");
        fprintf(alias, "%s\n%s\n", argv[2]+6, argv[3]);
        fclose(alias);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stdout, "please enter a valid command");
        return 1;
    }
    
    //print_command(argc, argv);
    strcpy(neogit_dir, find_neogit(neogit_dir));
    
    if (strcmp(argv[1], "config") == 0) {
        return run_config(argc, argv);
    } else if (strcmp(argv[1], "init") == 0) {
        return run_init(argc, argv);
    } else if (strcmp(argv[1], "add") == 0) {
        return run_add(argc, argv);
    } else if (strcmp(argv[1], "reset") == 0) {
        return run_reset(argc, argv);
    } else if (strcmp(argv[1], "commit") == 0) {
        return run_commit(argc, argv);
    } else if (strcmp(argv[1], "checkout") == 0) {
        return run_checkout(argc, argv);
    } else if (strcmp(argv[1], "status") == 0) {
        return run_status(argc);
    } else if (strcmp(argv[1], "set") == 0) {
        return set_shortcut(argc, argv);
    } else if (strcmp(argv[1], "replace") == 0) {
        return replace_shortcut(argc, argv);
    } else if (strcmp(argv[1], "remove") == 0) {
        return remove_shortcut(argc, argv);
    } else if (strcmp(argv[1], "log") == 0) {
        return run_log(argc, argv);
    } else if (strcmp(argv[1], "branch") == 0) {
        return run_branch(argc, argv);
    }
    
    return 0;
}

