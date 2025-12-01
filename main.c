#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 100
#define FILE_NAME "students.dat"

typedef struct {
    int id;
    char name[50];
    char reg_num[20];
    char branch[30];
    char program[20];
    char gender[10];
    char phone[15];
    int age;
    float gpa;
    struct {
        char subject_name[50];
        float marks;
    } subjects[6];
} Student;

char default_subject_names[6][50] = {
    "Subject 1", "Subject 2", "Subject 3", "Subject 4", "Subject 5", "Subject 6"
};

// ================== STUDENT GOBJECT (GTK4) ==================

#define STUDENT_TYPE_OBJECT (student_object_get_type())
G_DECLARE_FINAL_TYPE(StudentObject, student_object, STUDENT, OBJECT, GObject)

struct _StudentObject {
    GObject parent_instance;
    Student *data; // Pointer to the underlying Student struct in the array
    int index;     // Index in the global array
};

G_DEFINE_TYPE(StudentObject, student_object, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_NAME,
    PROP_REG_NUM,
    PROP_BRANCH,
    PROP_PROGRAM,
    PROP_GENDER,
    PROP_PHONE,
    PROP_AGE,
    PROP_GPA,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void student_object_init(StudentObject *self) {
}

static void student_object_get_property(GObject *object, guint property_id,
                                      GValue *value, GParamSpec *pspec) {
    StudentObject *self = STUDENT_OBJECT(object);
    // Safety check
    if (!self->data) return;

    switch (property_id) {
    case PROP_NAME:
        g_value_set_string(value, self->data->name);
        break;
    case PROP_REG_NUM:
        g_value_set_string(value, self->data->reg_num);
        break;
    case PROP_BRANCH:
        g_value_set_string(value, self->data->branch);
        break;
    case PROP_PROGRAM:
        g_value_set_string(value, self->data->program);
        break;
    case PROP_GENDER:
        g_value_set_string(value, self->data->gender);
        break;
    case PROP_PHONE:
        g_value_set_string(value, self->data->phone);
        break;
    case PROP_AGE:
        g_value_set_int(value, self->data->age);
        break;
    case PROP_GPA:
        g_value_set_float(value, self->data->gpa);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void student_object_set_property(GObject *object, guint property_id,
                                      const GValue *value, GParamSpec *pspec) {
    // Read-only for now via properties, we update underlying data directly
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
}

static void student_object_class_init(StudentObjectClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->get_property = student_object_get_property;
    gobject_class->set_property = student_object_set_property;

    obj_properties[PROP_NAME] = g_param_spec_string("name", "Name", "Student Name",
        "", G_PARAM_READABLE);
    obj_properties[PROP_REG_NUM] = g_param_spec_string("reg-num", "Reg Num", "Registration Number",
        "", G_PARAM_READABLE);
    obj_properties[PROP_BRANCH] = g_param_spec_string("branch", "Branch", "Student Branch",
        "", G_PARAM_READABLE);
    obj_properties[PROP_PROGRAM] = g_param_spec_string("program", "Program", "Student Program",
        "", G_PARAM_READABLE);
    obj_properties[PROP_GENDER] = g_param_spec_string("gender", "Gender", "Student Gender",
        "", G_PARAM_READABLE);
    obj_properties[PROP_PHONE] = g_param_spec_string("phone", "Phone", "Student Phone",
        "", G_PARAM_READABLE);
    obj_properties[PROP_AGE] = g_param_spec_int("age", "Age", "Student Age",
        0, 100, 0, G_PARAM_READABLE);
    obj_properties[PROP_GPA] = g_param_spec_float("gpa", "GPA", "Student GPA",
        0.0, 10.0, 0.0, G_PARAM_READABLE);

    g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);
}

StudentObject *student_object_new(Student *data, int index) {
    StudentObject *obj = g_object_new(STUDENT_TYPE_OBJECT, NULL);
    obj->data = data;
    obj->index = index;
    return obj;
}

Student students[MAX_STUDENTS];
int student_count = 0;

GtkWidget *window;
GtkWidget *search_entry;
GtkWidget *column_view; // Replaces tree_view
GListStore *list_store; // Replaces GtkListStore
GtkSingleSelection *selection_model; // For selection handling
GtkWidget *total_label, *avg_gpa_label;

// Stack + form widgets
GtkWidget *stack;
GtkWidget *add_name_entry;
GtkWidget *add_reg_entry;
GtkWidget *add_branch_combo;
GtkWidget *add_program_combo;
GtkWidget *add_gender_combo;
GtkWidget *add_phone_entry;
GtkWidget *add_age_spin;
GtkWidget *add_gpa_spin;

// dark mode / sidebar
GtkWidget *sidebar_revealer;
GtkWidget *dark_mode_switch;
gboolean is_dark_mode = FALSE;
GtkCssProvider *theme_provider = NULL;

// Function declarations
void load_data();
void save_data();
void refresh_table();
void update_statistics();
void show_edit_dialog(int index);
void delete_student_by_index(int index);
void on_search_changed(GtkEntry *entry, gpointer data);

void on_delete_clicked(GtkButton *button, gpointer data);
void on_edit_clicked(GtkButton *button, gpointer data);

void on_nav_list_clicked(GtkButton *button, gpointer data);
void on_nav_add_clicked(GtkButton *button, gpointer data);
void on_nav_about_clicked(GtkButton *button, gpointer data);
void on_add_student_clicked(GtkButton *button, gpointer data);
void on_cancel_add_clicked(GtkButton *button, gpointer data);
void on_save_new_student_clicked(GtkButton *button, gpointer data);

void apply_theme(gboolean dark);
void on_dark_mode_toggled(GtkSwitch *widget, GParamSpec *pspec, gpointer data);
void on_sidebar_toggle_clicked(GtkButton *button, gpointer data);

GtkWidget* create_list_page();
GtkWidget* create_add_page();
GtkWidget* create_about_page();
GtkWidget* create_marksheet_page();
GtkWidget* create_stat_card(const char *value, const char *label,
                            const char *sublabel, GtkWidget **value_label);

void on_student_row_activated(GtkColumnView *view, guint position, gpointer data);
void on_save_marks_clicked(GtkButton *button, gpointer data);
void on_cancel_marks_clicked(GtkButton *button, gpointer data);

void activate(GtkApplication *app, gpointer user_data);

// ColumnView Callbacks
static void setup_label_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_name_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_reg_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_branch_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_program_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_gender_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_phone_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_age_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);
static void bind_gpa_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item);

