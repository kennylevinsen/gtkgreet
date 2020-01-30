dependencies = [dependency('gtk+-3.0'), dependency('json-c')]

if get_option('layershell')
	dependencies += dependency('gtk-layer-shell-0')
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
