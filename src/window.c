#include "window.h"
#include "notes-window.h"
#include "priority-window.h"
#include "sidebar.h"
#include "state.h"
#include "tags-page.h"

void errands_window_build() {
  LOG("Creating main window");

  state.main_window = adw_application_window_new(GTK_APPLICATION(state.app));
  g_object_set(state.main_window, "title", "Errands", "hexpand", true, NULL);
  gtk_window_set_default_size(GTK_WINDOW(state.main_window), 800, 600);

  // Main View Stack
  state.stack = adw_view_stack_new();

  errands_notes_window_build();
  errands_priority_window_build();
  errands_tags_page_build();
  errands_task_list_build();
  errands_sidebar_build();

  // Split view
  state.split_view = adw_navigation_split_view_new();
  adw_navigation_split_view_set_sidebar(
      ADW_NAVIGATION_SPLIT_VIEW(state.split_view),
      adw_navigation_page_new(state.sidebar, "Sidebar"));
  adw_navigation_split_view_set_content(
      ADW_NAVIGATION_SPLIT_VIEW(state.split_view),
      adw_navigation_page_new(state.stack, "Content"));

  adw_application_window_set_content(ADW_APPLICATION_WINDOW(state.main_window),
                                     state.split_view);
}