void on_save_new_student_clicked(GtkButton *button, gpointer data) {
    const char *name = gtk_editable_get_text(GTK_EDITABLE(add_name_entry));
    const char *reg = gtk_editable_get_text(GTK_EDITABLE(add_reg_entry));
    
    if (strlen(name) == 0 || strlen(reg) == 0) {
        g_print("Error: Name and Reg Num required\n");
        return;
    }

    if (student_count >= MAX_STUDENTS) {
        g_print("Error: Max students reached\n");
        return;
    }

    int i = student_count;
    
    strncpy(students[i].name, name, 49);
    strncpy(students[i].reg_num, reg, 19);

    GtkStringObject *branch_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(add_branch_combo));
    const char *branch = gtk_string_object_get_string(branch_obj);
    strncpy(students[i].branch, branch, 29);

    GtkStringObject *program_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(add_program_combo));
    const char *program = gtk_string_object_get_string(program_obj);
    strncpy(students[i].program, program, 19);

    GtkStringObject *gender_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(add_gender_combo));
    const char *gender = gtk_string_object_get_string(gender_obj);
    strncpy(students[i].gender, gender, 9);

    const char *phone = gtk_editable_get_text(GTK_EDITABLE(add_phone_entry));
    strncpy(students[i].phone, phone, 14);

    students[i].age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(add_age_spin));
    students[i].gpa = (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(add_gpa_spin));
    students[i].id = i + 1;

    // Initialize subjects with default names
    for (int j = 0; j < 6; j++) {
        strncpy(students[i].subjects[j].subject_name, default_subject_names[j], 49);
        students[i].subjects[j].marks = 0.0;
    }

    student_count++;
    save_data();
    refresh_table();
    
    // Clear inputs
    gtk_editable_set_text(GTK_EDITABLE(add_name_entry), "");
    gtk_editable_set_text(GTK_EDITABLE(add_reg_entry), "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(add_branch_combo), 0);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(add_program_combo), 0);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(add_gender_combo), 0);
    gtk_editable_set_text(GTK_EDITABLE(add_phone_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(add_age_spin), 18);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(add_gpa_spin), 0.0);

    gtk_stack_set_visible_child_name(GTK_STACK(stack), "list_page");
}

