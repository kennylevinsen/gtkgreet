dependencies = [dependency('gtk+-3.0'), dependency('json-c')]

gtk_layer_shell = dependency('gtk-layer-shell-0', required: get_option('layershell'))
if gtk_layer_shell.found()
	dependencies += gtk_layer_shell
	add_project_arguments('-DLAYER_SHELL', language : 'c')
endif

executable(
		'gtkgreet',
		[
			'main.c',
			'proto.c',
			'config.c',
			'actions.c',
			'window.c',
			'gtkgreet.c'
		],
		dependencies: dependencies,
		install: true)
