#include "data.h"
#include "cJSON.h"
#include "state.h"
#include "utils.h"

#include <glib.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to read a file into a string
static char *errands_data_read() {
  // Get data dir
  const char *data_dir =
      g_build_path("/", g_get_user_data_dir(), "errands", NULL);
  // Create if not exist
  if (!directory_exists(data_dir)) {
    g_mkdir_with_parents(data_dir, 0755);
  }
  // Get data.json file path
  const char *data_file_path = g_build_path("/", data_dir, "data.json", NULL);
  // Create if not exist
  if (!file_exists(data_file_path)) {
    FILE *file = fopen(data_file_path, "w");
    fprintf(file, "{\"lists\":[],\"tags\":[],\"tasks\":[]}");
    fclose(file);
  }

  FILE *file = fopen(data_file_path, "r"); // Open the file in read mode
  if (!file) {
    LOG("Could not open file"); // Print error if file cannot be opened
    return NULL;
  }

  // Move the file pointer to the end of the file to get the size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file); // Get the current position (file size)
  fseek(file, 0, SEEK_SET);     // Move back to the beginning of the file
  // Allocate memory for the string (+1 for the null terminator)
  char *buffer = (char *)malloc(file_size + 1);
  fread(buffer, 1, file_size, file);
  buffer[file_size] = '\0'; // Null-terminate the string
  fclose(file);

  // Free memory
  g_free((gpointer)data_dir);
  g_free((gpointer)data_file_path);

  return buffer; // Return the string
}

void errands_data_load() {
  LOG("Loading user data");
  char *data = errands_data_read();
  cJSON *json = cJSON_Parse(data);
  free(data);

  // Parse lists
  state.tl_data = g_ptr_array_new();
  cJSON *arr = cJSON_GetObjectItem(json, "lists");
  int len = cJSON_GetArraySize(arr);
  cJSON *item;
  for (int i = 0; i < len; i++) {
    item = cJSON_GetArrayItem(arr, i);
    TaskListData *tl = malloc(sizeof(*tl));
    tl->color = strdup(cJSON_GetObjectItem(item, "color")->valuestring);
    tl->deleted = (bool)cJSON_GetObjectItem(item, "deleted")->valueint;
    tl->name = strdup(cJSON_GetObjectItem(item, "name")->valuestring);
    tl->show_completed =
        (bool)cJSON_GetObjectItem(item, "show_completed")->valueint;
    tl->synced = (bool)cJSON_GetObjectItem(item, "synced")->valueint;
    tl->uid = strdup(cJSON_GetObjectItem(item, "uid")->valuestring);
    g_ptr_array_add(state.tl_data, tl);
  }

  // Parse tasks
  arr = cJSON_GetObjectItem(json, "tasks");
  len = cJSON_GetArraySize(arr);
  state.t_data = g_ptr_array_new();
  for (int i = 0; i < len; i++) {
    item = cJSON_GetArrayItem(arr, i);
    TaskData *t = malloc(sizeof(*t));
    t->attachments = g_ptr_array_new();
    // Get attachments
    t->tags = g_ptr_array_new();
    cJSON *atch_arr = cJSON_GetObjectItem(item, "attachments");
    for (int i = 0; i < cJSON_GetArraySize(atch_arr); i++) {
      g_ptr_array_add(t->attachments,
                      strdup(cJSON_GetArrayItem(atch_arr, i)->valuestring));
    }
    t->color = strdup(cJSON_GetObjectItem(item, "color")->valuestring);
    t->completed = (bool)cJSON_GetObjectItem(item, "completed")->valueint;
    t->changed_at =
        strdup(cJSON_GetObjectItem(item, "changed_at")->valuestring);
    t->created_at =
        strdup(cJSON_GetObjectItem(item, "created_at")->valuestring);
    t->deleted = (bool)cJSON_GetObjectItem(item, "deleted")->valueint;
    t->due_date = strdup(cJSON_GetObjectItem(item, "due_date")->valuestring);
    t->expanded = (bool)cJSON_GetObjectItem(item, "expanded")->valueint;
    t->list_uid = strdup(cJSON_GetObjectItem(item, "list_uid")->valuestring);
    t->notes = strdup(cJSON_GetObjectItem(item, "notes")->valuestring);
    t->notified = (bool)cJSON_GetObjectItem(item, "notified")->valueint;
    t->parent = strdup(cJSON_GetObjectItem(item, "parent")->valuestring);
    t->percent_complete =
        cJSON_GetObjectItem(item, "percent_complete")->valueint;
    t->priority = cJSON_GetObjectItem(item, "priority")->valueint;
    t->rrule = strdup(cJSON_GetObjectItem(item, "rrule")->valuestring);
    t->start_date =
        strdup(cJSON_GetObjectItem(item, "start_date")->valuestring);
    t->synced = (bool)cJSON_GetObjectItem(item, "synced")->valueint;
    // Get tags
    t->tags = g_ptr_array_new();
    cJSON *tags_arr = cJSON_GetObjectItem(item, "tags");
    for (int i = 0; i < cJSON_GetArraySize(tags_arr); i++) {
      g_ptr_array_add(t->tags,
                      strdup(cJSON_GetArrayItem(tags_arr, i)->valuestring));
    }
    t->text = strdup(cJSON_GetObjectItem(item, "text")->valuestring);
    t->toolbar_shown =
        (bool)cJSON_GetObjectItem(item, "toolbar_shown")->valueint;
    t->trash = (bool)cJSON_GetObjectItem(item, "trash")->valueint;
    t->uid = strdup(cJSON_GetObjectItem(item, "uid")->valuestring);
    g_ptr_array_add(state.t_data, t);
  }

  // Parse tags
  arr = cJSON_GetObjectItem(json, "tags");
  len = cJSON_GetArraySize(arr);
  state.tags_data = g_ptr_array_new();
  for (int i = 0; i < len; i++) {
    item = cJSON_GetArrayItem(arr, i);
    g_ptr_array_add(state.tags_data, strdup(item->valuestring));
  }
  cJSON_Delete(json);
}