void apply_theme(gboolean dark) {
    const char *css_dark =
        "window { background: linear-gradient(135deg, #0f0c29, #302b63, #24243e); color: #ffffff; }"
        ".stat-card { background: rgba(255, 255, 255, 0.05); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 15px; padding: 20px; margin: 5px; box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37); }"
        ".stat-value { font-size: 42px; font-weight: 800; color: #ffffff; text-shadow: 0 0 10px rgba(255,255,255,0.3); }"
        ".stat-label { font-size: 13px; color: #a2a8d3; text-transform: uppercase; letter-spacing: 1px; margin-top: 5px; }"
        ".add-button { background: linear-gradient(135deg, #b026ff, #7b2cbf); color: white; border-radius: 8px; padding: 10px 24px; border: none; font-weight: bold; box-shadow: 0 4px 15px rgba(176, 38, 255, 0.4); transition: transform 0.2s; }"
        ".add-button:hover { background: linear-gradient(135deg, #c046ff, #9b4cff); transform: translateY(-2px); }"
        ".delete-button { background: linear-gradient(135deg, #ff416c, #ff4b2b); color: white; border-radius: 8px; padding: 10px 24px; border: none; font-weight: bold; box-shadow: 0 4px 15px rgba(255, 65, 108, 0.4); transition: transform 0.2s; }"
        ".delete-button:hover { background: linear-gradient(135deg, #ff517c, #ff5b3b); transform: translateY(-2px); }"
        ".sidebar { background: rgba(15, 12, 41, 0.95); border-right: 1px solid rgba(255, 255, 255, 0.05); }"
        ".sidebar-title label { color: #ffffff; font-size: 18px; font-weight: 800; letter-spacing: 1px; }"
        ".sidebar-nav-button { background: linear-gradient(90deg, rgba(255,255,255,0.03), transparent); color: #b8b8b8; border-radius: 10px; margin: 8px 15px; padding: 12px; font-weight: 600; border: 1px solid rgba(255,255,255,0.05); transition: all 0.3s; }"
        ".sidebar-nav-button:hover { background: linear-gradient(90deg, #7b2cbf, #b026ff); color: white; border-color: #b026ff; box-shadow: 0 0 20px rgba(176, 38, 255, 0.4); }"
        "dialog { background: #1e1e2e; color: #ffffff; border: 1px solid #7b2cbf; }"
        "dialog entry { background: #0f0c29; color: #ffffff; border: 1px solid #7b2cbf; }"
        "dialog label { color: #a2a8d3; }"
        "#custom-tree { background: rgba(0, 0, 0, 0.2); border: 1px solid rgba(123, 44, 191, 0.3); border-radius: 12px; margin-top: 15px; }"
        "#custom-tree listview { background: transparent; }"
        "#custom-tree listview row { background: transparent; color: #e0e0e0; border-bottom: 1px solid rgba(123, 44, 191, 0.2); transition: background 0.2s; }"
        "#custom-tree listview row:hover { background: rgba(123, 44, 191, 0.2); color: white; }"
        "#custom-tree listview row:selected { background: rgba(176, 38, 255, 0.2); color: white; border-left: 3px solid #b026ff; }"
        "#custom-tree header button { background: rgba(0,0,0,0.3); color: #b026ff; font-weight: bold; border: none; border-bottom: 2px solid #7b2cbf; text-transform: uppercase; letter-spacing: 1px; padding: 12px; }"
        ".search-entry { background: white; color: black; border-radius: 8px; border: none; padding: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }"
        ".search-entry text { color: black; }"
        "switch { background-color: #1a1a2e; border: 1px solid #7b2cbf; border-radius: 20px; min-height: 26px; min-width: 50px; transition: all 0.3s; }"
        "switch:checked { background-color: #302b63; border-color: #b026ff; }"
        "switch slider { background-color: #facc15; border-radius: 50%; margin: 3px; min-width: 20px; min-height: 20px; box-shadow: 0 0 8px #facc15; background-image: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPScyNCcgaGVpZ2h0PScyNCcgdmlld0JveD0nMCAwIDI0IDI0Jz48dGV4dCB4PSc1MCUnIHk9JzUwJScgZG9taW5hbnQtYmFzZWxpbmU9J2NlbnRyYWwnIHRleHQtYW5jaG9yPSdtaWRkbGUnIGZvbnQtc2l6ZT0nMTYnPvCfjJk8L3RleHQ+PC9zdmc+'); }";

    const char *css_light =
        "window { background: linear-gradient(135deg, #fdfbf7, #fff1eb); color: #4a403a; }"
        ".stat-card { background: rgba(255, 255, 255, 0.6); border: 1px solid rgba(255, 255, 255, 0.4); border-radius: 15px; padding: 20px; margin: 5px; box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.05); }"
        ".stat-value { font-size: 42px; font-weight: 800; color: #2d2420; text-shadow: 0 0 10px rgba(0,0,0,0.05); }"
        ".stat-label { font-size: 13px; color: #8d7f71; text-transform: uppercase; letter-spacing: 1px; margin-top: 5px; }"
        ".add-button { background: linear-gradient(135deg, #f6d365, #fda085); color: #4a403a; border-radius: 8px; padding: 10px 24px; border: none; font-weight: bold; box-shadow: 0 4px 15px rgba(246, 211, 101, 0.4); transition: transform 0.2s; }"
        ".add-button:hover { background: linear-gradient(135deg, #fce38a, #f38181); transform: translateY(-2px); }"
        ".delete-button { background: linear-gradient(135deg, #ff758c, #ff7eb3); color: white; border-radius: 8px; padding: 10px 24px; border: none; font-weight: bold; box-shadow: 0 4px 15px rgba(255, 117, 140, 0.4); transition: transform 0.2s; }"
        ".delete-button:hover { background: linear-gradient(135deg, #ff8fa3, #ff9ec3); transform: translateY(-2px); }"
        ".sidebar { background: rgba(255, 255, 255, 0.85); border-right: 1px solid rgba(0, 0, 0, 0.05); }"
        ".sidebar-title label { color: #2d2420; font-size: 18px; font-weight: 800; letter-spacing: 1px; }"
        ".sidebar-nav-button { background: linear-gradient(90deg, rgba(0,0,0,0.03), transparent); color: #8d7f71; border-radius: 10px; margin: 8px 15px; padding: 12px; font-weight: 600; border: 1px solid rgba(0,0,0,0.05); transition: all 0.3s; }"
        ".sidebar-nav-button:hover { background: linear-gradient(90deg, #f6d365, #fda085); color: #4a403a; border-color: #fda085; box-shadow: 0 0 20px rgba(246, 211, 101, 0.3); }"
        "dialog { background: #fff1eb; color: #4a403a; border: 1px solid #fda085; }"
        "dialog entry { background: #ffffff; color: #4a403a; border: 1px solid #e0e0e0; }"
        "dialog label { color: #8d7f71; }"
        "#custom-tree { background: rgba(255, 255, 255, 0.5); border: 1px solid rgba(253, 160, 133, 0.3); border-radius: 12px; margin-top: 15px; }"
        "#custom-tree listview { background: transparent; }"
        "#custom-tree listview row { background: transparent; color: #4a403a; border-bottom: 1px solid rgba(0,0,0,0.05); transition: background 0.2s; }"
        "#custom-tree listview row:hover { background: rgba(253, 160, 133, 0.1); color: #2d2420; }"
        "#custom-tree listview row:selected { background: rgba(246, 211, 101, 0.2); color: #2d2420; border-left: 3px solid #fda085; }"
        "#custom-tree header button { background: rgba(255,255,255,0.8); color: #d35400; font-weight: bold; border: none; border-bottom: 2px solid #fda085; text-transform: uppercase; letter-spacing: 1px; padding: 12px; }"
        "switch { background-color: #fff1eb; border-radius: 20px; border: 1px solid #fda085; min-height: 26px; min-width: 50px; transition: all 0.3s; }"
        "switch:checked { background-color: #fce38a; border-color: #f6d365; }"
        "switch slider { background-color: #f59e0b; border-radius: 50%; margin: 3px; min-width: 20px; min-height: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); background-image: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0naHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmcnIHdpZHRoPScyNCcgaGVpZ2h0PScyNCcgdmlld0JveD0nMCAwIDI0IDI0Jz48dGV4dCB4PSc1MCUnIHk9JzUwJScgZG9taW5hbnQtYmFzZWxpbmU9J2NlbnRyYWwnIHRleHQtYW5jaG9yPSdtaWRkbGUnIGZvbnQtc2l6ZT0nMTYnPuKYgO+4jzwvdGV4dD48L3N2Zz4='); }";

    if (!theme_provider) {
        theme_provider = gtk_css_provider_new();
    }

    gtk_css_provider_load_from_string(theme_provider, dark ? css_dark : css_light);

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(theme_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    is_dark_mode = dark;
}

// ===================== DARK MODE / SIDEBAR BUTTONS =====================

void on_dark_mode_toggled(GtkSwitch *widget, GParamSpec *pspec, gpointer data) {
    gboolean dark = gtk_switch_get_active(widget);
    apply_theme(dark);
}

void on_sidebar_toggle_clicked(GtkButton *button, gpointer data) {
    gboolean visible =
        gtk_revealer_get_reveal_child(GTK_REVEALER(sidebar_revealer));

    gtk_revealer_set_reveal_child(
        GTK_REVEALER(sidebar_revealer),
        !visible
    );
}



// ===================== UI BUILD HELPERS =====================

// ===================== UI BUILD HELPERS =====================

GtkWidget* create_stat_card(const char *value, const char *label, const char *sublabel,
                            GtkWidget **value_label) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_add_css_class(card, "stat-card");

    GtkWidget *val_label = gtk_label_new(value);
    gtk_widget_add_css_class(val_label, "stat-value");
    gtk_widget_set_halign(val_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), val_label);

    GtkWidget *main_label = gtk_label_new(label);
    gtk_widget_add_css_class(main_label, "stat-label");
    gtk_widget_set_halign(main_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), main_label);

    if (sublabel) {
        GtkWidget *sub_label = gtk_label_new(sublabel);
        gtk_widget_add_css_class(sub_label, "stat-sublabel");
        gtk_widget_set_halign(sub_label, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(card), sub_label);
    }

    if (value_label) *value_label = val_label;

    return card;
}

