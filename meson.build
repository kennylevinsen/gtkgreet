project(
	'gtkgreet',
	'c',
	version: '0.7',
	license: 'GPL3',
	meson_version: '>=0.47.0',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=true',
	],
)

add_project_arguments('-Wno-unused-parameter', language: 'c')
add_project_arguments('-Wno-missing-braces', language: 'c')

prefix = get_option('prefix')
localedir = join_paths(prefix, get_option('localedir'))

subdir('po')
subdir('gtkgreet')

scdoc = dependency('scdoc', required: get_option('man-pages'), version: '>= 1.9.7')

if scdoc.found()
	sh = find_program('sh')

	man_pages = ['gtkgreet.1.scd']

	mandir = get_option('mandir')

	foreach src : man_pages
		topic = src.split('.')[0]
		section = src.split('.')[1]
		output = '@0@.@1@'.format(topic, section)

		custom_target(
			output,
			input: 'man/' + src,
			output: output,
			command: [
				sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.get_pkgconfig_variable('scdoc'), output)
			],
			install: true,
			install_dir: '@0@/man@1@'.format(mandir, section)
		)
	endforeach
endif