void errands_data_write() {
  LOG("Writing user data");

  cJSON *json = cJSON_CreateObject();

  // Write lists data
  cJSON *lists = cJSON_CreateArray();
  for (int i = 0; i < state.tl_data->len; i++) {
    TaskListData *data = state.tl_data->pdata[i];
    cJSON *l_data = cJSON_CreateObject();
    cJSON_AddItemToObject(l_data, "color", cJSON_CreateString(data->color));
    cJSON_AddItemToObject(l_data, "deleted", cJSON_CreateBool(data->deleted));
    cJSON_AddItemToObject(l_data, "name", cJSON_CreateString(data->name));
    cJSON_AddItemToObject(l_data, "show_completed",
                          cJSON_CreateBool(data->show_completed));
    cJSON_AddItemToObject(l_data, "synced", cJSON_CreateBool(data->synced));
    cJSON_AddItemToObject(l_data, "uid", cJSON_CreateString(data->uid));
    cJSON_AddItemToArray(lists, l_data);
  }
  cJSON_AddItemToObject(json, "lists", lists);

  // Write tags data
  cJSON *tags = cJSON_CreateArray();
  for (int i = 0; i < state.tags_data->len; i++) {
    char *data = state.tags_data->pdata[i];
    cJSON_AddItemToArray(tags, cJSON_CreateString(data));
  }
  cJSON_AddItemToObject(json, "tags", tags);

  // Write tasks data
  cJSON *tasks = cJSON_CreateArray();
  for (int i = 0; i < state.t_data->len; i++) {
    TaskData *data = state.t_data->pdata[i];
    cJSON *t_data = cJSON_CreateObject();
    cJSON_AddItemToObject(t_data, "attachments", cJSON_CreateArray());
    cJSON_AddItemToObject(t_data, "color", cJSON_CreateString(data->color));
    cJSON_AddItemToObject(t_data, "completed",
                          cJSON_CreateBool(data->completed));
    cJSON_AddItemToObject(t_data, "changed_at",
                          cJSON_CreateString(data->changed_at));
    cJSON_AddItemToObject(t_data, "created_at",
                          cJSON_CreateString(data->created_at));
    cJSON_AddItemToObject(t_data, "deleted", cJSON_CreateBool(data->deleted));
    cJSON_AddItemToObject(t_data, "due_date",
                          cJSON_CreateString(data->due_date));
    cJSON_AddItemToObject(t_data, "expanded", cJSON_CreateBool(data->expanded));
    cJSON_AddItemToObject(t_data, "list_uid",
                          cJSON_CreateString(data->list_uid));
    cJSON_AddItemToObject(t_data, "notes", cJSON_CreateString(data->notes));
    cJSON_AddItemToObject(t_data, "notified", cJSON_CreateBool(data->notified));
    cJSON_AddItemToObject(t_data, "parent", cJSON_CreateString(data->parent));
    cJSON_AddItemToObject(t_data, "percent_complete",
                          cJSON_CreateNumber(data->percent_complete));
    cJSON_AddItemToObject(t_data, "priority",
                          cJSON_CreateNumber(data->priority));
    cJSON_AddItemToObject(t_data, "rrule", cJSON_CreateString(data->rrule));
    cJSON_AddItemToObject(t_data, "start_date",
                          cJSON_CreateString(data->start_date));
    cJSON_AddItemToObject(t_data, "synced", cJSON_CreateBool(data->synced));
    cJSON_AddItemToObject(t_data, "tags", cJSON_CreateArray());
    cJSON_AddItemToObject(t_data, "text", cJSON_CreateString(data->text));
    cJSON_AddItemToObject(t_data, "toolbar_shown",
                          cJSON_CreateBool(data->toolbar_shown));
    cJSON_AddItemToObject(t_data, "trash", cJSON_CreateBool(data->trash));
    cJSON_AddItemToObject(t_data, "uid", cJSON_CreateString(data->uid));

    cJSON_AddItemToArray(tasks, t_data);
  }
  cJSON_AddItemToObject(json, "tasks", tasks);

  // Save to file
  char *json_string = cJSON_PrintUnformatted(json);
  FILE *file = fopen("data.json", "w");
  if (file == NULL) {
    perror("Error opening file");
    cJSON_Delete(json);
    free(json_string);
  }
  fprintf(file, "%s", json_string);
  fclose(file);

  // Clean up
  cJSON_Delete(json);
  free(json_string);
}

