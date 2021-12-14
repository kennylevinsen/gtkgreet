i18n = import('i18n')
# define GETTEXT_PACKAGE
add_project_arguments('-DGETTEXT_PACKAGE="gtkgreet"', '-DLOCALEDIR="/usr/local/share/locale"', language:'c')
i18n.gettext(meson.project_name(),
    args: '--directory=' + meson.source_root(),
    preset: 'glib'
)