void on_search_changed(GtkEntry *entry, gpointer data) {
    refresh_table();
}

GtkWidget* create_list_page() {
    GtkWidget *page_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Header section
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_add_css_class(header, "header-box");
    gtk_box_append(GTK_BOX(page_vbox), header);

    // Statistics cards
    GtkWidget *stats_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(header), stats_box);

    gtk_box_append(GTK_BOX(stats_box),
                   create_stat_card("0", "Total Students", NULL, &total_label));
    gtk_box_append(GTK_BOX(stats_box),
                   create_stat_card("0.00", "Average GPA", NULL, &avg_gpa_label));

    // Search + Delete / Edit buttons
    GtkWidget *controls_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(header), controls_box);

    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry),
                                   "Search student by Name or Reg No.");
    gtk_widget_add_css_class(search_entry, "search-entry");
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_box_append(GTK_BOX(controls_box), search_entry);
    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), NULL);

    GtkWidget *delete_button = gtk_button_new_with_label("Delete Selected");
    gtk_widget_add_css_class(delete_button, "delete-button");
    gtk_box_append(GTK_BOX(controls_box), delete_button);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_clicked), NULL);

    GtkWidget *edit_button = gtk_button_new_with_label("Edit Selected");
    gtk_widget_add_css_class(edit_button, "add-button");
    gtk_box_append(GTK_BOX(controls_box), edit_button);
    g_signal_connect(edit_button, "clicked", G_CALLBACK(on_edit_clicked), NULL);

    // TreeView / ColumnView
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(page_vbox), scrolled_window);

    // Create ColumnView
    column_view = gtk_column_view_new(NULL);
    gtk_widget_set_name(column_view, "custom-tree"); // For CSS
    gtk_widget_add_css_class(column_view, "custom-tree");

    list_store = g_list_store_new(STUDENT_TYPE_OBJECT);
    selection_model = gtk_single_selection_new(G_LIST_MODEL(list_store));
    gtk_column_view_set_model(GTK_COLUMN_VIEW(column_view), GTK_SELECTION_MODEL(selection_model));
    g_signal_connect(column_view, "activate", G_CALLBACK(on_student_row_activated), NULL);

    // Helper macro for columns
    #define ADD_COLUMN(title, bind_func) \
        { \
            GtkListItemFactory *factory = gtk_signal_list_item_factory_new(); \
            g_signal_connect(factory, "setup", G_CALLBACK(setup_label_cb), NULL); \
            g_signal_connect(factory, "bind", G_CALLBACK(bind_func), NULL); \
            GtkColumnViewColumn *col = gtk_column_view_column_new(title, factory); \
            gtk_column_view_append_column(GTK_COLUMN_VIEW(column_view), col); \
            g_object_unref(col); \
        }

    ADD_COLUMN("Full Name", bind_name_cb);
    ADD_COLUMN("Reg Num", bind_reg_cb);
    ADD_COLUMN("Branch", bind_branch_cb);
    ADD_COLUMN("Program", bind_program_cb);
    ADD_COLUMN("Gender", bind_gender_cb);
    ADD_COLUMN("Phone", bind_phone_cb);
    ADD_COLUMN("Age", bind_age_cb);
    ADD_COLUMN("GPA", bind_gpa_cb);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), column_view);

    return page_vbox;
}

void on_delete_clicked(GtkButton *button, gpointer data) {
    guint position = gtk_single_selection_get_selected(selection_model);
    if (position == GTK_INVALID_LIST_POSITION) {
        // No selection
        // In GTK4 we don't have gtk_message_dialog_new easily blocking.
        // We'll just print for now or use a non-modal alert?
        // Let's use a simple alert dialog if possible, or just ignore.
        // For simplicity in migration, let's just return if nothing selected.
        return;
    }

    StudentObject *obj = STUDENT_OBJECT(g_list_model_get_item(G_LIST_MODEL(selection_model), position));
    int index = obj->index;
    g_object_unref(obj); // g_list_model_get_item returns a new reference

    // Delete directly (confirmation dialog is complex in GTK4 migration, skipping for now)
    delete_student_by_index(index);
}

void on_edit_clicked(GtkButton *button, gpointer data) {
    guint position = gtk_single_selection_get_selected(selection_model);
    if (position == GTK_INVALID_LIST_POSITION) return;

    StudentObject *obj = STUDENT_OBJECT(g_list_model_get_item(G_LIST_MODEL(selection_model), position));
    int index = obj->index;
    g_object_unref(obj);

    show_edit_dialog(index);
}

// ================== COLUMN VIEW CALLBACKS ==================

static void setup_label_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_label_new(NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_add_css_class(label, "cell-label");
    gtk_list_item_set_child(list_item, label);
}

static void bind_name_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->name);
}

static void bind_reg_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->reg_num);
}

static void bind_branch_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->branch);
}

static void bind_program_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->program);
}

static void bind_gender_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->gender);
}

static void bind_phone_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    gtk_label_set_text(GTK_LABEL(label), obj->data->phone);
}

static void bind_age_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", obj->data->age);
    gtk_label_set_text(GTK_LABEL(label), buf);
}

static void bind_gpa_cb(GtkSignalListItemFactory *factory, GtkListItem *list_item) {
    GtkWidget *label = gtk_list_item_get_child(list_item);
    StudentObject *obj = STUDENT_OBJECT(gtk_list_item_get_item(list_item));
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", obj->data->gpa);
    gtk_label_set_text(GTK_LABEL(label), buf);
}


// ===== MAIN =====