void errands_data_add_list(char *name) {}

TaskData *errands_data_add_task(char *text, char *list_uid, char *parent_uid) {
  TaskData *t = malloc(sizeof(*t));
  t->attachments = NULL;
  t->color = strdup("");
  t->completed = false;
  t->changed_at = get_date_time();
  t->created_at = get_date_time();
  t->deleted = false;
  t->due_date = strdup("");
  t->expanded = false;
  t->list_uid = strdup(list_uid);
  t->notes = strdup("");
  t->notified = false;
  t->parent = strdup(parent_uid);
  t->percent_complete = 0;
  t->priority = 0;
  t->rrule = strdup("");
  t->start_date = strdup("");
  t->synced = false;
  t->tags = NULL;
  t->text = strdup(text);
  t->toolbar_shown = false;
  t->trash = false;
  t->uid = g_uuid_string_random();
  g_ptr_array_insert(state.t_data, 0, t);
  errands_data_write();
  return t;
}

TaskData *errands_data_get_task(char *uid) {
  TaskData *td = NULL;
  for (int i = 0; i < state.t_data->len; i++) {
    td = state.t_data->pdata[i];
    if (!strcmp(td->uid, uid))
      break;
  }
  return td;
}

static void errands_print_tasks(const char *parent_uid, const char *list_uid,
                                int indent) {
  for (int i = 0; i < state.t_data->len; i++) {
    TaskData *td = state.t_data->pdata[i];
    if (!strcmp(td->parent, parent_uid) && !strcmp(td->list_uid, list_uid)) {
      for (int j = 0; j < indent; j++)
        printf("    ");
      printf("[%s] %s\n", td->completed ? "x" : " ", td->text);
      errands_print_tasks(td->uid, list_uid, indent + 1);
    }
  }
}

void errands_data_print(char *list_uid, char *file_path) {
  // Print list name
  const char *list_name;
  for (int i = 0; i < state.tl_data->len; i++) {
    TaskListData *tld = state.tl_data->pdata[i];
    if (!strcmp(list_uid, tld->uid)) {
      list_name = tld->name;
      break;
    }
  }
  int len = strlen(list_name);
  printf("╔");
  for (int i = 0; i <= len + 1; i++) {
    printf("═");
  }
  printf("╗\n");
  printf("║ %s ║\n", list_name);
  printf("╚");
  for (int i = 0; i <= len + 1; i++) {
    printf("═");
  }
  printf("╝\n");
  // Print tasks
  errands_print_tasks("", list_uid, 0);
}
