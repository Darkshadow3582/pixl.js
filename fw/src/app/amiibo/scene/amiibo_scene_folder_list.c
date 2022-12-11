#include "amiibo_scene.h"
#include "app_amiibo.h"
#include "mui_list_view.h"
#include "nrf_log.h"
#include "vos.h"

static void amiibo_scene_folder_list_on_selected(mui_list_view_event_t event, mui_list_view_t *p_list_view,
                                                 mui_list_item_t *p_item) {
    app_amiibo_t *app = p_list_view->user_data;
    if (event == MUI_LIST_VIEW_EVENT_SELECTED) {
        string_set(app->current_folder, p_item->text);
        mui_scene_dispatcher_next_scene(app->p_scene_dispatcher, AMIIBO_SCENE_AMIIBO_LIST);
    } else {
        string_set(app->current_folder, p_item->text);
        mui_scene_dispatcher_next_scene(app->p_scene_dispatcher, AMIIBO_SCENE_FOLDER_LIST_MENU);
    }
}

void amiibo_scene_folder_list_on_enter(void *user_data) {
    app_amiibo_t *app = user_data;
    vos_obj_t folders[VOS_MAX_FOLDER_SIZE];

    uint32_t folder_size =
        vos_get_driver(app->current_drive)->list_folder(VOS_BUCKET_AMIIBO, folders, VOS_MAX_FOLDER_SIZE);

    if (folder_size == 0) {
        mui_list_view_add_item(app->p_list_view, 0xe1d6, "<Empty Folder>", -1);
    } else {
        for (uint32_t i = 0; i < folder_size; i++) {
            mui_list_view_add_item(app->p_list_view, 0xe1d6, folders[i].name, (void *)i);
        }
    }

    mui_list_view_set_selected_cb(app->p_list_view, amiibo_scene_folder_list_on_selected);
    mui_list_view_set_user_data(app->p_list_view, app);

    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, AMIIBO_VIEW_ID_LIST);
}

void amiibo_scene_folder_list_on_exit(void *user_data) {
    app_amiibo_t *app = user_data;
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
    mui_list_view_set_user_data(app->p_list_view, NULL);
    mui_list_view_clear_items(app->p_list_view);
}