GtkWidget* create_add_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(box, 40);
    gtk_widget_set_margin_bottom(box, 40);

    GtkWidget *title = gtk_label_new("Add New Student");
    gtk_widget_add_css_class(title, "title-label");
    gtk_widget_add_css_class(title, "title-1"); // GTK4 typography class
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_box_append(GTK_BOX(box), grid);

    // Name
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Full Name:"), 0, 0, 1, 1);
    add_name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), add_name_entry, 1, 0, 1, 1);

    // Reg Num
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Registration Number:"), 0, 1, 1, 1);
    add_reg_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), add_reg_entry, 1, 1, 1, 1);

    // Branch
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Branch:"), 0, 2, 1, 1);
    const char *branches[] = {"CSE", "IT", "ECE", "EEE", "Mechanical", "Civil", "Other", NULL};
    add_branch_combo = gtk_drop_down_new_from_strings(branches);
    gtk_grid_attach(GTK_GRID(grid), add_branch_combo, 1, 2, 1, 1);

    // Program
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Program:"), 0, 3, 1, 1);
    const char *programs[] = {"BTECH", "MBA", "DIPLOMA", NULL};
    add_program_combo = gtk_drop_down_new_from_strings(programs);
    gtk_grid_attach(GTK_GRID(grid), add_program_combo, 1, 3, 1, 1);

    // Gender
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Gender:"), 0, 4, 1, 1);
    const char *genders[] = {"Male", "Female", "Other", NULL};
    add_gender_combo = gtk_drop_down_new_from_strings(genders);
    gtk_grid_attach(GTK_GRID(grid), add_gender_combo, 1, 4, 1, 1);

    // Phone
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Phone:"), 0, 5, 1, 1);
    add_phone_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), add_phone_entry, 1, 5, 1, 1);

    // Age
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Age:"), 0, 6, 1, 1);
    add_age_spin = gtk_spin_button_new_with_range(16, 60, 1);
    gtk_grid_attach(GTK_GRID(grid), add_age_spin, 1, 6, 1, 1);

    // GPA
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("GPA:"), 0, 7, 1, 1);
    add_gpa_spin = gtk_spin_button_new_with_range(0.0, 10.0, 0.01);
    gtk_grid_attach(GTK_GRID(grid), add_gpa_spin, 1, 7, 1, 1);

    // Buttons
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), btn_box);

    GtkWidget *save_btn = gtk_button_new_with_label("Save Student");
    gtk_widget_add_css_class(save_btn, "add-button"); // Reuse existing class
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_new_student_clicked), NULL);
    gtk_box_append(GTK_BOX(btn_box), save_btn);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_add_clicked), NULL);
    gtk_box_append(GTK_BOX(btn_box), cancel_btn);

    return box;
}

