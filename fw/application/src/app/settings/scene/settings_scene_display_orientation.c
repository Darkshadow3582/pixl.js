#include "app_settings.h"
#include "i18n/language.h"
#include "mui_icons.h"
#include "settings.h"
#include "settings_scene.h"

enum settings_display_orientation_menu_t {
    SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE,
    SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE_180,
    SETTINGS_DISPLAY_ORIENTATION_MENU_EXIT,
};

static void settings_scene_display_orientation_list_view_on_selected(mui_list_view_event_t event,
                                                                     mui_list_view_t *p_list_view,
                                                                     mui_list_item_t *p_item) {
    app_settings_t *app = p_list_view->user_data;
    uint32_t selection = (uint32_t)p_item->user_data;
    settings_data_t *p_settings = settings_get_data();

    switch (selection) {
    case SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE:
        p_settings->display_orientation = DISPLAY_ORIENTATION_LANDSCAPE;
        mui_u8g2_set_display_orientation(p_settings->display_orientation);
        break;

    case SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE_180:
        p_settings->display_orientation = DISPLAY_ORIENTATION_LANDSCAPE_180;
        mui_u8g2_set_display_orientation(p_settings->display_orientation);
        break;

    case SETTINGS_DISPLAY_ORIENTATION_MENU_EXIT:
    default:
        break;
    }

    mui_scene_dispatcher_previous_scene(app->p_scene_dispatcher);
}

void settings_scene_display_orientation_on_enter(void *user_data) {
    app_settings_t *app = user_data;

    mui_list_view_add_item(app->p_list_view, ICON_ARROW_UP, _T(APP_SET_LANDSCAPE),
                           (void *)SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE);
    mui_list_view_add_item(app->p_list_view, ICON_ARROW_DOWN, _T(APP_SET_LANDSCAPE_FLIPPED),
                           (void *)SETTINGS_DISPLAY_ORIENTATION_MENU_LANDSCAPE_180);
    mui_list_view_add_item(app->p_list_view, ICON_BACK, getLangString(_L_BACK),
                           (void *)SETTINGS_DISPLAY_ORIENTATION_MENU_EXIT);

    mui_list_view_set_selected_cb(app->p_list_view, settings_scene_display_orientation_list_view_on_selected);
    mui_view_dispatcher_switch_to_view(app->p_view_dispatcher, SETTINGS_VIEW_ID_MAIN);
}

void settings_scene_display_orientation_on_exit(void *user_data) {
    app_settings_t *app = user_data;
    mui_list_view_clear_items(app->p_list_view);
    mui_list_view_set_selected_cb(app->p_list_view, NULL);
}
