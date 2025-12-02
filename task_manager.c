#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configuration ---
#define MAX_TASKS 100 // Maximum number of tasks the manager can handle
#define MAX_DESC_LEN 100 // Maximum length for a task description
#define FILENAME "tasks.txt" // File to store task data

// --- Data Structure ---

// Define the Task structure
typedef struct {
    int id; // Unique identifier for the task
    char description[MAX_DESC_LEN]; // What the task is
    int completed; // 0 for incomplete, 1 for completed
} Task;

// Array to hold all tasks in memory
Task task_list[MAX_TASKS]; 
int task_count = 0; // Current number of tasks in the list
int next_id = 1; // Tracks the next ID to assign to a new task

// --- Utility Function ---

// Clears the console screen, supporting both Windows and Unix systems
void clear_screen() {
    // Windows command to clear the console
    #ifdef _WIN32
        system("cls");
    // Unix/Linux/macOS command to clear the console
    #else
        system("clear");
    #endif
}

// --- File I/O Functions ---

// Saves the current list of tasks to the file
void save_tasks() {
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        printf("Error: Could not open %s for writing.\n", FILENAME);
        return;
    }

    // Write the current task count and next_id first
    fprintf(file, "%d\n", task_count);
    fprintf(file, "%d\n", next_id);

    // Write each task
    for (int i = 0; i < task_count; i++) {
        // Format: ID|Completed|Description\n
        fprintf(file, "%d|%d|%s\n", 
                task_list[i].id, 
                task_list[i].completed, 
                task_list[i].description);
    }

    fclose(file);
}

// Loads tasks from the file into the task_list array
void load_tasks() {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        // If the file doesn't exist, start with an empty list
        task_count = 0;
        next_id = 1;
        printf("Info: Task file not found. Starting with an empty list.\n");
        return;
    }

    // Read task_count and next_id
    if (fscanf(file, "%d\n", &task_count) != 1) {
        task_count = 0; // Handle empty file case
    }
    if (fscanf(file, "%d\n", &next_id) != 1) {
        next_id = 1; // Handle case where next_id is missing
    }

    // Read each task
    for (int i = 0; i < task_count; i++) {
        char buffer[MAX_DESC_LEN + 10]; // Buffer to read the whole line
        if (fgets(buffer, sizeof(buffer), file) == NULL) {
            // Stop if we run out of lines prematurely
            task_count = i;
            break;
        }

        // Parse the line using sscanf
        // Expected format: ID|Completed|Description
        // %d - read integer, | - literal character, %[^\n] - read until newline
        if (sscanf(buffer, "%d|%d|%99[^\n]", 
                   &task_list[i].id, 
                   &task_list[i].completed, 
                   task_list[i].description) != 3) {
            // Failed to parse a line, print error and truncate list
            printf("Warning: Corrupt data in file, skipping task %d.\n", i + 1);
            task_count = i;
            break; 
        }
    }

    fclose(file);
    printf("Info: Loaded %d tasks.\n", task_count);
}

// --- Task Management Functions ---

// Adds a new task
void add_task() {
    if (task_count >= MAX_TASKS) {
        printf("Error: Task list is full (Max %d).\n", MAX_TASKS);
        return;
    }

    printf("\nEnter task description (max %d chars): \n> ", MAX_DESC_LEN);
    // Read the line from stdin, store in the new task structure
    if (fgets(task_list[task_count].description, MAX_DESC_LEN, stdin) == NULL) {
        printf("Error reading input.\n");
        return;
    }

    // Remove the newline character added by fgets
    task_list[task_count].description[strcspn(task_list[task_count].description, "\n")] = 0;
    
    // Assign ID and default status
    task_list[task_count].id = next_id++;
    task_list[task_count].completed = 0; // 0 means Incomplete

    task_count++;
    printf("Success: Task #%d added.\n", task_list[task_count-1].id);
    save_tasks();
}

// Displays all tasks
void view_tasks() {
    printf("\n--- Task List ---\n");
    if (task_count == 0) {
        printf("No tasks currently in the list.\n");
        printf("-----------------\n");
        return;
    }

    printf(" ID | Status     | Description\n");
    printf("----|------------|----------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < task_count; i++) {
        const char *status = task_list[i].completed ? "COMPLETED" : "INCOMPLETE";
        printf(" %2d | %-10s | %s\n", 
               task_list[i].id, 
               status, 
               task_list[i].description);
    }
    printf("-----------------\n");
    
    // Add a pause to allow the user to read the list before clearing
    printf("\nPress ENTER to return to the menu...");
    getchar(); 
}

// Marks a task as complete
void update_task_status() {
    int id;
    view_tasks(); // Show the current list first

    if (task_count == 0) return;

    printf("\nEnter the ID of the task to mark as COMPLETED: ");
    if (scanf("%d", &id) != 1) {
        // Clear input buffer for safety if scanf fails
        while (getchar() != '\n');
        printf("Invalid input. Please enter a number.\n");
        return;
    }
    // Clear input buffer after successful scanf
    while (getchar() != '\n'); 

    // Find the task by ID
    for (int i = 0; i < task_count; i++) {
        if (task_list[i].id == id) {
            task_list[i].completed = 1; // Mark as complete
            printf("Success: Task #%d ('%s') marked as COMPLETED.\n", 
                   id, task_list[i].description);
            save_tasks();
            return;
        }
    }

    printf("Error: Task with ID %d not found.\n", id);
}

// Removes a task by ID
void remove_task() {
    int id;
    view_tasks();

    if (task_count == 0) return;

    printf("\nEnter the ID of the task to REMOVE: ");
    if (scanf("%d", &id) != 1) {
        while (getchar() != '\n');
        printf("Invalid input. Please enter a number.\n");
        return;
    }
    while (getchar() != '\n'); 

    int found_index = -1;
    // Find the task index by ID
    for (int i = 0; i < task_count; i++) {
        if (task_list[i].id == id) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Shift all subsequent tasks one position to the left
        // to fill the gap left by the removed task
        printf("Success: Removing Task #%d ('%s').\n", 
               id, task_list[found_index].description);

        for (int i = found_index; i < task_count - 1; i++) {
            task_list[i] = task_list[i+1];
        }
        task_count--; // Decrease the total task count

        save_tasks();
    } else {
        printf("Error: Task with ID %d not found.\n", id);
    }
}

// --- Main Program and Menu ---

void display_menu() {
    printf("\n==================================\n");
    printf("      C Console Task Manager\n");
    printf("==================================\n");
    printf("1. View Tasks\n");
    printf("2. Add New Task\n");
    printf("3. Mark Task as Completed\n");
    printf("4. Remove Task\n");
    printf("5. Exit and Save\n");
    printf("----------------------------------\n");
    printf("Enter your choice: ");
}

int main() {
    // 1. Load data from file at startup
    load_tasks(); 

    int choice;
    char buffer[10]; // Buffer for menu input

    while (1) {
        // Always clear screen before displaying the menu for a clean UI
        clear_screen(); 
        display_menu();

        // Read choice as a string to handle non-integer input gracefully
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        // Convert the string to an integer
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                view_tasks();
                break;
            case 2:
                add_task();
                break;
            case 3:
                update_task_status();
                break;
            case 4:
                remove_task();
                break;
            case 5:
                printf("\nExiting Task Manager. Tasks saved to %s.\n", FILENAME);
                return 0; // Exit the main function
            default:
                printf("\nInvalid choice. Please enter a number between 1 and 5.\n");
                printf("\nPress ENTER to continue...");
                getchar(); // Wait for user input
                break;
        }
    }
    return 0; // Should not be reached, but good practice
}