GtkWidget* create_about_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(box, 40);
    gtk_widget_set_margin_bottom(box, 40);

    GtkWidget *title = gtk_label_new("Project Team");
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_add_css_class(title, "title-label");
    gtk_box_append(GTK_BOX(box), title);

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(card, "stat-card");
    gtk_widget_set_size_request(card, 400, -1);
    gtk_box_append(GTK_BOX(box), card);

    // Member 1: Raksham
    GtkWidget *m1_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(card), m1_box);
    GtkWidget *m1_name = gtk_label_new("Raksham");
    gtk_widget_add_css_class(m1_name, "stat-value");
    gtk_widget_set_halign(m1_name, GTK_ALIGN_START);
    gtk_label_set_attributes(GTK_LABEL(m1_name), pango_attr_list_from_string("size=16000"));
    gtk_box_append(GTK_BOX(m1_box), m1_name);
    GtkWidget *m1_id = gtk_label_new("Admission No: 24GPTC4170010");
    gtk_widget_add_css_class(m1_id, "stat-label");
    gtk_widget_set_halign(m1_id, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(m1_box), m1_id);
    GtkWidget *m1_role = gtk_label_new("Role: Project Lead & System Architect");
    gtk_widget_add_css_class(m1_role, "stat-label");
    gtk_widget_set_halign(m1_role, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(m1_role, 5);
    gtk_box_append(GTK_BOX(m1_box), m1_role);

    gtk_box_append(GTK_BOX(card), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Member 2: Sunny
    GtkWidget *m2_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(card), m2_box);
    GtkWidget *m2_name = gtk_label_new("Sunny");
    gtk_widget_add_css_class(m2_name, "stat-value");
    gtk_widget_set_halign(m2_name, GTK_ALIGN_START);
    gtk_label_set_attributes(GTK_LABEL(m2_name), pango_attr_list_from_string("size=16000"));
    gtk_box_append(GTK_BOX(m2_box), m2_name);
    GtkWidget *m2_id = gtk_label_new("Admission No: 24GPTC4170005");
    gtk_widget_add_css_class(m2_id, "stat-label");
    gtk_widget_set_halign(m2_id, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(m2_box), m2_id);
    GtkWidget *m2_role = gtk_label_new("Role: UI/UX Lead & Theming");
    gtk_widget_add_css_class(m2_role, "stat-label");
    gtk_widget_set_halign(m2_role, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(m2_role, 5);
    gtk_box_append(GTK_BOX(m2_box), m2_role);

    gtk_box_append(GTK_BOX(card), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Member 3: Jahnavi
    GtkWidget *m3_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(card), m3_box);
    GtkWidget *m3_name = gtk_label_new("Jahnavi");
    gtk_widget_add_css_class(m3_name, "stat-value");
    gtk_widget_set_halign(m3_name, GTK_ALIGN_START);
    gtk_label_set_attributes(GTK_LABEL(m3_name), pango_attr_list_from_string("size=16000"));
    gtk_box_append(GTK_BOX(m3_box), m3_name);
    GtkWidget *m3_id = gtk_label_new("Admission No: 24GPTC4170048");
    gtk_widget_add_css_class(m3_id, "stat-label");
    gtk_widget_set_halign(m3_id, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(m3_box), m3_id);
    GtkWidget *m3_role = gtk_label_new("Role: Frontend Developer");
    gtk_widget_add_css_class(m3_role, "stat-label");
    gtk_widget_set_halign(m3_role, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(m3_role, 5);
    gtk_box_append(GTK_BOX(m3_box), m3_role);

    gtk_box_append(GTK_BOX(card), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Member 4: Payal
    GtkWidget *m4_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(card), m4_box);
    GtkWidget *m4_name = gtk_label_new("Payal");
    gtk_widget_add_css_class(m4_name, "stat-value");
    gtk_widget_set_halign(m4_name, GTK_ALIGN_START);
    gtk_label_set_attributes(GTK_LABEL(m4_name), pango_attr_list_from_string("size=16000"));
    gtk_box_append(GTK_BOX(m4_box), m4_name);
    GtkWidget *m4_id = gtk_label_new("Admission No: 24GPTC4170082");
    gtk_widget_add_css_class(m4_id, "stat-label");
    gtk_widget_set_halign(m4_id, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(m4_box), m4_id);
    GtkWidget *m4_role = gtk_label_new("Role: QA & Documentation");
    gtk_widget_add_css_class(m4_role, "stat-label");
    gtk_widget_set_halign(m4_role, GTK_ALIGN_START);
    gtk_widget_set_margin_bottom(m4_role, 5);
    gtk_box_append(GTK_BOX(m4_box), m4_role);

    gtk_box_append(GTK_BOX(card), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // GitHub Link
    GtkWidget *gh_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(card), gh_box);
    GtkWidget *gh_label = gtk_label_new("Project Source Code");
    gtk_widget_add_css_class(gh_label, "stat-label");
    gtk_widget_set_halign(gh_label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(gh_box), gh_label);
    
    GtkWidget *gh_link = gtk_link_button_new_with_label("https://github.com/rexyfound", "github.com/rexyfound");
    gtk_widget_set_halign(gh_link, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(gh_box), gh_link);

    GtkWidget *footer = gtk_label_new("Student Data Management System v1.0");
    gtk_widget_add_css_class(footer, "footer-label");
    gtk_widget_set_margin_top(footer, 20);
    gtk_box_append(GTK_BOX(box), footer);

    return box;
}

// ================== MARKSHEET PAGE ==================

GtkWidget *marks_name_label;
GtkWidget *marks_reg_label;
GtkWidget *subject_entries[6];
GtkWidget *marks_spins[6];
int current_marks_index = -1;

GtkWidget* create_marksheet_page() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(box, 40);
    gtk_widget_set_margin_bottom(box, 40);

    GtkWidget *title = gtk_label_new("Student Record / Marksheet");
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_add_css_class(title, "title-label");
    gtk_box_append(GTK_BOX(box), title);

    // Student Info
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_add_css_class(info_box, "stat-card");
    gtk_box_append(GTK_BOX(box), info_box);

    marks_name_label = gtk_label_new("Student Name");
    gtk_widget_add_css_class(marks_name_label, "stat-value");
    gtk_label_set_attributes(GTK_LABEL(marks_name_label), pango_attr_list_from_string("size=20000"));
    gtk_box_append(GTK_BOX(info_box), marks_name_label);

    marks_reg_label = gtk_label_new("Reg No");
    gtk_widget_add_css_class(marks_reg_label, "stat-label");
    gtk_box_append(GTK_BOX(info_box), marks_reg_label);

    // Marks Grid
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_append(GTK_BOX(box), grid);

    // Headers
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Subject Name"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Marks Obtained"), 1, 0, 1, 1);

    for (int i = 0; i < 6; i++) {
        subject_entries[i] = gtk_entry_new();
        gtk_widget_set_size_request(subject_entries[i], 200, -1);
        gtk_grid_attach(GTK_GRID(grid), subject_entries[i], 0, i + 1, 1, 1);

        marks_spins[i] = gtk_spin_button_new_with_range(0.0, 100.0, 1.0);
        gtk_grid_attach(GTK_GRID(grid), marks_spins[i], 1, i + 1, 1, 1);
    }

    // Buttons
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), btn_box);

    GtkWidget *save_btn = gtk_button_new_with_label("Save Record");
    gtk_widget_add_css_class(save_btn, "add-button");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_marks_clicked), NULL);
    gtk_box_append(GTK_BOX(btn_box), save_btn);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Back");
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_marks_clicked), NULL);
    gtk_box_append(GTK_BOX(btn_box), cancel_btn);

    return box;
}

void on_student_row_activated(GtkColumnView *view, guint position, gpointer data) {
    GtkSelectionModel *model = gtk_column_view_get_model(view);
    StudentObject *obj = STUDENT_OBJECT(g_list_model_get_item(G_LIST_MODEL(model), position));
    
    if (obj && obj->data) {
        current_marks_index = obj->index;
        
        gtk_label_set_text(GTK_LABEL(marks_name_label), obj->data->name);
        gtk_label_set_text(GTK_LABEL(marks_reg_label), obj->data->reg_num);

        for (int i = 0; i < 6; i++) {
            gtk_editable_set_text(GTK_EDITABLE(subject_entries[i]), obj->data->subjects[i].subject_name);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(marks_spins[i]), obj->data->subjects[i].marks);
        }

        gtk_stack_set_visible_child_name(GTK_STACK(stack), "marksheet_page");
    }
}

void on_save_marks_clicked(GtkButton *button, gpointer data) {
    if (current_marks_index >= 0 && current_marks_index < student_count) {
        for (int i = 0; i < 6; i++) {
            const char *subj_name = gtk_editable_get_text(GTK_EDITABLE(subject_entries[i]));
            float marks = gtk_spin_button_get_value(GTK_SPIN_BUTTON(marks_spins[i]));

            strncpy(students[current_marks_index].subjects[i].subject_name, subj_name, 49);
            students[current_marks_index].subjects[i].marks = marks;

            // Update global defaults (last saved wins)
            strncpy(default_subject_names[i], subj_name, 49);
        }
        save_data();
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "list_page");
    }
}

void on_cancel_marks_clicked(GtkButton *button, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "list_page");
}

void load_data() {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (fp) {
        fread(&student_count, sizeof(int), 1, fp);
        fread(students, sizeof(Student), student_count, fp);
        
        // Load default subject names if available
        if (student_count > 0) {
             for(int i=0; i<6; i++) {
                 if(strlen(students[0].subjects[i].subject_name) > 0) {
                     strncpy(default_subject_names[i], students[0].subjects[i].subject_name, 49);
                 }
             }
        }
        
        fclose(fp);
    }
}

void save_data() {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (fp) {
        fwrite(&student_count, sizeof(int), 1, fp);
        fwrite(students, sizeof(Student), student_count, fp);
        fclose(fp);
    }
}

void update_statistics() {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", student_count);
    gtk_label_set_text(GTK_LABEL(total_label), buf);

    if (student_count > 0) {
        float total_gpa = 0;
        for (int i = 0; i < student_count; i++) {
            total_gpa += students[i].gpa;
        }
        snprintf(buf, sizeof(buf), "%.2f", total_gpa / student_count);
        gtk_label_set_text(GTK_LABEL(avg_gpa_label), buf);
    } else {
        gtk_label_set_text(GTK_LABEL(avg_gpa_label), "0.00");
    }
}

void refresh_table() {
    g_list_store_remove_all(list_store);
    for (int i = 0; i < student_count; i++) {
        StudentObject *obj = student_object_new(&students[i], i);
        g_list_store_append(list_store, obj);
        g_object_unref(obj);
    }
    update_statistics();
}

void delete_student_by_index(int index) {
    if (index < 0 || index >= student_count) return;
    
    for (int i = index; i < student_count - 1; i++) {
        students[i] = students[i + 1];
    }
    student_count--;
    save_data();
    refresh_table();
}

// Navigation Callbacks
void on_nav_list_clicked(GtkButton *button, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "list_page");
}

void on_nav_add_clicked(GtkButton *button, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "add_page");
}

void on_nav_about_clicked(GtkButton *button, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "about_page");
}

void on_cancel_add_clicked(GtkButton *button, gpointer data) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "list_page");
}

// Edit Dialog
GtkWidget *edit_name_entry, *edit_reg_entry, *edit_branch_combo, *edit_program_combo, *edit_gender_combo, *edit_phone_entry, *edit_age_spin, *edit_gpa_spin;
int edit_index = -1;

void on_save_edit_clicked(GtkButton *button, gpointer data) {
    GtkWidget *dialog = GTK_WIDGET(data);
    
    if (edit_index >= 0 && edit_index < student_count) {
        const char *name = gtk_editable_get_text(GTK_EDITABLE(edit_name_entry));
        const char *reg = gtk_editable_get_text(GTK_EDITABLE(edit_reg_entry));
        
        strncpy(students[edit_index].name, name, 49);
        strncpy(students[edit_index].reg_num, reg, 19);
        
        GtkStringObject *branch_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(edit_branch_combo));
        strncpy(students[edit_index].branch, gtk_string_object_get_string(branch_obj), 29);

        GtkStringObject *program_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(edit_program_combo));
        strncpy(students[edit_index].program, gtk_string_object_get_string(program_obj), 19);
        
        GtkStringObject *gender_obj = gtk_drop_down_get_selected_item(GTK_DROP_DOWN(edit_gender_combo));
        strncpy(students[edit_index].gender, gtk_string_object_get_string(gender_obj), 9);

        const char *phone = gtk_editable_get_text(GTK_EDITABLE(edit_phone_entry));
        strncpy(students[edit_index].phone, phone, 14);
        
        students[edit_index].age = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(edit_age_spin));
        students[edit_index].gpa = (float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(edit_gpa_spin));
        
        save_data();
        refresh_table();
    }
    
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void on_cancel_edit_clicked(GtkButton *button, gpointer data) {
    GtkWidget *dialog = GTK_WIDGET(data);
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void show_edit_dialog(int index) {
    edit_index = index;
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Edit Student");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_append(GTK_BOX(box), grid);

    // Fields
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Name:"), 0, 0, 1, 1);
    edit_name_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(edit_name_entry), students[index].name);
    gtk_grid_attach(GTK_GRID(grid), edit_name_entry, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Reg Num:"), 0, 1, 1, 1);
    edit_reg_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(edit_reg_entry), students[index].reg_num);
    gtk_grid_attach(GTK_GRID(grid), edit_reg_entry, 1, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Branch:"), 0, 2, 1, 1);
    const char *branches[] = {"CSE", "IT", "ECE", "EEE", "Mechanical", "Civil", "Other", NULL};
    edit_branch_combo = gtk_drop_down_new_from_strings(branches);
    // Select current branch (simple loop)
    for(int i=0; branches[i]; i++) {
        if(strcmp(branches[i], students[index].branch) == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(edit_branch_combo), i);
            break;
        }
    }
    gtk_grid_attach(GTK_GRID(grid), edit_branch_combo, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Program:"), 0, 3, 1, 1);
    const char *programs[] = {"BTECH", "MBA", "DIPLOMA", NULL};
    edit_program_combo = gtk_drop_down_new_from_strings(programs);
    for(int i=0; programs[i]; i++) {
        if(strcmp(programs[i], students[index].program) == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(edit_program_combo), i);
            break;
        }
    }
    gtk_grid_attach(GTK_GRID(grid), edit_program_combo, 1, 3, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Gender:"), 0, 4, 1, 1);
    const char *genders[] = {"Male", "Female", "Other", NULL};
    edit_gender_combo = gtk_drop_down_new_from_strings(genders);
    for(int i=0; genders[i]; i++) {
        if(strcmp(genders[i], students[index].gender) == 0) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(edit_gender_combo), i);
            break;
        }
    }
    gtk_grid_attach(GTK_GRID(grid), edit_gender_combo, 1, 4, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Phone:"), 0, 5, 1, 1);
    edit_phone_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(edit_phone_entry), students[index].phone);
    gtk_grid_attach(GTK_GRID(grid), edit_phone_entry, 1, 5, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Age:"), 0, 6, 1, 1);
    edit_age_spin = gtk_spin_button_new_with_range(16, 60, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(edit_age_spin), students[index].age);
    gtk_grid_attach(GTK_GRID(grid), edit_age_spin, 1, 6, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("GPA:"), 0, 7, 1, 1);
    edit_gpa_spin = gtk_spin_button_new_with_range(0.0, 10.0, 0.01);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(edit_gpa_spin), students[index].gpa);
    gtk_grid_attach(GTK_GRID(grid), edit_gpa_spin, 1, 7, 1, 1);

    // Buttons
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), btn_box);

    GtkWidget *save_btn = gtk_button_new_with_label("Save Changes");
    gtk_widget_add_css_class(save_btn, "add-button");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_edit_clicked), dialog);
    gtk_box_append(GTK_BOX(btn_box), save_btn);

    GtkWidget *cancel_btn = gtk_button_new_with_label("Cancel");
    g_signal_connect(cancel_btn, "clicked", G_CALLBACK(on_cancel_edit_clicked), dialog);
    gtk_box_append(GTK_BOX(btn_box), cancel_btn);

    gtk_window_present(GTK_WINDOW(dialog));
}

void activate(GtkApplication *app, gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Student Data Management");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

    // Main Layout: Overlay for Sidebar + Main Content
    GtkWidget *overlay = gtk_overlay_new();
    gtk_window_set_child(GTK_WINDOW(window), overlay);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), hbox);

    // Sidebar
    sidebar_revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(sidebar_revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_reveal_child(GTK_REVEALER(sidebar_revealer), TRUE);
    gtk_box_append(GTK_BOX(hbox), sidebar_revealer);

    GtkWidget *sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(sidebar_box, "sidebar");
    gtk_widget_set_size_request(sidebar_box, 200, -1);
    gtk_revealer_set_child(GTK_REVEALER(sidebar_revealer), sidebar_box);

    // Sidebar Content
    GtkWidget *sidebar_title = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(sidebar_title, "sidebar-title");
    gtk_widget_set_margin_top(sidebar_title, 20);
    gtk_widget_set_margin_bottom(sidebar_title, 20);
    gtk_widget_set_margin_start(sidebar_title, 15);
    gtk_box_append(GTK_BOX(sidebar_box), sidebar_title);

    GtkWidget *app_icon = gtk_image_new_from_icon_name("user-info");
    gtk_widget_set_size_request(app_icon, 24, 24);
    gtk_box_append(GTK_BOX(sidebar_title), app_icon);

    GtkWidget *title_lbl = gtk_label_new("Student\nManager");
    gtk_box_append(GTK_BOX(sidebar_title), title_lbl);

    // Nav Buttons
    GtkWidget *nav_list = gtk_button_new_with_label("Dashboard");
    gtk_widget_add_css_class(nav_list, "sidebar-nav-button");
    gtk_widget_set_margin_start(nav_list, 10);
    gtk_widget_set_margin_end(nav_list, 10);
    g_signal_connect(nav_list, "clicked", G_CALLBACK(on_nav_list_clicked), NULL);
    gtk_box_append(GTK_BOX(sidebar_box), nav_list);

    GtkWidget *nav_add = gtk_button_new_with_label("Add Student");
    gtk_widget_add_css_class(nav_add, "sidebar-nav-button");
    gtk_widget_set_margin_start(nav_add, 10);
    gtk_widget_set_margin_end(nav_add, 10);
    g_signal_connect(nav_add, "clicked", G_CALLBACK(on_nav_add_clicked), NULL);
    gtk_box_append(GTK_BOX(sidebar_box), nav_add);

    GtkWidget *nav_about = gtk_button_new_with_label("About Us");
    gtk_widget_add_css_class(nav_about, "sidebar-nav-button");
    gtk_widget_set_margin_start(nav_about, 10);
    gtk_widget_set_margin_end(nav_about, 10);
    g_signal_connect(nav_about, "clicked", G_CALLBACK(on_nav_about_clicked), NULL);
    gtk_box_append(GTK_BOX(sidebar_box), nav_about);

    // Main Content Area
    GtkWidget *vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(vbox_main, TRUE);
    gtk_box_append(GTK_BOX(hbox), vbox_main);

    // Top Bar (Toggle Sidebar + Dark Mode Switch)
    GtkWidget *top_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(top_bar, 10);
    gtk_widget_set_margin_start(top_bar, 10);
    gtk_widget_set_margin_end(top_bar, 20); // Add margin for switch
    gtk_box_append(GTK_BOX(vbox_main), top_bar);

    GtkWidget *toggle_btn = gtk_button_new_with_label("☰");
    g_signal_connect(toggle_btn, "clicked", G_CALLBACK(on_sidebar_toggle_clicked), NULL);
    gtk_box_append(GTK_BOX(top_bar), toggle_btn);

    // Spacer to push switch to right
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(top_bar), spacer);

    // Dark Mode Toggle (Moved to Top Right)
    dark_mode_switch = gtk_switch_new();
    gtk_widget_set_valign(dark_mode_switch, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(dark_mode_switch, GTK_ALIGN_END);
    g_signal_connect(dark_mode_switch, "notify::active", G_CALLBACK(on_dark_mode_toggled), NULL);
    gtk_box_append(GTK_BOX(top_bar), dark_mode_switch);

    // Stack
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_widget_set_vexpand(stack, TRUE);
    gtk_widget_set_margin_top(stack, 10);
    gtk_widget_set_margin_bottom(stack, 10);
    gtk_widget_set_margin_start(stack, 20);
    gtk_widget_set_margin_end(stack, 20);

    // Scrolled Window for Stack
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), stack);
    gtk_box_append(GTK_BOX(vbox_main), scrolled_window);

    GtkWidget *list_page = create_list_page();
    gtk_stack_add_named(GTK_STACK(stack), list_page, "list_page");

    GtkWidget *add_page = create_add_page();
    gtk_stack_add_named(GTK_STACK(stack), add_page, "add_page");

    GtkWidget *about_page = create_about_page();
    gtk_stack_add_named(GTK_STACK(stack), about_page, "about_page");

    GtkWidget *marksheet_page = create_marksheet_page();
    gtk_stack_add_named(GTK_STACK(stack), marksheet_page, "marksheet_page");

    // Initial Setup
    apply_theme(FALSE); // Start with light mode
    refresh_table();

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    load_data();

    GtkApplication *app = gtk_application_new("com.example.studentrecords",
                                              G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
